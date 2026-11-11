# Gateway local — W0012

Chỉ chạy loopback. Chuyển byte nhị phân WebSocket ↔ TCP theo route cấu hình cố định; không giải mã packet, không thay gameplay và không truy cập DB.

```text
gateway/
  src/gateway.mjs       library: listener, routes, lifecycle, queues, heartbeat
  src/main.mjs          CLI + shutdown
  tests/                WS/TCP fixture và transport/WASM integration
  config.example.json   cấu hình mẫu, không trỏ MU server thật
  config.local.json     cấu hình máy riêng, bị ignore
  package.json          ws 8.22.0 pin chính xác
  package-lock.json     lockfile của gateway, không dùng cho frontend
```

Từ Emulator:

```sh
npm --prefix web/gateway ci --ignore-scripts --omit=optional
npm --prefix web run test:gateway
```

Test cần quyền mở socket loopback và WASM đã build (`npm --prefix web run build:wasm`). Các TCP server/gateway fixture tự đóng khi test kết thúc. Không chạy MU server hoặc MySQL.

Để chạy riêng, tạo `web/gateway/config.local.json` dựa trên config.example.json, chỉ điền route tới dịch vụ fixture/local đã biết; rồi:

```sh
npm --prefix web/gateway start -- config.local.json
```

Lệnh trên chạy từ npm package gateway nên config.local.json nằm trong thư mục gateway. Chưa tạo config.local.json thật hoặc tự chạy gateway lâu dài. Cấu hình mẫu route fixture tới port9500 không tự tạo TCP server ở port đó.

Endpoint duy nhất dạng `/mu/<route>`, không query/host/port từ browser. Listener luôn 127.0.0.1, upstream chỉ nhận địa chỉ IP loopback chính xác 127.0.0.1 hoặc ::1, không resolve hostname ngoài. Origin phải khớp allowlist. Đây không phải cơ chế đăng nhập hoặc triển khai public; WSS/auth/Origin ở proxy production cần thiết kế trước khi triển khai ngoài máy.

Mỗi WS có một TCP upstream. Không gộp account/serial/crypto. Nén WebSocket tắt. Mặc định: 16 phiên, payload64KiB, queue256KiB, budget256KiB/giây theo từng chiều/cửa sổ1s, connect timeout5s, idle TCP60s, heartbeat10s. Khi TCP write báo backpressure thì pause WS; khi gửi TCP chunk sang WS thì pause TCP cho đến callback gửi. Hàng đợi vượt giới hạn, text frame, lỗi hoặc timeout đều đóng phiên; heartbeat yêu cầu pong, không chèn control byte vào stream MU.

Nguồn thư viện và API: [ws chính thức](https://github.com/websockets/ws), [tài liệu API](https://github.com/websockets/ws/blob/master/doc/ws.md). Đã kiểm tra registry npm chính thức khi chọn 8.22.0. Không cài optional native addon hoặc chạy install scripts. npm install báo 0 vulnerability tại thời điểm cài, không coi là bảo đảm an toàn mãi.

## Đã kiểm chứng / chưa kiểm chứng

7/7 test gateway qua:

- Byte stream đủ mọi giá trị byte qua WS/TCP, chunk chia/ghép và upstream riêng cho 2 client.
- Origin sai, route lạ/query và vượt số phiên bị từ chối trước mở upstream.
- Text/oversized payload, TCP EOF, budget byte và idle timeout.
- Cấu hình remote TCP sai bị từ chối; listener không có tùy chọn bind public.
- TransportSession + checked-reader WASM thật + WS/TCP fixture qua cùng chuỗi end-to-end.
- Client không pong bị đóng bởi heartbeat.

Đây là Node WS client và TCP fixture thật trên loopback, không phải UI browser network test hoặc server MU. Identity cipher chỉ dùng trong fixture một cách tường minh, không dùng để login. Chưa capture packet/login thực, kiểm tra ClientVerify, routing ConnectServer/GameServer, đo ping MU, TLS/WSS deployment, soak/load hoặc slow-client stress dài hạn. Connect timeout và queue cap đã có code; không gọi các branch chưa stress riêng là đã được chứng minh chịu tải.
