#!/usr/bin/env bash

set -euo pipefail

formatter="${CLANG_FORMAT_BIN:-clang-format-19}"
mode="${1:-apply}"

mapfile -d '' -t files < <(
    git ls-files -z -- \
        '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' '*.inl'
)

if [[ "${#files[@]}" -eq 0 ]]; then
    echo "No tracked C/C++ files found."
    exit 0
fi

echo "Found ${#files[@]} tracked C/C++ files."
"$formatter" --version

case "$mode" in
    apply)
        printf '%s\0' "${files[@]}" | xargs -0 -r -n 100 "$formatter" -i
        ;;
    check)
        printf '%s\0' "${files[@]}" | xargs -0 -r -n 100 "$formatter" --dry-run --Werror
        ;;
    *)
        echo "Unknown mode: $mode" >&2
        exit 1
        ;;
esac