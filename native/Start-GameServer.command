#!/bin/zsh
set -eu
native_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
server="$native_dir/build/game/GameServer-native"
if [[ ! -x "$server" ]]; then
  print -u2 "GameServer-native chưa được build. Xem $native_dir/README.md"
  exit 1
fi
exec "$server" --mysql-config "$native_dir/mysql.ini" "$@"
