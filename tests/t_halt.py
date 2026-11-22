from lib import Qemu, check


def run():
    q = Qemu(1, "halt")
    try:
        q.expect(r"\$ ", 60)
        q.send("halt\n")
        check("T09 halt powers off QEMU with exit code 0",
              q.wait_exit(20) == 0)
    finally:
        q.close()

    q = Qemu(1, "halt-busy")
    try:
        q.expect(r"\$ ", 60)
        q.send("spin &\n")
        q.expect(r"\$ ", 10)
        q.send("halt\n")
        check("T10 halt works while a process spins (CPUS=1)",
              q.wait_exit(30) == 0)
    finally:
        q.close()