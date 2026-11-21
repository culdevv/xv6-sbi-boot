import glob, re, shutil, subprocess
from lib import check

FORBIDDEN = re.compile(
    r"\b(mret|mtvec|mscratch|mhartid|mstatus|mepc|medeleg|mideleg|"
    r"menvcfg|mcounteren|pmpcfg0|pmpaddr0|mie)\b")


def find_readelf():
    for prefix in ("riscv64-unknown-elf-", "riscv64-linux-gnu-"):
        if shutil.which(prefix + "readelf"):
            return prefix + "readelf"
    return None


def run():
    tool = find_readelf()
    ok = False
    if tool:
        out = subprocess.run([tool, "-h", "kernel/kernel"],
                             capture_output=True, text=True).stdout
        m = re.search(r"Entry point address:\s+(0x[0-9a-fA-F]+)", out)
        ok = bool(m) and int(m.group(1), 16) == 0x80200000
    check("T07 kernel entry point is 0x80200000", ok)

    bad = []
    for path in glob.glob("kernel/*.[chS]"):
        with open(path, errors="replace") as f:
            for n, line in enumerate(f, 1):
                if FORBIDDEN.search(line):
                    bad.append(f"{path}:{n}")
    if bad:
        print("  M-mode leftovers:", ", ".join(bad[:10]))
    check("T08 no M-mode code left in kernel/", not bad)
