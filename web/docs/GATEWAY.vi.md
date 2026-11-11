# Gateway local — W0012, 2026-10-23

Đã bắt đầu M03 bằng gateway loopback riêng tại `web/gateway`, dùng ws8.22.0 với package-lock riêng. Không cài dependency này vào frontend hoặc dist. [Cây thư mục, cấu hình và lệnh tái lập](../gateway/README.vi.md).

Kết quả: gateway test 7/7 qua bằng TCP/WebSocket local thực, gồm full-byte passthrough, isolation, route/Origin/cap rejection, binary-only/maxPayload, TCP EOF, budget/idle, transport+WASM integration và heartbeat. Các listener test được đóng ở cuối; không giữ gateway fixture chạy nền.

Kiểm tra liên quan: npm frontend/core test10/10 qua; test:wasm qua. Output Debug/Release vẫn qua allowlist, không chứa ws/node_modules/config. Build package frontend không tự mở gateway. Ping overlay vẫn chưa kết nối, vì chưa có MU session thực.

Gateway không sửa cipher/serial, packet hoặc gameplay. Không đoán server code hay tự dùng IP/port trả về ConnectServer làm route. Chưa đổi config MU hoặc bật MU server/DB.

M03 **PARTIAL**: chưa WSS ngoài local, identity/auth, discovery/login, route theo server code thật, packet fixture đã loại credential, perf/load/soak và slow-client stress. M02 crypto/state/ClientVerify vẫn mở. Bước sau: đối chiếu source codec/transport cipher và route ConnectServer trước khi thử discovery thật; không bỏ kiểm tra server để login được.
