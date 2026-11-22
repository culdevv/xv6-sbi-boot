#!/usr/bin/env python3
import os
import re

from lib import Qemu, check


HART_PATTERN = r"(?m)^hart (\d+) starting\s*$"
EXPECTED_HARTS = set(range(8))


def test_stress(repeat):
    all_ok = True

    for i in range(1, repeat + 1):
        print(f"\n===== T15: CPUS=8, iteration {i}/{repeat} =====",
              flush=True)

        q = Qemu(8, "stress")

        try:
            # 1. Check kernel boot banner.
            boot_ok = bool(
                q.expect(r"xv6 kernel is booting", 30)
            )

            # 2. Wait for eight distinct harts.
            q.wait_unique(HART_PATTERN, 8, 30)

            actual_harts = {
                int(h) for h in q.unique(HART_PATTERN)
            }
            harts_ok = actual_harts == EXPECTED_HARTS

            # 3. Check that xv6 reaches the shell.
            shell_ok = bool(q.expect(r"\$ ", 30))

            iteration_ok = boot_ok and harts_ok and shell_ok

            if not harts_ok:
                missing = EXPECTED_HARTS - actual_harts
                unexpected = actual_harts - EXPECTED_HARTS

                print(f"Missing harts: {sorted(missing)}",
                      flush=True)
                print(f"Unexpected harts: {sorted(unexpected)}",
                      flush=True)

            if not boot_ok:
                print("Kernel boot banner not detected.",
                      flush=True)

            if not shell_ok:
                print("Shell prompt not detected.",
                      flush=True)

            check(
                f"T15 iteration {i}/{repeat}: "
                f"CPUS=8, all harts online, shell prompt",
                iteration_ok
            )

            if not iteration_ok:
                all_ok = False

        finally:
            q.close()
            q.log.close()

    check(
        f"T15 stress boot summary: {repeat} iterations",
        all_ok
    )


def run():
    try:
        repeat = int(os.environ.get("BOOT_REPEAT", "5"))

        if repeat < 1:
            raise ValueError("BOOT_REPEAT must be >= 1")

    except ValueError as e:
        check(f"T15 invalid BOOT_REPEAT: {e}", False)
        return

    print(f"T15: running CPUS=8 for {repeat} iterations",
          flush=True)

    test_stress(repeat)
