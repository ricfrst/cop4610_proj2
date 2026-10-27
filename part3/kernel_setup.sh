#!/bin/bash
# Adds the elevator system calls (548-550) to a Linux kernel source tree and
# prepares a build config. Safe to run more than once.
#
# Usage: ./kernel_setup.sh ~/linux-6.16.x
set -e

KDIR="${1:?usage: $0 <path-to-kernel-source>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
KDIR="$(cd "$KDIR" && pwd)"

TBL="$KDIR/arch/x86/entry/syscalls/syscall_64.tbl"
HDR="$KDIR/include/linux/syscalls.h"
TOP="$KDIR/Makefile"

for f in "$TBL" "$HDR" "$TOP"; do
    [ -f "$f" ] || { echo "error: $f not found, is $KDIR a kernel source tree?"; exit 1; }
done

# 1. syscalls/syscalls.c and syscalls/Makefile
mkdir -p "$KDIR/syscalls"
cp "$HERE/syscalls.c" "$KDIR/syscalls/syscalls.c"
echo 'obj-y := syscalls.o' > "$KDIR/syscalls/Makefile"
echo "[ok] syscalls/syscalls.c and syscalls/Makefile"

# 2. syscall table entries
if ! grep -q 'sys_start_elevator' "$TBL"; then
    [ -z "$(tail -c1 "$TBL")" ] || echo >> "$TBL"
    printf '548\tcommon\tstart_elevator\t\tsys_start_elevator\n' >> "$TBL"
    printf '549\tcommon\tissue_request\t\tsys_issue_request\n' >> "$TBL"
    printf '550\tcommon\tstop_elevator\t\tsys_stop_elevator\n' >> "$TBL"
fi
echo "[ok] arch/x86/entry/syscalls/syscall_64.tbl"

# 3. prototypes in include/linux/syscalls.h, placed before the final #endif
if ! grep -q 'sys_start_elevator' "$HDR"; then
    last=$(grep -n '^#endif' "$HDR" | tail -1 | cut -d: -f1)
    {
        head -n $((last - 1)) "$HDR"
        echo 'asmlinkage long sys_start_elevator(void);'
        echo 'asmlinkage long sys_issue_request(int start_floor, int destination_floor, int type);'
        echo 'asmlinkage long sys_stop_elevator(void);'
        tail -n +"$last" "$HDR"
    } > "$HDR.tmp"
    mv "$HDR.tmp" "$HDR"
fi
echo "[ok] include/linux/syscalls.h"

# 4. build the syscalls/ directory into the kernel from the top-level Makefile
if ! grep -q '^core-y.*+= *syscalls/' "$TOP"; then
    if [ "$(grep -c '^core-y[[:space:]]*:=' "$TOP")" -ne 1 ]; then
        echo "error: could not find a single 'core-y :=' line in $TOP; add"
        echo "       'core-y += syscalls/' after it by hand"
        exit 1
    fi
    sed -i '/^core-y[[:space:]]*:=/a core-y\t\t+= syscalls/' "$TOP"
fi
echo "[ok] Makefile"

# 5. config: start from the running kernel's config, drop Ubuntu's signing
#    certificates (the build fails without this) and skip debug info so the
#    build is faster and much smaller
cd "$KDIR"
if [ ! -f .config ]; then
    cp "/boot/config-$(uname -r)" .config
fi
scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
scripts/config --disable DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT
scripts/config --disable DEBUG_INFO_DWARF4
scripts/config --disable DEBUG_INFO_DWARF5
scripts/config --enable DEBUG_INFO_NONE
make olddefconfig > /dev/null
echo "[ok] .config"

echo
echo "Done. Next: cd $KDIR && make menuconfig (Save, Exit), then build."
