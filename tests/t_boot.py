from lib import Qemu, check

IDS = {1: "T01", 2: "T02", 3: "T03", 4: "T04", 8: "T05"}


def boot_session(cpus):
    q = Qemu(cpus, "boot")
    try:
        ok = bool(q.expect(r"\$ ", 60))
        if ok:
            q.send("ls\n")
            ok = bool(q.expect(r"README", 15))
        check(f"{IDS[cpus]} boot to shell and ls works (CPUS={cpus})", ok)

        online = q.wait_unique(r"hart (\d+) starting", cpus, 20)
        got = len(q.unique(r"hart (\d+) starting"))
        check(f"T06 all harts online {got}/{cpus} (CPUS={cpus})", online)
    finally:
        q.close()


def run():
    for cpus in (1, 2, 3, 4, 8):
        boot_session(cpus)
