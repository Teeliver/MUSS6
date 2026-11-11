#!/bin/zsh
set -eu
native_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
exec /usr/bin/python3 "$native_dir/tools/mu_web_stack.py" "$@"
