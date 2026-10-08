#!/usr/bin/env python3
"""Reference CanDrive client (protocol only — no DEM logic)."""

from __future__ import annotations

import argparse
import socket
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from dem_proto.can_drive_pb2 import CanDriveRequest, CanDriveResponse  # noqa: E402
from tcp_framing import recv_message, send_message  # noqa: E402


def query(
    host: str,
    port: int,
    lat: float,
    lon: float,
    azimuth_deg: float,
    distance_m: float,
) -> dict[str, str]:
    sock = socket.create_connection((host, port), timeout=30)
    sock.settimeout(120)
    try:
        req = CanDriveRequest(
            lat=lat,
            lon=lon,
            azimuth_deg=azimuth_deg,
            distance_m=distance_m,
        )
        send_message(sock, req)
        resp = recv_message(sock, CanDriveResponse)
        return dict(resp.data)
    finally:
        sock.close()


def main() -> None:
    p = argparse.ArgumentParser(description="CanDrive protocol reference client")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=9100)
    p.add_argument("--lat", type=float, default=50.45)
    p.add_argument("--lon", type=float, default=30.85)
    p.add_argument("--azimuth", type=float, default=0.0)
    p.add_argument("--distance", type=float, default=10.0)
    p.add_argument("--once", action="store_true", help="single request (default)")
    args = p.parse_args()

    try:
        data = query(
            args.host,
            args.port,
            args.lat,
            args.lon,
            args.azimuth,
            args.distance,
        )
    except ConnectionRefusedError:
        print(
            "connection refused — start CanDrive server first "
            "(dem: python -u tcp_server.py)",
            file=sys.stderr,
        )
        sys.exit(1)

    for k in ("ok", "grade_pct", "delta_h_m", "z_start_m", "z_end_m", "error"):
        if k in data:
            print(f"{k}={data[k]}")
    for k, v in sorted(data.items()):
        if k not in {"ok", "grade_pct", "delta_h_m", "z_start_m", "z_end_m", "error"}:
            print(f"{k}={v}")

    sys.exit(0 if data.get("ok") == "1" else 3)


if __name__ == "__main__":
    main()
