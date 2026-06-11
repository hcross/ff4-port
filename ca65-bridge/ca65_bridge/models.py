"""Data models — replica of the relevant re_agent.core.models dataclasses.

We replicate rather than import these to stay standalone (no hard dependency
on re-agent). The names and signatures are kept strictly identical so the
backend remains compatible with the REBackend protocol.
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
