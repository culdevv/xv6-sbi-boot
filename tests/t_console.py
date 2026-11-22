import re
from lib import Qemu, check


def run():
    q = Qemu(1, "console")
    try:
        q.expect(r"\$ ", 60)
        q.send("printtest\n")
        q.expect(r"PT_DONE", 40)
        seen = {int(x) for x in re.findall(
            r"PT (\d+) ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", q.last)}
        check("T13 200 printf lines via SBI, none lost",
              seen == set(range(200)))

        q.expect(r"\$ ", 10)
        q.send("echo midterm-ok\n")
        check("T14 console input echo works",
              bool(q.expect(r"\n\s*midterm-ok", 10)))
    finally:
        q.close()