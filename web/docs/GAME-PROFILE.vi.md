# GameServer key derivation và greeting — W0015, 2026-10-23

Đã thêm `client-core/include/mu/game_profile.hpp`, WASM checked-result functions và `client/src/protocol/game-profile.js`. Không đọc profile runtime/serial thật, không log hoặc nhúng credential/config game vào build.

## Nguồn trực tiếp

- GameServer/HackCheck.cpp InitHackCheck: CustomerName cố định MuOnline trong bộ server hiện có, trường32 byte; ServerSerial17 byte, gồm16 serial + NUL; tổng/tràn WORD và BYTE theo source; MHP raw keys chỉ được hiệu chỉnh khi có ít nhất một giá trị khác0.
- Main_EX603/Main/HackCheck.cpp và Protect.h: CustomerName32, ClientSerial17 và công thức base keys.
- GameServer/Protocol.h PMSG_CONNECT_CLIENT_SEND và Protocol.cpp GCConnectClientSend: F1:00, result, index big-endian, version5 byte, ServerCode WORD khi EXTRA1.
- native/client/client.cpp load_profile/parse_greeting: adapter hiện tại dùng MuOnline và EX603 EXTRA1 greeting14 byte.

## Implementation đã có

`deriveGameKeys(core,{serial,rawMhp1,rawMhp2})` suy ra TransportKeys cho transform W0013. Profile hiện hỗ trợ serial16 ký tự ASCII in được không có space, raw keys0..255; encoding khác ngoài phạm vi adapter này, không tự đổi Unicode thành byte gần giống. Không có serial/key mặc định và không tải key từ server cho tiện login.

Base keys tính đủ32 bước, lặp serial theo modulo17, không modulo16. Khi MHP tắt, trả flag false và hai effective MHP keys0 theo source server (adapter native cũ vẫn tính giá trị không dùng khi flag false; không khác transform byte). Khi bật, cộng tổng CustomerName rồi wrap byte. Custom CustomerName khác MuOnline chưa hỗ trợ cho bộ server này.

`parseGameGreeting(core,bytes,expectedCode)` yêu cầu đúng C1 0E F1 00, result1, đúng14 byte EXTRA1; index là big-endian, server code là little-endian. Phải khớp server code đã chọn; trả encodedVersion gồm5 byte gốc, không tự so chuỗi human-readable/version hoặc bỏ qua kết quả server từ chối.

Đây chỉ là key derivation transport và **parser greeting**, chưa có session mở GameServer, account/login, ClientVerify, C3/C4 SimpleModulus/XOR hoặc serial state. Chưa hỗ trợ greeting12 byte của build không EXTRA1. Không tự đổi build layout để nhận mọi packet.

## Kiểm chứng

`transport-reference.mjs` nay lấy thêm nguyên thân InitHackCheck từ GameServer/HackCheck.cpp vào fixture namespace, với BYTE/WORD và các helper Win32 tối thiểu. Không link server startup/config. Bọc cảnh báo signed/unsigned trong phần source-reference cũ; code core mới vẫn compile strict Wall/Wextra/Werror/pedantic.

- 3 serial giả lập ×4 cấu hình MHP =12 trường hợp so key suy ra với hàm source server.
- Transform roundtrip đủ256 giá trị byte theo key suy ra; các source transform comparison/golden và split checks W0013 tiếp tục qua.
- Greeting fixture index11900/serverCode4660, đúng endian/version, từ chối từng độ dài cắt và result từ chối.
- JS-WASM kiểm tra key API, invalid profile, greeting và code không khớp.

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Kết quả: WASM build qua; npm test **16/16 qua**, không skip; test:wasm qua framing, protocol/source-reference, derivation và greeting. Chưa network GameServer handshake hoặc browser probe. Gateway không đổi trong đợt này.

## Tiếp theo / giới hạn

M02/M03 vẫn PARTIAL. Bước tiếp theo là session probe GameServer có cipher derive, deadline/hủy và kiểm chứng greeting qua gateway với fixture mã hóa. Sau đó còn key provisioning/identity/ClientVerify và crypto packet trước login thật. MU server/DB không bật, source native/Data/config không thay đổi; ping overlay chưa có giá trị server thật.
