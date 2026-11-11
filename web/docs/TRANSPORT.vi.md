# Transport và vòng đời phiên — W0011, 2026-10-23

Đã thêm `client/src/platform/transport/session.js`, chưa tự mở kết nối từ app. Trang Debug/Release giữ nguyên canvas/overlay W0010; ping vẫn chưa kết nối.

## Ranh giới và cây thư mục

- `client-core/include/mu/frame_reader.hpp`: framing thuần C++, không I/O/timer.
- `client-core/frame_reader_bindings.cpp`: thêm feedResult/finishResult, bắt std::exception trong C++ rồi trả object kết quả cho JavaScript; giữ API cũ cho contract nội bộ.
- `client/src/platform/wasm/create-reader.js`: API transport-facing, chuyển kết quả lỗi thành Error JavaScript và hủy reader idempotent. Không để lỗi packet C++ đi trực tiếp qua ranh giới transport.
- `client/src/platform/transport/session.js`: quản lý WebSocket injected, timeout, hàng đợi, thứ tự byte và kết thúc phiên.
- `tests/transport.test.mjs`: socket EventTarget giả lập + core WASM thật + clock giả lập xác định. Không phải test gateway TCP hoặc server MU thật.

Không đưa logic codec vào diagnostics, không đổi source/Data native gốc. Các adapter được đóng gói ở cả hai mode nhưng chưa được app khởi tạo; diagnostics vẫn chỉ có trong Debug.

## Hành vi đã triển khai

Mỗi TransportSession có reader/socket riêng. State: idle → connecting → open → closed/failed. Phiên terminal không được tái sử dụng; reconnect phải tạo session mới. Hủy reader đúng một lần, xóa timer, đóng socket; sự kiện đến sau đóng bị bỏ qua.

WebSocket binaryType=arraybuffer; chỉ nhận dữ liệu binary. `decodeTransport` và `encodeTransport` **bắt buộc được cấp**, không có mặc định identity làm giả crypto. Thứ tự xử lý: decode transport bytes → core.feed → onFrame. Outbound encode → send; không assume một WS message là một MU frame. Các test dùng identity adapter một cách tường minh, không dùng cho login thật.

Deadline mặc định 5 giây cho kết nối và packet chưa đủ, theo thời lượng native đang dùng. Dữ liệu nhỏ giọt không reset timeout của cùng packet; khi hoàn thành frame và bắt đầu frame tiếp, deadline mới được tạo. Giới hạn chunk, byte còn chờ và hàng đợi gửi mặc định 262.144 byte. Không có hàng đợi gửi vô hạn hoặc tự retry packet.

Lỗi framing/decoder/consumer, EOF giữa frame, timeout và vượt hàng đợi đều kết thúc phiên. Không log byte packet, credentials hoặc nội dung lỗi consumer. Khi peer đóng ở ranh giới frame hợp lệ, phiên kết thúc closed. Khi người dùng hủy, không coi packet dở là một lỗi mới.

URL phải ws/wss; ws thường chỉ cho loopback. Đây là kiểm tra adapter browser, **không thay gateway allowlist/Origin/auth** của M03. Không có host/port TCP tùy ý hoặc truy cập DB ở đây.

## Kiểm chứng

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

- WASM bridge build thành công sau khi bổ sung checked-result API.
- npm test **10/10 qua**, không skip: native contract, build Debug/Release và 6 nhóm transport dùng WASM thật.
- test:wasm qua shared framing, bridge checks, safe-reader error/dispose và cùng contract C++.
- Bao phủ chia/ghép byte, phiên tách biệt, copy output, connection timeout, partial timeout không reset, EOF, malformed/nonbinary, giới hạn queue, decoder/callback lỗi và local cancel. Release vẫn qua kiểm tra không chứa diagnostics.

Chưa thực hiện WebSocket qua mạng, gateway TCP, crypto, ClientVerify, serial/session gameplay, ping server hoặc login. Không claim server lag/FPS đã được xử lý từ test socket giả lập. Không bật MU server/DB hoặc sửa Data/config/production.

## Bước kế tiếp

Triển khai gateway loopback với upstream route cố định và test bằng TCP/WebSocket local fixture trước. Sau đó mới nối cipher/identity được đối chiếu source và thử discovery/login thật. M02 vẫn PARTIAL; M03 chưa có gateway implementation.
