#!/bin/sh
# Builds Quick Result.
#
#   sh build.sh            build out/quickresult.nro
#   sh build.sh release    build, then make dist/QuickResult-<version>.zip (the folder players put in mods/)
#   sh build.sh install    build, then copy the mod into the recomp's mods/QuickResult/ (RECOMP=<recomp folder>)
#   sh build.sh diag       build the research plugin, out/quickresult_diag.nro (logs the quest-end flow, changes nothing)
#
# Needs an arm-none-eabi GCC and binutils, and Python 3. Overrides: CROSS=<tool prefix>, PYTHON=<python command>.
set -e
cd "$(dirname "$0")"
VERSION=1.1.0
OUT=out

# Find the toolchain: on PATH, else Arm's Windows installer folder
CROSS=${CROSS:-arm-none-eabi-}
if ! command -v "${CROSS}gcc" >/dev/null 2>&1; then
    for d in "/c/Program Files (x86)/Arm GNU Toolchain arm-none-eabi"/*/bin "/c/Program Files/Arm GNU Toolchain arm-none-eabi"/*/bin; do
        if [ -x "$d/arm-none-eabi-gcc.exe" ]; then PATH="$d:$PATH"; break; fi
    done
fi
command -v "${CROSS}gcc" >/dev/null 2>&1 || { echo "error: ${CROSS}gcc not found (install the Arm GNU Toolchain, see README)"; exit 1; }

# Find Python 3 (on Windows, "python3" may be the Microsoft Store stub, and python.org's installer may skip PATH)
if [ -z "$PYTHON" ]; then
    CANDIDATES="python3 python py"
    if [ -n "$LOCALAPPDATA" ] && command -v cygpath >/dev/null 2>&1; then
        for p in "$(cygpath "$LOCALAPPDATA")"/Programs/Python/Python3*/python.exe; do
            [ -x "$p" ] && CANDIDATES="$CANDIDATES $p"
        done
    fi
    for p in $CANDIDATES; do
        if "$p" -c 'import sys; sys.exit(sys.version_info[0] != 3)' >/dev/null 2>&1; then PYTHON=$p; break; fi
    done
fi
[ -n "$PYTHON" ] || { echo "error: Python 3 not found (see README)"; exit 1; }

build() {  # build <source> <output name>
    CFLAGS="-march=armv7-a -marm -mfloat-abi=hard -mfpu=vfpv3-d16 -std=gnu11 -O2 -fPIC -ffreestanding -fno-builtin
            -nostdlib -fno-stack-protector -fvisibility=hidden -fno-unwind-tables -fno-asynchronous-unwind-tables
            -Wall -Wextra -Isrc -DPLUGIN_VERSION=\"$VERSION\""
    mkdir -p "$OUT"
    "${CROSS}gcc" $CFLAGS -c "$1" -o "$OUT/$2.o"
    "${CROSS}gcc" $CFLAGS -c src/start.S -o "$OUT/start.o"
    "${CROSS}ld" -shared -T build/nro.ld --hash-style=sysv -z max-page-size=0x1000 \
        -soname "$2.nro" "$OUT/start.o" "$OUT/$2.o" -o "$OUT/$2.elf"
    "$PYTHON" -I build/elf2nro.py "$OUT/$2.elf" "$OUT/$2.nro"
}

# The mod folder as players get it
make_mod() {
    mkdir -p "$1/romfs/nativeNX/plugins"
    cp "$OUT/quickresult.nro" "$1/romfs/nativeNX/plugins/"
}

case "$1" in
diag)
    build research/diag.c quickresult_diag
    ;;
release)
    build src/quickresult.c quickresult
    rm -rf "$OUT/release" && make_mod "$OUT/release/QuickResult"
    cp README.md LICENSE THIRD_PARTY_NOTICES "$OUT/release/QuickResult/"
    mkdir -p dist
    "$PYTHON" -I - "$OUT/release" "dist/QuickResult-$VERSION.zip" <<'EOF'
import os, sys, zipfile
root, out = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    for d, _, files in sorted(os.walk(root)):
        for f in sorted(files):
            p = os.path.join(d, f)
            z.write(p, os.path.relpath(p, root).replace(os.sep, '/'))
EOF
    echo "made: dist/QuickResult-$VERSION.zip"
    ;;
install)
    [ -n "$RECOMP" ] || { echo "error: set RECOMP to the recomp's folder (the one with mods/), e.g. RECOMP=/d/MHGURecomp"; exit 1; }
    build src/quickresult.c quickresult
    make_mod "$RECOMP/mods/QuickResult"
    echo "installed: $RECOMP/mods/QuickResult"
    ;;
"")
    build src/quickresult.c quickresult
    ;;
*)
    echo "usage: sh build.sh [release|install|diag]"; exit 1
    ;;
esac
