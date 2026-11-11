# ConnectServer codec và transport cipher — W0013, 2026-10-23

## Nguồn và phạm vi

Đã đọc trực tiếp:

- ConnectServer/ConnectServerProtocol.h/.cpp: init C1:00, request F4:06/F4:03, list count big-endian, endpoint address16 + port WORD.
- ConnectServer/ServerList.cpp: record code WORD, load BYTE, type BYTE.
- Main_EX603/Main/HackCheck.cpp: EncryptData/DecryptData byte transform.
- GameServer/HackCheck.cpp: transform transport và lớp MHP tùy chọn.
- native/client/client.cpp: parse_list, handshake, connect_game và encode/decode_transport hiện tại.

Đã thêm core thuần `client-core/include/mu/connect_protocol.hpp`, WASM checked-result binding và adapter `client/src/protocol/connect-codec.js`. Core không đọc config, file key, serial, account hoặc socket.

## Đã triển khai

- Init ConnectServer chỉ chấp nhận C1 04 00 01.
- Tạo list request C1 04 F4 06 và endpoint request C1 05 F4 03 <code>.
- List C2 F4:06: frame length/count big-endian; từng server code little-endian, load/type giữ nguyên. Kiểm tra length/count, duplicate code và bounds; danh sách rỗng hợp lệ.
- List có thể mang code16-bit nhưng request chọn server của source này chỉ có 1 byte. Endpoint request từ chối code>255, không cắt byte âm thầm.
- Endpoint C1 F4:03 đúng22 byte, address16 có NUL, port little-endian khác0. Address được trả như metadata, không mở TCP theo trường này.
- `server-route.js` chỉ ánh xạ code đã cấu hình sang `/mu/<route>` khi address/port khớp cấu hình. Không tự dùng endpoint trả về làm URL/TCP target.
- Transport encode/decode: byte arithmetic modulo256, base transform rồi MHP khi encode; đảo MHP trước base khi decode. Keys phải cấp rõ ràng, kiểm tra byte range; không có keys mặc định hoặc profile giả.

Đây **không phải** C3/C4 SimpleModulus, XOR packet filter, credential obfuscation, key derivation hoặc ClientVerify. ConnectServer trong native hiện tại dùng plain transport; không áp GameServer cipher lên ConnectServer. Giá trị key trong test là fixture tổng hợp, không đọc key/serial runtime thật hoặc đưa vào dist.

## Đối chiếu và kiểm thử

Native contract và WASM contract dùng cùng `connect_protocol_contract.cpp`. Với test build, `transport-reference.mjs` lấy đúng thân 4 hàm từ GameServer/HackCheck.cpp vào header tạm, bổ sung tối thiểu BYTE/global fixture trong namespace riêng. So sánh output core với **hàm server được cung cấp**, không chỉ encode/decode roundtrip. Không link server startup hoặc đọc cấu hình game.

Fixture gồm tất cả256 giá trị byte, nhiều bộ keys có/không MHP, mọi ranh giới cắt stream, một vector golden5 byte, packet list hai record/code256, empty list, duplicate/count sai, từng độ dài truncated, endpoint sai và code chọn vượt255. JS-WASM còn kiểm tra checked errors và route allowlist ở Node.

Lệnh:

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Các test không mở server/DB. Core/adapter chưa tự được nối vào app; Debug/Release vẫn cùng layout W0010 và ping chưa kết nối. Contract native/WASM không thay thế packet capture và discovery thật.

## Chưa hoàn tất

Chưa derive profile keys từ metadata phù hợp browser, load Enc1/Dec2, SimpleModulus/XOR, serial state, identity/ClientVerify và login. Route helper mới là kiểm tra cục bộ; vẫn cần gateway cấu hình ConnectServer và game routes thật, packet fixture có bằng chứng runtime. Chưa mở MU server hoặc thay config/Data/native production. M02/M03 vẫn PARTIAL.

Bước sau: state machine discovery bất đồng bộ qua gateway dùng fixture protocol đầy đủ; chỉ sau đó mới cân nhắc discovery MU thật. Không bỏ authentication hoặc HackCheck để làm tiến trình chạy qua.

Kết quả thực W0013: WASM build qua; npm test **12/12 qua**, không skip; test:wasm qua cả framing contract và connect-protocol contract, gồm source-reference comparison. Kiểm tra output Debug/Release tiếp tục qua. Chưa test discovery/login trên server thật.
