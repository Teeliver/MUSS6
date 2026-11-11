# Discovery/chọn server bất đồng bộ — W0014, 2026-10-23

Đã thêm `web/client/src/session/discovery.js`. Đây là tầng điều phối phiên, không phải UI chọn server và không gọi trực tiếp TCP từ browser.

## API và quyền sở hữu

`createDiscovery({core, createSocket, connectUrl, routes, timers, deadlineMs})` nhận core WASM đã load, factory WebSocket và URL route gateway cố định. Không nhận địa chỉ ConnectServer trực tiếp hoặc query host/port. Routes được copy/freeze tại lúc tạo để thay config bên ngoài giữa request không đổi endpoint được chấp nhận.

- `discover({signal})`: mở phiên CS riêng, đợi WS open → xác minh init C1 04 00 01 → gửi F4:06 → parse list → đóng phiên và trả danh sách.
- `select(code,{signal})`: kiểm tra code có cấu hình và vừa1 byte **trước khi tạo socket**; mở phiên CS mới → init → gửi F4:03 → parse endpoint → đối chiếu route → đóng CS và trả `{code, endpoint, routePath, gameUrl}`.
- `gameUrl` luôn dùng origin gateway từ cấu hình và path route đã kiểm chứng. Endpoint server trả về không tự trở thành đích TCP. Hàm select **không tự mở GameServer**.

Mỗi operation có reader/socket/timer riêng, có thể chạy độc lập. Abort trước khi bắt đầu không tạo socket. Abort đang chờ, lỗi handshake/packet/route, peer close hoặc deadline đều reject promise và dọn phiên/timer. Mỗi phase chờ handshake/response có deadline5s mặc định; frame chia nhỏ vẫn bị giới hạn bởi partial deadline transport W0011. Không polling bận hoặc vòng receive blocking.

ConnectServer dùng plain transport theo source native/server đã đọc W0013. Chỉ lớp CS này cấp identity adapter có chủ đích; không dùng nó để bỏ cipher GameServer. Không đọc credential/profile hoặc tự giảm ClientVerify.

## Kiểm chứng

```sh
npm --prefix web test
npm --prefix web run test:wasm
npm --prefix web run test:gateway
```

- Frontend/core: **16/16 qua**, không skip; gồm4 nhóm discovery dùng WASM thật, socket/clock mô phỏng.
- WASM contract/codec/reference checks tiếp tục qua.
- Gateway: **9/9 qua**, không skip; thêm2 nhóm discovery dùng WS/TCP loopback thật và ConnectServer fixture.

Fixture network đóng vai CS: gửi init chia byte, đọc request bằng core reader, gửi list2 record hoặc endpoint22 byte. Test khẳng định list và select dùng đúng2 phiên CS, request đúng thứ tự, endpoint sai bị từ chối, handshake im lặng timeout và code chưa cấu hình không mở phiên. Không có GameServer route được kết nối trong fixture này.

Test mô phỏng thêm danh sách trùng code, header/init sai, byte chia/ghép, config bị thay bên ngoài, hủy request, đóng từ xa, timeout phase và hai operation độc lập. Gateway/fixture được đóng sau test. Debug/Release vẫn qua allowlist; chưa khởi tạo discovery từ entrypoint app nên giao diện và ping giữ nguyên.

## Chưa hoàn tất

Chưa chạy browser UI discovery, menu chọn server gốc hoặc server MU thật. Chưa có runtime config chuẩn để kết nối CS/game, key derivation, C3/C4/SimpleModulus, ClientVerify/identity, game handshake/login, account/character state hoặc WSS ngoài local. M02/M03 vẫn PARTIAL. Không bật MU server/DB, không thay source native/Data/config game.

Bước sau: session GameServer handshake và profile/key derivation có fixture dựa trên source; chỉ thử server thật khi ranh giới identity/cipher đầy đủ và ghi rõ phạm vi test.
