#!/usr/bin/env python3
"""Compile original EX603 GameServer TUs without linking or starting a server.

This is a diagnostic aid for the real CMake object target, not a substitute for a
runtime build. It writes only aggregate compiler diagnostics, never runtime config.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import json
import pathlib
import subprocess
import sys

PLATFORM_SOURCES = {
    "GameServer.cpp", "GameMain.cpp", "SocketManager.cpp", "SocketManagerUdp.cpp",
    "Connection.cpp", "QueryManager.cpp", "ServerDisplayer.cpp", "MiniDump.cpp",
    "Util.cpp", "HackCheck.cpp",
}


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    parser.add_argument("--compiler", default="clang++")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--only", nargs="*", help="Specific original .cpp basenames")
    args = parser.parse_args()
    if not (root / "native/include/mu/platform.hpp").is_file():
        parser.error("native/include/mu/platform.hpp must be implemented before probing gameplay")
    if args.jobs < 1 or args.jobs > 16:
        parser.error("--jobs must be 1..16")
    files = sorted((root / "GameServer").glob("*.cpp"))
    files = [p for p in files if p.name not in PLATFORM_SOURCES]
    if args.only is not None:
        requested = set(args.only)
        unknown = requested - {p.name for p in files}
        if unknown:
            parser.error("unknown or excluded source: " + ", ".join(sorted(unknown)))
        files = [p for p in files if p.name in requested]
    flags = [
        "-fsyntax-only", "-std=c++17", "-fms-extensions", "-Wno-writable-strings",
        "-Wno-ignored-pragmas", "-Wno-deprecated-declarations", "-ferror-limit=12",
        "-DMU_NATIVE_MACOS=1", "-DGAMESERVER_UPDATE=603", "-DGAMESERVER_LANGUAGE=1",
        "-DGAMESERVER_SHOP=3", "-DGAMESERVER_EXTRA=1", "-DENCRYPT_STATE=1",
        "-I" + str(root / "native/include"), "-iquote", str(root / "GameServer"),
        "-I" + str(root.parents[1] / "Util/lua/include"),
    ]

    def probe(path: pathlib.Path) -> dict:
        process = subprocess.run([args.compiler, *flags, str(path)], capture_output=True, text=True, errors="replace", timeout=90)
        return {
            "source": str(path.relative_to(root)), "exitCode": process.returncode,
            "diagnostics": process.stderr,
        }

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(probe, files))
    passed = sum(r["exitCode"] == 0 for r in results)
    report = {
        "profile": "EX603ENG", "kind": "syntax-only-original-gameplay",
        "compiler": args.compiler, "flags": flags,
        "tested": len(results), "passed": passed, "failed": len(results) - passed,
        "excludedRuntimeSources": sorted(PLATFORM_SOURCES), "results": results,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print(f"Original GameServer syntax probes: {passed}/{len(results)} passed; report: {args.output}")
    return 0 if passed == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
