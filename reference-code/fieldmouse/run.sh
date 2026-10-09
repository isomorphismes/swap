#!/usr/bin/env bash
# Native Field Mouse executor for the Rough.js port.
# No Node or JavaScript interpreter is involved.
set -euo pipefail

here="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source_file="$here/demo.fm"
case "${1:-}" in
  --test)
    source_file="$here/tests.fm"
    shift
    ;;
  --parity)
    source_file="$here/parity.fm"
    shift
    ;;
  --script)
    if [[ $# -lt 2 ]]; then
      echo 'usage: run.sh --script FILE [args...]' >&2
      exit 64
    fi
    source_file="$2"
    shift 2
    ;;
esac

if [[ -n "${FIELD_MOUSE:-}" ]]; then
  executor="$FIELD_MOUSE"
elif command -v fieldmouse >/dev/null 2>&1; then
  executor="$(command -v fieldmouse)"
elif [[ -n "${FIELD_MOUSE_REPO:-}" &&
        -x "$FIELD_MOUSE_REPO/build/exec/fieldmouse" ]]; then
  executor="$FIELD_MOUSE_REPO/build/exec/fieldmouse"
else
  cat >&2 <<'EOF'
Field Mouse executable not found.
Build the real interpreter from https://github.com/dilapidated-shed/fieldmouse:
  idris2 --build fieldmouse.ipkg
Then set FIELD_MOUSE=/absolute/path/to/build/exec/fieldmouse
or FIELD_MOUSE_REPO=/absolute/path/to/fieldmouse.
EOF
  exit 127
fi

if [[ ! -f "$source_file" ]]; then
  echo "missing Field Mouse program: $source_file" >&2
  exit 66
fi

combined="$(mktemp "${TMPDIR:-/tmp}/swap-rough-XXXXXX.fm")"
trap 'rm -f -- "$combined"' EXIT
cat "$here/Rough.fm" "$here/Ellipse.fm" "$source_file" > "$combined"
"$executor" "$combined" "$@"
