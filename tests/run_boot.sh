
#!/bin/bash
# Chay tu thu muc goc repo: bash tests/run_boot.sh
# Can co kernel/kernel va fs.img (make kernel/kernel fs.img)

fail=0
mkdir -p test-results

check() {   # $1=ten test, $2=so CPU, $3=chuoi can thay trong log 
  local log=test-results/$1.log
  ( sleep 8; echo ls; sleep 3 ) | timeout 40 qemu-system-riscv64 \
    -machine virt -bios none -kernel kernel/kernel -m 128M -smp "$2" -nographic \
    -global virtio-mmio.force-legacy=false \
    -drive file=fs.img,if=none,format=raw,id=x0 \
    -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 > "$log" 2>&1
  if grep -q "$3" "$log"; then
    echo "PASS: $1"
  else
    echo "FAIL: $1"
    fail=1
  fi
}

check boot_cpus1 1 "init: starting sh"
check boot_cpus3 3 "init: starting sh"

exit $fail

