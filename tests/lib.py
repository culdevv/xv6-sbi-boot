import datetime, os, queue, re, signal, subprocess, threading, time

RESULTS = []
LOGDIR = "test-results"

def check(name, ok):
    print(("PASS: " if ok else "FAIL: ") + name, flush=True)
    RESULTS.append(bool(ok))

class Qemu:
    def __init__(self, cpus, tag="run"):
        os.makedirs(LOGDIR, exist_ok=True)
        stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
        self.log = open(f"{LOGDIR}/{tag}-cpus{cpus}-{stamp}.log", "w")
        self.p = subprocess.Popen(
            ["make", "-s", "qemu", f"CPUS={cpus}"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, start_new_session=True)
        self.q = queue.Queue()
        self.buf = ""
        self.last = ""
        self.transcript = ""
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self):
        for chunk in iter(lambda: self.p.stdout.read1(4096), b""):
            text = chunk.decode(errors="replace")
            self.transcript += text
            self.log.write(text)
            self.log.flush()
            self.q.put(text)

    def expect(self, pattern, timeout):
        end = time.time() + timeout
        while True:
            m = re.search(pattern, self.buf)
            if m:
                self.last = self.buf[:m.end()]
                self.buf = self.buf[m.end():]
                return m
            left = end - time.time()
            if left <= 0:
                return None
            try:
                self.buf += self.q.get(timeout=left)
            except queue.Empty:
                return None

    def unique(self, pattern):
        return set(re.findall(pattern, self.transcript))

    def wait_unique(self, pattern, n, timeout):
        end = time.time() + timeout
        while time.time() < end:
            if len(self.unique(pattern)) >= n:
                return True
            time.sleep(0.5)
        return len(self.unique(pattern)) >= n

    def send(self, s):
        self.p.stdin.write(s.encode())
        self.p.stdin.flush()

    def wait_exit(self, timeout):
        try:
            return self.p.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            return None

    def close(self):
        try:
            os.killpg(self.p.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        self.p.wait()
        self.log.close()