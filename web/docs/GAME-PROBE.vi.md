# GameServer greeting probe mã hóa — W0016, 2026-10-23

Đã thêm `web/client/src/session/game-probe.js`. Đây là probe **chỉ đọc greeting rồi đóng**, không phải đăng nhập hoặc session gameplay lâu dài. Không khởi tạo từ app Debug/Release.

## Luồng

`createGameProbe({core,createSocket,gatewayUrl,routes,timers,deadlineMs})` tạo hàm `probe(selected,profile,{signal})`.

1. Đối chiếu lại code/address/port đã chọn với snapshot cấu hình; bỏ qua trường gameUrl do caller tự đưa. Dựng URL từ origin gateway và route đã xác minh.
2. Derive keys từ profile explicit bằng core W0015. Profile sai/endpoint chưa liệt kê bị từ chối trước khi mở socket.
3. Mở phiên riêng; transport decode byte **trước** khi core framing, giữ đúng thứ tự khi chunk cipher bị chia.
4. Kiểm greeting EXTRA1, result và server code; trả index/serverCode/encodedVersion thô, rồi đóng phiên và dọn reader/timer/listener AbortSignal.

Không gửi byte application/login/ClientVerify, không tự gửi lại hoặc thử key khác. Deadline mặc định5s sau WS open cho greeting, cùng connect/partial deadline transport. Abort trước khi mở không tạo socket; abort đang chờ, peer close, packet lỗi hoặc timeout đều reject và đóng phiên. Mỗi probe độc lập.

Key sai không nhất thiết lập tức tạo header lỗi: byte giải mã có thể trông giống frame hợp lệ nhưng chưa đủ. Khi đó probe chờ tới deadline thay vì đoán packet hay chặn UI. Error không echo key/profile/packet vào log.

## Kiểm thử

```sh
npm --prefix web test
npm --prefix web run test:wasm
npm --prefix web run test:gateway
```

- Frontend/core **20/20 qua**, không skip: thêm4 nhóm game-probe dùng core WASM thật và socket/clock mô phỏng. Greeting14 byte được chia ở cả15 vị trí; output đúng index11900/code1/version raw; không gửi byte login. Kiểm server result từ chối, code sai, key sai, route/profile sai, config snapshot, abort, deadline và EOF dở.
- Gateway/network **12/12 qua**, không skip: thêm3 nhóm probe qua WS/TCP loopback thật. Fixture greeting mã hóa bằng derive keys, MHP tắt/bật; sai result/code/key và server im lặng đều bị từ chối. Fixture đếm byte client gửi tới TCP và xác nhận **0 byte application**.
- test:wasm/source-reference/greeting W0015 tiếp tục qua. Node test đầu tiên cần chỉnh ca wrong-key để chạy clock mô phỏng tới deadline; sau sửa đã chạy lại20/20. Đã dừng process test cũ chờ clock, không để lại test treo.

Tất cả gateway/TCP fixture tự đóng sau test, không phải MU server thật hoặc test browser GameServer network. Debug/Release vẫn qua build allowlist, không có UI mới và ping vẫn chưa kết nối.

## Chưa hoàn tất

Chưa profile provisioning runtime, identity/ClientVerify, packet XOR/SimpleModulus C3/C4, serial/account login, character/gameplay hoặc đo ping MU. Greeting accepted chỉ nghĩa là fixture gửi result1 và đúng layout/code, **không chứng minh authentication/version hợp lệ**. Version giữ encoded bytes chưa tự so chuỗi.

M02/M03 vẫn PARTIAL. Không bật MU server/DB, không dùng serial/tài khoản thật, không sửa Data/config/native production. Bước sau là rà source ClientVerify và thiết kế identity browser có bằng chứng trước khi mở luồng account/login, không tắt kiểm tra server.
