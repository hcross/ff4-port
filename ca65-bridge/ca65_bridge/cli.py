"""ca65-bridge CLI — lightweight mirror of ghidra-bridge."""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ca65_bridge.backend import Ca65BridgeBackend


def _cmd_get_asm(b: Ca65BridgeBackend, args) -> int:
    r = b.get_asm(args.target)
    if not r:
        print(f"label not found: {args.target}", file=sys.stderr)
        return 1
    print(f"# address_hint: {r.address}  instr={r.instruction_count}  calls={r.call_count}")
    print(r.instructions)
    return 0


def _cmd_xrefs_from(b: Ca65BridgeBackend, args) -> int:
    refs = b.xrefs_from(args.target)
    if not refs:
        print(f"no outgoing xref from {args.target}", file=sys.stderr)
    for x in refs:
        print(f"{x.ref_type:7s}  {x.name}  @{x.address or '?'}")
    return 0


def _cmd_xrefs_to(b: Ca65BridgeBackend, args) -> int:
    refs = b.xrefs_to(args.target)
    if not refs:
        print(f"no incoming xref to {args.target}", file=sys.stderr)
    for x in refs:
        print(f"{x.ref_type:7s}  from {x.name}  @{x.address or '?'}")
    return 0


def _cmd_search(b: Ca65BridgeBackend, args) -> int:
    matches = b.search(args.pattern)
    for f in matches:
        print(f"{f.class_name:10s}  {f.name}  @{f.address or '?'}")
    return 0


def _cmd_classify(b: Ca65BridgeBackend, args) -> int:
    c = b.classify_routine(args.target,
                            instr_max=args.instr_max,
                            calls_max=args.calls_max)
    if c is None:
        print(f"label not found: {args.target}", file=sys.stderr)
        return 1
    print(f"{args.target}: {c.decision}")
    for r in c.reasons:
        print(f"  - {r}")
    return 0


def _cmd_classify_module(b: Ca65BridgeBackend, args) -> int:
    """Classify all routines of a module and tabulate."""
    funcs = b.remaining(args.module)
    if not funcs:
        print(f"no routine found for module={args.module}", file=sys.stderr)
        return 1
    n_translate, n_delegate = 0, 0
    print(f"{'function':30s}  {'decision':10s}  reasons")
    for f in funcs:
        c = b.classify_routine(f.name,
                                instr_max=args.instr_max,
                                calls_max=args.calls_max)
        if not c:
            continue
        if c.decision == 'translate':
            n_translate += 1
        else:
            n_delegate += 1
        reasons_str = "; ".join(c.reasons[:1])  # first reason only for brevity
        print(f"{f.name:30s}  {c.decision:10s}  {reasons_str}")
    total = n_translate + n_delegate
    print(f"\n--- summary ---  total={total}  translate={n_translate}  delegate={n_delegate}  ratio_translate={100*n_translate/max(total,1):.0f}%")
    return 0


def _cmd_info(b: Ca65BridgeBackend, args) -> int:
    caps = b.capabilities
    print(f"root       : {b.root}")
    print(f"routines   : {len(b._routines)}")
    print(f"capabilities:")
    for f in caps.__dataclass_fields__:
        print(f"  {f:18s} = {getattr(caps, f)}")
    return 0


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(
        prog="ca65-bridge",
        description="RE backend for ca65/65816 disassemblies",
    )
    p.add_argument("--root", required=True, type=Path,
                   help="root of the disassembly repo")
    sub = p.add_subparsers(dest="cmd", required=True)

    sp = sub.add_parser("get-asm", help="body of a routine")
    sp.add_argument("target")
    sp.set_defaults(fn=_cmd_get_asm)

    sp = sub.add_parser("xrefs-from", help="outgoing calls/jumps/branches")
    sp.add_argument("target")
    sp.set_defaults(fn=_cmd_xrefs_from)

    sp = sub.add_parser("xrefs-to", help="who calls/branches to target")
    sp.add_argument("target")
    sp.set_defaults(fn=_cmd_xrefs_to)

    sp = sub.add_parser("search", help="regex grep on labels")
    sp.add_argument("pattern")
    sp.set_defaults(fn=_cmd_search)

    sp = sub.add_parser("classify", help="ADR-003: translate vs delegate for one routine")
    sp.add_argument("target")
    sp.add_argument("--instr-max", type=int, default=50)
    sp.add_argument("--calls-max", type=int, default=2)
    sp.set_defaults(fn=_cmd_classify)

    sp = sub.add_parser("classify-module", help="ADR-003 over an entire module")
    sp.add_argument("module", help="battle, menu, field, btlgfx, cutscene, sound")
    sp.add_argument("--instr-max", type=int, default=50)
    sp.add_argument("--calls-max", type=int, default=2)
    sp.set_defaults(fn=_cmd_classify_module)

    sp = sub.add_parser("info", help="capabilities + stats")
    sp.set_defaults(fn=_cmd_info)

    args = p.parse_args(argv)
    b = Ca65BridgeBackend(args.root)
    return args.fn(b, args)


if __name__ == "__main__":
    sys.exit(main())
