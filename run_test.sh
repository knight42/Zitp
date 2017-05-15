#!/bin/bash

HERE=$(realpath "$0")
HERE=$(dirname "$HERE")
name="$1"
prog="$HERE/build/zitp"
p="$HERE/tests/$name"
temp=$(mktemp)
trap 'rm -f $temp' EXIT

"$prog" -i "$p/input.txt" -p "$p/program.txt" -o "$temp" >/dev/null

if [[ $? -ne 0 ]]; then
    echo >&2 "Failed: $name"
    exit 1
fi

if ! diff -q "$p/output.expected" "$temp" >/dev/null
then
    echo >&2 "Failed: $name"
    exit 1
fi

exit 0
