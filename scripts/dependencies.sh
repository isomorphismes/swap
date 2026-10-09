#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
source "$root/DEPS.lock"
deps=${SWAP_DEPS_DIR:-$root/.deps}
mkdir -p "$deps"
checkout() {
    local name=$1 url=$2 revision=$3 path="$deps/$1"
    [[ $revision =~ ^[0-9a-f]{40}$ ]] || { echo "Invalid dependency revision" >&2; exit 1; }
    if [[ ! -e $path ]]; then
        git init -q "$path"
        git -C "$path" remote add origin "$url"
        git -C "$path" fetch --depth 1 origin "$revision"
        git -C "$path" checkout -q --detach FETCH_HEAD
    fi
    [[ $(git -C "$path" rev-parse HEAD) == "$revision" ]] || {
        echo "Wrong $name revision; refusing to replace an existing checkout" >&2; exit 1;
    }
    [[ -z $(git -C "$path" status --porcelain --untracked-files=all) ]] || {
        echo "Dirty $name dependency; refusing to use it" >&2; exit 1;
    }
}
checkout sokol https://github.com/floooh/sokol.git "$SOKOL_REVISION"
checkout android-NDK https://github.com/isomorphisms/android-NDK.git "$ANDROID_NDK_SUBSTRATE_REVISION"
