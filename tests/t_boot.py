#!/usr/bin/env python3
# import os, queue,  re, signal, subprocess, sys, threading, time
from lib import Qemu, check

# results = []

# def check(name, ok):
#     print(("PASS: " if ok else "FAIL: ") + name, flush=True)
#     results.append(bool(ok))

# class Qemu:
#     def __init__(self, cpus):
#         self.p = subprocess.Popen(
#             ["make", "-s", "qemu", f"CPUS={cpus}"],
#             stdin=subprocess.PIPE, stdout=subprocess.PIPE,
#             stderr=subprocess.STDOUT, start_new_session=True)
#         self.q, self.buf = queue.Queue(), ""
#         threading.Thread(target=self._read, daemon=True).start()

#     def _read(self):
#         for chunk in iter(lambda: self.p.stdout.read1(4096), b""):
#             self.q.put(chunk.decode(errors="replace"))

#     def expect(self, pattern, timeout):
#         end = time.time() + timeout
#         while True:
#             m = re.search(pattern, self.buf)
#             if m:
#                 self.buf = self.buf[m.end():]
#                 return m
#             left = end - time.time()
#             if left <= 0:
#                 return None
#             try:
#                 self.buf += self.q.get(timeout=left)
#             except queue.Empty:
#                 return None

#     def send(self, s):
#         self.p.stdin.write(s.encode()); self.p.stdin.flush()

#     def close(self):
#         try:
#             os.killpg(self.p.pid, signal.SIGKILL)
#         except ProcessLookupError:
#             pass
#         self.p.wait()

def test_basic(cpus):
    q = Qemu(cpus, "boot")
    try:
        check(f"T01 boot banner (CPUS={cpus})",
              q.expect(r"xv6 kernel is booting", 30))
        check(f"T02 shell prompt (CPUS={cpus})", q.expect(r"\$ ", 30))
        q.send("ls\n")
        check(f"T03 ls shows README (CPUS={cpus})", q.expect(r"README", 10))
        q.expect(r"\$ ", 10)
        q.send("ticks\n")
        m = q.expect(r"TICKS (\d+) (\d+)\r?\n", 20)
        ok = bool(m) and 20 <= int(m.group(2)) - int(m.group(1)) <= 22
        check(f"T04 timer ticks advance (CPUS={cpus})", ok)
    finally:
        q.close()

def run():
    test_basic(1)

# if __name__ == "__main__":
#     test_basic(1)
#     print(f"{sum(results)}/{len(results)} passed")
#     sys.exit(0 if all(results) else 1)
