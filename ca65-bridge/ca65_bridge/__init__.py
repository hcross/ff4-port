"""ca65-bridge — RE backend for ca65/65816 disassembly projects."""
from ca65_bridge.backend import Ca65BridgeBackend
from ca65_bridge.models import (
    AsmResult,
    BackendCapabilities,
    FunctionEntry,
    XRef,
)

__all__ = [
    "Ca65BridgeBackend",
    "AsmResult",
    "BackendCapabilities",
    "FunctionEntry",
    "XRef",
]
__version__ = "0.1.0"
