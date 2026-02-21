#!/usr/bin/env python3
import socket
import sys
import time

# Modes:
#   pipeline  : send chunked POST + GET in a single sendall
#   sequential: send chunked POST, read response #1, then send GET, read response #2

def recv_until(sock: socket.socket, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            break
        buf += chunk
    return buf


def recv_http_response(sock: socket.socket, timeout: float = 5.0) -> bytes:
    sock.settimeout(timeout)
    data = b""

    # Read headers
    while b"\r\n\r\n" not in data:
        chunk = sock.recv(4096)
        if not chunk:
            return data
        data += chunk

    header, rest = data.split(b"\r\n\r\n", 1)
    headers_text = header.decode("latin1", errors="replace")

    # Parse Content-Length if present
    content_length = None
    for line in headers_text.split("\r\n")[1:]:
        if ":" not in line:
            continue
        k, v = line.split(":", 1)
        if k.strip().lower() == "content-length":
            try:
                content_length = int(v.strip())
            except ValueError:
                content_length = None

    body = rest
    if content_length is not None:
        while len(body) < content_length:
            chunk = sock.recv(4096)
            if not chunk:
                break
            body += chunk
        return header + b"\r\n\r\n" + body[:content_length]

    # Fallback: not handling chunked responses here; just return what we have.
    return header + b"\r\n\r\n" + body


def read_all(sock: socket.socket, timeout: float = 2.0) -> bytes:
    sock.settimeout(timeout)
    out = b""
    while True:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            break
        if not chunk:
            break
        out += chunk
    return out


def build_requests(host: str, port: int):
    post = (
        "POST /upload HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Connection: keep-alive\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "5\r\nhello\r\n"
        "6\r\n world\r\n"
        "0\r\n\r\n"
    ).encode("latin1")

    get = (
        "GET / HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Connection: close\r\n"
        "\r\n"
    ).encode("latin1")

    return post, get


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 8080
    mode = sys.argv[3] if len(sys.argv) > 3 else "pipeline"

    if mode not in ("pipeline", "sequential"):
        print("usage: test_chunked_keepalive.py [host] [port] [pipeline|sequential]")
        sys.exit(2)

    post, get = build_requests(host, port)

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    if mode == "pipeline":
        req = post + get
        print(f"mode=pipeline sending bytes={len(req)}")
        preview = req[:400].replace(b"\r", b"\\r").replace(b"\n", b"\\n")
        print(f"preview={preview!r}")
        s.sendall(req)

        r1 = recv_http_response(s, timeout=10.0)
        print("--- response #1 ---")
        sys.stdout.write(r1.decode("latin1", errors="replace"))
        print("\n-------------------")

        try:
            r2 = recv_http_response(s, timeout=10.0)
        except socket.timeout:
            r2 = b""

        print("--- response #2 ---")
        if r2:
            sys.stdout.write(r2.decode("latin1", errors="replace"))
            print("\n-------------------")
        else:
            print("(no second response header)\n")

        extra = read_all(s, timeout=1.0)
        if extra:
            print("--- extra bytes after #2 (drain) ---")
            sys.stdout.write(extra.decode("latin1", errors="replace"))
            print("\n-----------------------------------")

    else:
        print(f"mode=sequential sending POST bytes={len(post)}")
        s.sendall(post)

        r1 = recv_http_response(s, timeout=10.0)
        print("--- response #1 (POST) ---")
        sys.stdout.write(r1.decode("latin1", errors="replace"))
        print("\n--------------------------")

        print(f"sending GET bytes={len(get)}")
        s.sendall(get)

        try:
            r2 = recv_http_response(s, timeout=10.0)
        except socket.timeout:
            r2 = b""

        print("--- response #2 (GET) ---")
        if r2:
            sys.stdout.write(r2.decode("latin1", errors="replace"))
            print("\n-------------------------")
        else:
            print("(no second response header)\n")

        extra = read_all(s, timeout=1.0)
        if extra:
            print("--- extra bytes after #2 (drain) ---")
            sys.stdout.write(extra.decode("latin1", errors="replace"))
            print("\n-----------------------------------")

    s.close()


if __name__ == "__main__":
    main()
