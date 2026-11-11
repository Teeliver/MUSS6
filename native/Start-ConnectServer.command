#!/bin/zsh
set -eu
native_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
server="$native_dir/build/connect/ConnectServer-native"
if [[ ! -x "$server" ]]; then
  print -u2 "ConnectServer-native chưa được build. Xem $native_dir/README.md"
  exit 1
fi
exec "$server" --config "$native_dir/config/connect" "$@"
