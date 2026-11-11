#!/usr/bin/env python3
"""Start existing, already-built local MU Web services; never rebuild or alter DB."""
import os
from pathlib import Path
import socket
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
LOGS = ROOT / 'native/runtime/background'
NODE = '/opt/homebrew/bin/node'
DATA = '/Users/angelale/Downloads/MuClientEX603/Data'
SERVICES = [
    ('ConnectServer', 44405, [str(ROOT/'native/Start-ConnectServer.command')]),
    ('JoinServer', 55970, [str(ROOT/'native/Start-JoinServer.command')]),
    ('DataServer', 55960, [str(ROOT/'native/Start-DataServer.command')]),
    ('GameServer', 55900, [str(ROOT/'native/Start-GameServer.command')]),
    ('Identity', 9001, [NODE, str(ROOT/'web/gateway/src/identity-main.mjs'), str(ROOT/'web/.build/identity/mu-native-identity'), DATA]),
    ('Gateway', 9000, [NODE, str(ROOT/'web/gateway/src/main.mjs'), str(ROOT/'web/gateway/config.local.json')]),
    ('Web', 8080, ['/usr/bin/python3', '-m', 'http.server', '8080', '--bind', '127.0.0.1', '--directory', str(ROOT/'web/dist/debug')]),
]

def listening(port):
    try:
        with socket.create_connection(('127.0.0.1', port), timeout=.3):
            return True
    except PermissionError:
        raise RuntimeError('Môi trường này không có quyền kiểm tra cổng. Chạy launcher trong Terminal của macOS.')
    except OSError:
        return False

def main():
    action = sys.argv[1] if len(sys.argv)>1 else 'start'
    if action not in ('start', 'status', 'check'):
        print('Cách dùng: native/MU-Web.command [start|status|check]')
        return 2
    if action == 'status':
        for name, port in [('MySQL', 3306)] + [(n,p) for n,p,_ in SERVICES]:
            print(f'{name:15} {port:5}  '+('ĐANG NGHE' if listening(port) else 'CHƯA CHẠY'))
        return 0
    required = ['native/build/connect/ConnectServer-native','native/build/support/JoinServer-native','native/build/support/DataServer-native','native/build/game/GameServer-native','web/dist/debug/index.html','web/dist/debug/runtime/live-profile.json','web/gateway/config.local.json','web/gateway/node_modules/ws/package.json','web/.build/identity/mu-native-identity']
    missing = [p for p in required if not (ROOT/p).exists()]
    if missing:
        print('Thiếu file đã build/cấu hình: '+', '.join(missing))
        return 1
    if not Path(NODE).exists() or not Path(DATA).is_dir():
        print('Không tìm thấy Node hoặc thư mục Data client trên máy này.')
        return 1
    if action == 'check':
        print('Đủ executable, frontend, Gateway, Identity và Data; chưa khởi động dịch vụ.')
        return 0
    LOGS.mkdir(parents=True, exist_ok=True)
    if not listening(3306):
        print('Đang bật MySQL; cần Docker Desktop đang chạy.', flush=True)
        if subprocess.call([str(ROOT/'native/Start-MySQL.command')], cwd=ROOT):
            print('MySQL chưa sẵn sàng. Mở Docker Desktop rồi chạy lại.')
            return 1
    for name, port, command in SERVICES:
        if listening(port):
            print(f'{name}: cổng {port} đã có dịch vụ, giữ nguyên.', flush=True)
            continue
        log = LOGS/(name+'.log')
        with log.open('ab') as output:
            process = subprocess.Popen(command, cwd=ROOT, stdin=subprocess.DEVNULL, stdout=output, stderr=subprocess.STDOUT, start_new_session=True, env={**os.environ,'PATH':'/opt/homebrew/bin:'+os.environ.get('PATH','')})
        (LOGS/(name+'.pid')).write_text(str(process.pid)+'\n')
        deadline = time.monotonic()+60
        while not listening(port):
            if process.poll() is not None or time.monotonic()>deadline:
                print(f'{name} chưa sẵn sàng. Xem log: {log}')
                return 1
            time.sleep(.25)
        print(f'{name}: đã mở cổng {port}.', flush=True)
    print('\nMở game: http://127.0.0.1:8080/?scene=servers\nCác dịch vụ chạy nền; có thể đóng Terminal.\nCổng mở chưa thay thế kiểm tra đăng nhập/vào nhân vật.')
    return 0

if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
