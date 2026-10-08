"""Length-prefixed Protobuf framing (uint32 BE + payload)."""

from __future__ import annotations

import socket
import struct
from typing import TypeVar

from google.protobuf.message import Message

T = TypeVar("T", bound=Message)

_HEADER = struct.Struct(">I")
_MAX_FRAME = 4 * 1024 * 1024


def send_message(sock: socket.socket, msg: Message) -> None:
    payload = msg.SerializeToString()
    sock.sendall(_HEADER.pack(len(payload)) + payload)


def recv_exact(sock: socket.socket, n: int) -> bytes:
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("connection closed")
        buf.extend(chunk)
    return bytes(buf)


def recv_message(sock: socket.socket, msg_type: type[T]) -> T:
    (size,) = _HEADER.unpack(recv_exact(sock, _HEADER.size))
    if size > _MAX_FRAME:
        raise ValueError(f"frame too large: {size}")
    payload = recv_exact(sock, size)
    msg = msg_type()
    msg.ParseFromString(payload)
    return msg
