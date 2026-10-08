#!/usr/bin/env python3
"""Generate dem_proto/can_drive_pb2.py from ../proto/can_drive.proto."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROTO = ROOT.parent / "proto" / "can_drive.proto"
OUT = ROOT / "dem_proto"


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "__init__.py").write_text("")
    cmd = [
        sys.executable,
        "-m",
        "grpc_tools.protoc",
        f"-I{PROTO.parent}",
        f"--python_out={OUT}",
        str(PROTO),
    ]
    subprocess.check_call(cmd)
    print(f"generated {OUT / 'can_drive_pb2.py'}")


if __name__ == "__main__":
    main()
