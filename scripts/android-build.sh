#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
source "$root/DEPS.lock"
abi=${1:-armeabi-v7a}
case "$abi" in
    armeabi-v7a)
        target=armv7a-linux-androideabi; gnu_target=arm-linux-gnueabi
        header_target=arm-linux-androideabi
        flags=(-marm -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp) ;;
    arm64-v8a)
        target=aarch64-linux-android; gnu_target=aarch64-linux-gnu
        header_target=aarch64-linux-android; flags=(-ffixed-x18) ;;
    *) echo "Unsupported ABI: $abi" >&2; exit 2 ;;
esac
sdk=${ANDROID_HOME:-${ANDROID_SDK_ROOT:-}}
ndk=${ANDROID_NDK_HOME:-${sdk:+$sdk/ndk/$ANDROID_NDK_VERSION}}
[[ -n "$ndk" && -f "$ndk/source.properties" ]] || {
    echo "Install Android NDK $ANDROID_NDK_VERSION and set ANDROID_NDK_HOME" >&2; exit 1;
}
observed=$(sed -n 's/^Pkg.Revision[[:space:]]*=[[:space:]]*//p' "$ndk/source.properties" | tr -d '\r')
[[ "$observed" == "$ANDROID_NDK_VERSION" ]] || {
    echo "Expected NDK $ANDROID_NDK_VERSION, got $observed" >&2; exit 1;
}
[[ $(uname -s) == Linux && $(uname -m) == x86_64 ]] || {
    echo 'This build entry is restricted to Linux x86_64 hosts' >&2; exit 1;
}
: "${ICK_CC:?Set ICK_CC from the pinned ai-ci/ick-android qualification}"
[[ -x $ICK_CC && $("$ICK_CC" -dumpmachine) == "$gnu_target" ]] || {
    echo "Wrong ICK target; expected $gnu_target" >&2; exit 1;
}
bash "$root/scripts/dependencies.sh"
deps=${SWAP_DEPS_DIR:-$root/.deps}
build="$root/build/android-$abi"
mkdir -p "$build"
toolchain="$ndk/toolchains/llvm/prebuilt/linux-x86_64"
sysroot="$toolchain/sysroot"
clang="$toolchain/bin/${target}21-clang"
builtin=$("$ICK_CC" -print-file-name=include)
[[ -f "$builtin/stddef.h" && -x "$clang" ]] || exit 1
common=(-std=c11 -O2 -fPIC -DNDEBUG -DSOKOL_GLES3
    -I "$root/src" -I "$deps/sokol"
    --sysroot="$sysroot" -nostdinc -isystem "$builtin"
    -isystem "$sysroot/usr/include" -isystem "$sysroot/usr/include/$header_target"
    -D__ANDROID__ -D__ANDROID_API__=21 -D__ANDROID_MIN_SDK_VERSION__=21
    -DBIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD)
objects=()
for name in swap input views permsix app; do
    warnings=(-Wall -Wextra)
    # Keep owned domain code strict without promoting third-party Sokol
    # implementation warnings to model defects.
    [[ $name != swap ]] || warnings+=(-Wpedantic -Werror)
    "$ICK_CC" "${flags[@]}" "${common[@]}" "${warnings[@]}" \
        -S "$root/src/$name.c" -o "$build/$name.s"
    "$clang" "${flags[@]}" -fPIC -c "$build/$name.s" -o "$build/$name.o"
    objects+=("$build/$name.o")
done
"$clang" -shared "${objects[@]}" -Wl,--no-undefined -Wl,-soname,libswap.so \
    -Wl,-z,max-page-size=16384 -landroid -llog -lEGL -lGLESv3 -lm -o "$build/libswap.so"
"$toolchain/bin/llvm-strip" --strip-unneeded "$build/libswap.so"
"$toolchain/bin/llvm-readelf" --dyn-syms "$build/libswap.so" > "$build/symbols.txt"
grep -E 'GLOBAL.*DEFAULT.*ANativeActivity_onCreate$' "$build/symbols.txt"
# Bind a candidate to its exact source, build recipe, dependency pins and bytes.
(cd "$root"; sha256sum DEPS.lock src/swap.h src/swap.c src/app.c \
    src/input.h src/input.c src/views.h src/views.c src/permsix.h src/permsix.c scripts/android-build.sh android/AndroidManifest.xml \
    "build/android-$abi/libswap.so") > "$build/inputs.sha256"
printf 'ICK_C_REVISION\t%s\nNDK_VERSION\t%s\n' "$ICK_REVISION" "$ANDROID_NDK_VERSION"
printf 'ICK_GAP\tAndroid platform assembly/link/runtime supplied by NDK, not ICK\n'
printf 'Native library (not an APK): %s/libswap.so\n' "$build"
