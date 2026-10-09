#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
: "${IDRIC_CHECKOUT:?Set IDRIC_CHECKOUT to the exact qualified Idric checkout}"
compiler_root=$(cd -- "$IDRIC_CHECKOUT" && pwd)
expected=94dfd99bd3e376507fedc8611053b7173b2519f0
actual=$(git -C "$compiler_root" rev-parse HEAD)
[[ "$actual" == "$expected" ]] || { echo "Wrong Idric revision: $actual" >&2; exit 1; }
compiler="$compiler_root/_/build/exec/idris2"
[[ -x "$compiler" ]] || { echo 'Pinned compiler has not been bootstrapped' >&2; exit 1; }
libs="$compiler_root/_/libs"
export IDRIS2_PATH="$libs/prelude/build/ttc:$libs/base/build/ttc:$libs/linear/build/ttc"
export IDRIS2_LIBS="$compiler_root/_/support/c"
export LD_LIBRARY_PATH="$compiler_root/_/support/c${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$root"
"$compiler" --cg chez --build raw-swap.ipkg
./build/exec/raw-swap-response-tests
printf 'SOURCE_REVISION\t%s\nIDRIC_REVISION\t%s\nBOUNDARY\thost reference raster only\n' \
  "$(git -C "$root" rev-parse HEAD)" "$actual"
