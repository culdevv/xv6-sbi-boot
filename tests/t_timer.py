from lib import Qemu, check


def run():
    q = Qemu(2, "timer")
    try:
        q.expect(r"\$ ", 60)
        q.send("ticks\n")
        m = q.expect(r"TICKS (\d+) (\d+)\r?\n", 30)
        ok = bool(m) and 20 <= int(m.group(2)) - int(m.group(1)) <= 22
        check("T11 ticks advance by sleep(20)", ok)

        q.expect(r"\$ ", 10)
        q.send("tickrate\n")
        m = q.expect(r"TICKRATE_MILLIHZ (-?\d+) ticks=(\d+)\r?\n", 40)
        rate = int(m.group(1)) if m else 0
        if m:
            print(f"  measured rate = {rate / 1000:.3f} Hz (nominal 10)")
        check("T12 timer rate within 5% of 10 Hz", abs(rate - 10000) <= 500)
    finally:
        q.close()