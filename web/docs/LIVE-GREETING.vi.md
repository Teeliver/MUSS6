# W0056 — GameServer greeting thật (2026-10-25)

Đã bật MySQL, JoinServer, DataServer và GameServer trên loopback. Trước startup đã backup MuOnline vào native/runtime/backups/MuOnline-before-web-20261025-213022.sql (0600, 120610 byte), gồm routines/triggers/events, không in nội dung ra log.

Browser nhận GameServer_0 từ ConnectServer, chọn server và giải mã greeting serverCode0 qua gateway. Probe đóng phiên sau greeting; chưa ClientVerify, login hoặc world. 102/102 npm tests và build Debug/Release qua; staging179 asset.

User đã đồng ý cấp ClientSerial và hai ServerEncDecKey cho Debug local. stage_live_profile.py chỉ lấy đúng ba trường, validate và ghi dist/debug/runtime/live-profile.json; không copy DB/account/HWID hoặc đưa vào Release. Sau build cần stage lại bằng python3 web/tools/stage_live_profile.py --client native/config/client.ini. Không phục vụ file này ra public. Auto-review ban đầu từ chối trước xác nhận; sau xác nhận việc cấp profile được chấp thuận.

Đang chạy trên127.0.0.1: MySQL3306, Connect44405/UDP55557, Join55970, Data55960, Game55900, gateway9000, static8080. Game log xác nhận world initialized, Join=1/Data=1. Chưa xác minh toàn bộ procedure/gameplay DB. Dừng Game -> Data -> Join -> Connect bằng Ctrl+C ở từng tiến trình; gateway/static riêng. Không tự dừng MySQL hoặc xóa container/Data.

PARTIAL: chưa identity provider macOS cho browser, account/login/character/world snapshot/GUI scene gốc. Không random HWID hoặc tắt ClientVerify. M03/M06 và skill/Windows visual parity vẫn chưa hoàn tất. Bước tiếp theo là provider theo derivation native, ràng buộc origin/lifecycle; không đưa HWID vào staticfiles.
