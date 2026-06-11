"""LLM providers — abstraction pluggable pour batch_translate.

Trois implémentations :
  * ClaudeCliProvider — invocation `claude --print` non-interactif (DEFAULT)
    Coût : inclus dans l'abonnement Claude Code, pas de billing API marginal.
  * AnthropicSdkProvider — SDK Python anthropic, API pay-as-you-go au token
    Avantage : prompt caching natif, contrôle fin du budget.
  * OpenAiCompatProvider — compatible OpenAI Chat Completions
    Pour Ollama local (no key), OpenRouter, LM Studio, vLLM, etc.

Toutes les implementations exposent :
    translate(system, examples, user_prompt, model) -> (code_or_None, CallStats)

Où :
    system, examples : strings (mis en cache si supporté)
    user_prompt : string (varie par fonction)
    model : string (ex. "claude-sonnet-4-5", "gpt-4o-mini", "llama3:8b")
"""
from __future__ import annotations

import dataclasses
import json
import os
import re
import subprocess
import sys
from typing import Optional, Protocol


# Tarifs par modèle, $/M tokens (à jour 2026-06)
PRICING = {
    # Anthropic
    "claude-haiku-4-5":  {"in": 1.0,  "out": 5.0,  "cache_read": 0.10},
    "claude-sonnet-4-5": {"in": 3.0,  "out": 15.0, "cache_read": 0.30},
    "claude-sonnet-4-6": {"in": 3.0,  "out": 15.0, "cache_read": 0.30},
    "claude-opus-4-7":   {"in": 15.0, "out": 75.0, "cache_read": 1.50},
    # OpenAI / OpenAI-compat (à compléter ou ignorer pour local)
    "gpt-4o":      {"in": 2.5,  "out": 10.0, "cache_read": 1.25},
    "gpt-4o-mini": {"in": 0.15, "out": 0.6,  "cache_read": 0.075},
    "default":     {"in": 0.0,  "out": 0.0,  "cache_read": 0.0},
}


@dataclasses.dataclass
class CallStats:
    tokens_in: int = 0
    tokens_out: int = 0
    tokens_cache_read: int = 0
    tokens_cache_create: int = 0
    cost_usd: float = 0.0
    provider: str = ""
    model: str = ""


class LLMProvider(Protocol):
    """Contrat minimal d'un provider LLM."""
    name: str

    def translate(
        self,
        system: str,
        examples: str,
        user_prompt: str,
        model: str,
        max_output_tokens: int,
        dry_run: bool,
    ) -> tuple[Optional[str], CallStats]:
        ...


_CODE_BLOCK_RE = re.compile(r"```c\s*\n(.*?)```", re.S)


def extract_c_code(response: str) -> Optional[str]:
    m = _CODE_BLOCK_RE.search(response)
    return m.group(1).strip() if m else None


def _price_anthropic_style(stats: CallStats, model: str) -> float:
    p = PRICING.get(model, PRICING["default"])
    return (
        stats.tokens_in * p["in"] / 1_000_000
        + stats.tokens_out * p["out"] / 1_000_000
        + stats.tokens_cache_read * p["cache_read"] / 1_000_000
        + stats.tokens_cache_create * p["in"] * 1.25 / 1_000_000
    )


# ===========================================================================
# 1. ClaudeCliProvider — DEFAULT
# ===========================================================================

class ClaudeCliProvider:
    name = "claude-cli"

    def __init__(self, bin_path: str = "claude"):
        self.bin = bin_path

    def translate(
        self,
        system: str,
        examples: str,
        user_prompt: str,
        model: str,
        max_output_tokens: int,
        dry_run: bool,
    ) -> tuple[Optional[str], CallStats]:
        # Combine system + examples en un seul system prompt (claude CLI
        # n'a pas de notion de prompt caching exposée — il en fait peut-être
        # silencieusement, peu importe pour le billing utilisateur côté
        # abonnement).
        combined_system = f"{system}\n\n# Reference examples\n\n{examples}"

        if dry_run:
            stats = CallStats(
                tokens_in=(len(user_prompt) + len(combined_system)) // 4,
                tokens_out=max_output_tokens // 2,
                provider=self.name,
                model=model,
            )
            # Pas de cost — abonnement Claude Code
            return None, stats

        # Invocation : user prompt via stdin, system via flag, JSON en sortie
        # `--tools ""` désactive Read/Edit/Bash (pure text-to-text)
        cmd = [
            self.bin, "-p", user_prompt,
            "--append-system-prompt", combined_system,
            "--output-format", "json",
            "--model", model,
            "--tools", "",
        ]
        try:
            res = subprocess.run(
                cmd, capture_output=True, text=True,
                timeout=120,  # 2 min hard timeout par appel
            )
        except subprocess.TimeoutExpired:
            sys.stderr.write(f"[claude-cli] TIMEOUT for translation\n")
            return None, CallStats(provider=self.name, model=model)

        if res.returncode != 0:
            sys.stderr.write(f"[claude-cli] non-zero exit: {res.stderr[:500]}\n")
            return None, CallStats(provider=self.name, model=model)

        try:
            data = json.loads(res.stdout)
        except json.JSONDecodeError as e:
            sys.stderr.write(f"[claude-cli] JSON parse error: {e}\n")
            return None, CallStats(provider=self.name, model=model)

        text = data.get("result", "")
        usage = data.get("usage", {})
        stats = CallStats(
            tokens_in=usage.get("input_tokens", 0),
            tokens_out=usage.get("output_tokens", 0),
            cost_usd=float(data.get("total_cost_usd", 0.0)),
            provider=self.name,
            model=model,
        )
        return extract_c_code(text), stats


# ===========================================================================
# 2. AnthropicSdkProvider — pay-as-you-go API
# ===========================================================================

class AnthropicSdkProvider:
    name = "anthropic-sdk"

    def __init__(self):
        self._client = None  # lazy

    def _client_or_init(self):
        if self._client is None:
            try:
                import anthropic
            except ImportError:
                raise RuntimeError("`pip install anthropic` required for anthropic-sdk provider")
            self._client = anthropic.Anthropic()
        return self._client

    def translate(
        self,
        system: str,
        examples: str,
        user_prompt: str,
        model: str,
        max_output_tokens: int,
        dry_run: bool,
    ) -> tuple[Optional[str], CallStats]:
        if dry_run:
            sys_chars = len(system) + len(examples)
            user_chars = len(user_prompt)
            stats = CallStats(
                tokens_in=user_chars // 4,
                tokens_cache_read=sys_chars // 4,
                tokens_out=max_output_tokens // 2,
                provider=self.name,
                model=model,
            )
            stats.cost_usd = _price_anthropic_style(stats, model)
            return None, stats

        client = self._client_or_init()
        response = client.messages.create(
            model=model,
            max_tokens=max_output_tokens,
            system=[
                {"type": "text", "text": system,   "cache_control": {"type": "ephemeral"}},
                {"type": "text", "text": examples, "cache_control": {"type": "ephemeral"}},
            ],
            messages=[{"role": "user", "content": user_prompt}],
        )
        u = response.usage
        stats = CallStats(
            tokens_in=u.input_tokens,
            tokens_out=u.output_tokens,
            tokens_cache_read=getattr(u, "cache_read_input_tokens", 0) or 0,
            tokens_cache_create=getattr(u, "cache_creation_input_tokens", 0) or 0,
            provider=self.name,
            model=model,
        )
        stats.cost_usd = _price_anthropic_style(stats, model)
        text = response.content[0].text if response.content else ""
        return extract_c_code(text), stats


# ===========================================================================
# 3. OpenAiCompatProvider — OpenAI / Ollama / OpenRouter / LM Studio / vLLM
# ===========================================================================

class OpenAiCompatProvider:
    name = "openai-compat"

    def __init__(self, api_base: str, api_key: Optional[str] = None):
        self.api_base = api_base.rstrip("/")
        self.api_key = api_key  # peut être None (Ollama local)

    def translate(
        self,
        system: str,
        examples: str,
        user_prompt: str,
        model: str,
        max_output_tokens: int,
        dry_run: bool,
    ) -> tuple[Optional[str], CallStats]:
        if dry_run:
            sys_chars = len(system) + len(examples)
            user_chars = len(user_prompt)
            stats = CallStats(
                tokens_in=(sys_chars + user_chars) // 4,
                tokens_out=max_output_tokens // 2,
                provider=self.name,
                model=model,
            )
            stats.cost_usd = _price_anthropic_style(stats, model)
            return None, stats

        # Pas de prompt caching standardisé — on combine system+examples
        combined_system = f"{system}\n\n# Reference examples\n\n{examples}"
        payload = {
            "model": model,
            "messages": [
                {"role": "system", "content": combined_system},
                {"role": "user",   "content": user_prompt},
            ],
            "max_tokens": max_output_tokens,
            "temperature": 0.0,  # déterministe pour reproductibilité
        }
        headers = {"Content-Type": "application/json"}
        if self.api_key:
            headers["Authorization"] = f"Bearer {self.api_key}"

        # Utilise urllib pour éviter dépendance requests
        try:
            import urllib.request
            req = urllib.request.Request(
                f"{self.api_base}/chat/completions",
                data=json.dumps(payload).encode(),
                headers=headers,
                method="POST",
            )
            with urllib.request.urlopen(req, timeout=120) as resp:
                data = json.loads(resp.read())
        except Exception as e:
            sys.stderr.write(f"[openai-compat] HTTP error: {e}\n")
            return None, CallStats(provider=self.name, model=model)

        text = ""
        try:
            text = data["choices"][0]["message"]["content"]
        except (KeyError, IndexError):
            sys.stderr.write(f"[openai-compat] unexpected response shape\n")
            return None, CallStats(provider=self.name, model=model)

        usage = data.get("usage", {})
        stats = CallStats(
            tokens_in=usage.get("prompt_tokens", 0),
            tokens_out=usage.get("completion_tokens", 0),
            provider=self.name,
            model=model,
        )
        stats.cost_usd = _price_anthropic_style(stats, model)
        return extract_c_code(text), stats


# ===========================================================================
# Factory
# ===========================================================================

DEFAULT_MODELS = {
    "claude-cli":    "claude-sonnet-4-5",
    "anthropic-sdk": "claude-sonnet-4-5",
    "openai-compat": "gpt-4o-mini",
}


def create_provider(
    kind: str,
    *,
    bin_path: str = "claude",
    api_base: Optional[str] = None,
    api_key: Optional[str] = None,
) -> LLMProvider:
    if kind == "claude-cli":
        return ClaudeCliProvider(bin_path=bin_path)
    if kind == "anthropic-sdk":
        return AnthropicSdkProvider()
    if kind == "openai-compat":
        if not api_base:
            raise ValueError("openai-compat requires --api-base")
        return OpenAiCompatProvider(api_base=api_base, api_key=api_key)
    raise ValueError(f"unknown LLM provider: {kind}")
