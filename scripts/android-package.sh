#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
abi=${1:-armeabi-v7a}
case "$abi" in armeabi-v7a|arm64-v8a) ;; *) echo "Unsupported ABI" >&2; exit 2;; esac
# Explicitly enrolled stable signer only. Do not create or guess a keystore.
for key in ANDROID_KEYSTORE ANDROID_KEYSTORE_TYPE ANDROID_KEY_ALIAS \
           ANDROID_STORE_PASSWORD ANDROID_KEY_PASSWORD ANDROID_EXPECTED_CERT_SHA256; do
    [[ -n ${!key:-} ]] || { echo "Missing enrolled signing input: $key" >&2; exit 1; }
done
bash "$root/scripts/dependencies.sh"
deps=${SWAP_DEPS_DIR:-$root/.deps}
export ANDROID_PACKAGE_ID=org.isomorphismes.swap.controls ANDROID_EXPECTED_LABEL="Swap Controls"
export ANDROID_VERSION_CODE=1 ANDROID_VERSION_NAME=0.1.0
export ANDROID_MIN_SDK=21 ANDROID_TARGET_SDK=35 ANDROID_REQUIRE_NO_DEX=1
export ANDROID_SOURCE_COMMIT
ANDROID_SOURCE_COMMIT=$(git -C "$root" rev-parse HEAD)
[[ -z $(git -C "$root" status --porcelain --untracked-files=all) ]] || {
    echo "Refusing to package a dirty source checkout" >&2; exit 1;
}
receipt="$root/build/android-$abi/inputs.sha256"
[[ -f "$receipt" ]] || { echo "Build receipt is missing; rebuild the native library" >&2; exit 1; }
(cd "$root"; sha256sum --check --strict "$receipt") || {
    echo "Native source, dependency pins or library changed; rebuild before packaging" >&2; exit 1;
}
bash "$deps/android-NDK/apk/build-nativeactivity-apk.sh" \
    "$root/android/AndroidManifest.xml" "$root/build/android-$abi/libswap.so" \
    "$abi" "$root/dist/swap-$abi.apk"
printf '%s\n' 'Candidate only: shared AICI producer/version/update approval is still required before publication.'
