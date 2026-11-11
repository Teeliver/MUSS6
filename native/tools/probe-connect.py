#!/usr/bin/env python3
"""Read-only ConnectServer handshake and F4:06 discovery probe. No fake heartbeats."""
import argparse
import socket
import sys


def read_exact(peer, size):
    data = bytearray()
    while len(data) < size:
        chunk = peer.recv(size - len(data))
        if not chunk:
            raise RuntimeError('ConnectServer closed the connection before the complete response')
        data.extend(chunk)
    return bytes(data)


def read_frame(peer):
    kind = read_exact(peer, 1)
    if kind == b'\xc1':
        length = read_exact(peer, 1)
        header = kind + length
        size = length[0]
        minimum = 3
    elif kind == b'\xc2':
        length = read_exact(peer, 2)
        header = kind + length
        size = int.from_bytes(length, 'big')
        minimum = 4
    else:
        raise RuntimeError('Unexpected ConnectServer packet type')
    if not minimum <= size <= 2048:
        raise RuntimeError('Invalid ConnectServer frame length')
    return header + read_exact(peer, size - len(header))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=44405)
    args = parser.parse_args()
    if not 1 <= args.port <= 65535:
        parser.error('--port must be 1..65535')
    with socket.create_connection((args.host, args.port), timeout=3) as peer:
        peer.settimeout(3)
        if read_frame(peer) != b'\xc1\x04\x00\x01':
            raise RuntimeError('The expected successful C1:00 ConnectServer handshake was not received')
        peer.sendall(b'\xc1\x04\xf4\x06')
        for _ in range(8):
            frame = read_frame(peer)
            if frame[0] == 0xc2 and frame[3:5] == b'\xf4\x06':
                if len(frame) < 7:
                    raise RuntimeError('Truncated F4:06 server-list response')
                count = int.from_bytes(frame[5:7], 'big')
                if len(frame) != 7 + count * 4:
                    raise RuntimeError('Server-list count does not match its frame length')
                print('PASS: ConnectServer handshake C1:00 and server-list response F4:06.')
                print(f'Available servers: {count}')
                if count == 0:
                    print('Discovery is responding; availability still requires Join/Game heartbeats and a visible server entry.')
                print('This does not test database login or gameplay. UDP is not probed.')
                return 0
        raise RuntimeError('F4:06 server-list response was not received')


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, RuntimeError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
