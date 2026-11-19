#!/usr/bin/env python3
import glob, importlib, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import lib

def main():
    full = "--full" in sys.argv
    patterns = ["t_*.py"] + (["f_*.py"] if full else [])
    mods = sorted(p for pat in patterns for p in glob.glob(os.path.join(HERE, pat)))
    start = time.time()
    for path in mods:
        name = os.path.basename(path)[:-3]
        print(f"--- {name}", flush=True)
        try:
            importlib.import_module(name).run()
        except Exception as e:
            lib.check(f"{name} crashed: {e}", False)
    passed = sum(lib.RESULTS)
    print(f"{passed}/{len(lib.RESULTS)} passed in {time.time() - start:.0f}s")
    sys.exit(0 if lib.RESULTS and all(lib.RESULTS) else 1)

if __name__ == "__main__":
    main()