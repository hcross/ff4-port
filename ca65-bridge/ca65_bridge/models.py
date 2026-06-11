"""Data models — réplique des dataclasses de re_agent.core.models.

Réplique plutôt qu'import pour rester standalone (pas de dépendance hard sur
re-agent). Les noms et signatures sont strictement identiques pour rester
compatibles avec le protocol REBackend.
"""
from __future__ import annotations

from dataclasses import dataclass, field


@dataclass
class BackendCapabilities:
    has_decompile: bool = False
    has_asm: bool = True
    has_structs: bool = False
    has_xrefs: bool = True
    has_search: bool = True
    has_enums: bool = False


@dataclass
class XRef:
    address: str
    name: str
    ref_type: str  # "call" (jsr), "jump" (jmp), "branch" (bra/bcc/...)


@dataclass
class FunctionEntry:
    address: str
    name: str
    class_name: str = ""
    caller_count: int = 0


@dataclass
class AsmResult:
    address: str
    instructions: str  # verbatim body
    instruction_count: int
    call_count: int
    has_fp_sensitive: bool = False  # 65816 has no FPU
