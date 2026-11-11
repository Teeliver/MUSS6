#!/bin/zsh
set -eu
native_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
app="$native_dir/build/client/MuClient-viewer.app"
if [[ ! -d "$app" ]]; then
 print -u2 "Renderer chưa được build. Chạy cmake --build native/build --target MuClient-viewer"
 exit 1
fi
exec /usr/bin/open "$app"
