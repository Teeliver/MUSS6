# W0019 — Pipeline packet GameServer

## Phạm vi

`client/src/session/game-channel.js` ghép các adapter core đã kiểm chứng W0013–W0018. Đây là kênh packet cấp thấp, chưa phải state machine đăng nhập hay game online. `open` chỉ có nghĩa WebSocket đã mở; không phải greeting accepted, ClientVerify hoặc authentication.

- Gửi: packet XOR / SimpleModulus + serial → transport transform/MHP → WebSocket.
- Nhận: WebSocket → transport decode → frame reader → SimpleModulus/check serial → callback packet C1/C2.
- Không tự reverse packet XOR inbound của server. Plain inbound giữ nguyên serial.
- Route được kiểm chứng code/address/port qua allowlist trước khi tạo socket; không nhận URL GameServer tùy ý từ packet. Profile và hai key-file 54 byte phải cấp rõ ràng.
- Kênh sở hữu bản sao key JS; xóa bản sao sau connect hoặc close. Core cipher/reader được tạo riêng từng phiên và giải phóng khi phiên kết thúc. Không cam kết zero toàn bộ WASM heap.
- Không reconnect/retry packet sau lỗi serial/checksum. Thừa hưởng deadline partial frame, queue cap và cleanup của TransportSession. Không có greeting deadline ở adapter cấp thấp này: caller state machine phải quản lý deadline từng phase như game-probe hiện có.

## Kiểm thử

Lệnh từ Emulator:

```sh
npm --prefix web test
npm --prefix web run test:wasm
```

25/25 test qua, không skip; WASM contract/bridge qua. Test mới dùng WASM thật, profile/key tổng hợp và socket EventTarget mô phỏng, không phải WS/TCP network hoặc browser MU.

Kiểm tra mọi điểm chia của một frame mã hóa, đúng byte outbound sau cả hai lớp; nhiều plain frame cùng chunk; replay serial; consumer exception; packet gửi sai length; khởi tạo key sai; close trước connect; timeout frame thiếu; hai phiên có serial độc lập. Kiểm tra timer/socket kết thúc và không phát thêm packet sau lỗi. Build Debug/Release kiểm allowlist/cú pháp có module mới; UI giữ nguyên.

Reference block crypto đã đối chiếu trực tiếp với source Main ở W0018; test W0019 chứng minh cách ghép layer, không thay thế kiểm chứng interoperability MU thật.

## TODO / chưa đạt

- Chưa state machine greeting → ClientVerify → login, chưa identity provider/provisioning key/profile thật.
- Chưa nối channel vào UI; chưa gửi account hoặc mở server/DB MU.
- Chưa test network gateway với channel mới, browser MU, capture thật, soak/load hoặc hiệu năng gameplay.
- M02/M03 vẫn PARTIAL. Bước kế tiếp: quản lý phase greeting/identity với deadline và trạng thái rõ ràng bằng fixture, trước khi nối login.
