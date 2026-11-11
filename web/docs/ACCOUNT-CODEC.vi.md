# W0021 — Codec đăng nhập EX603

## Nguồn và ABI

- GameServer/Protocol.h: PMSG_CONNECT_ACCOUNT_RECV đóng gói 1 byte; với GAMESERVER_UPDATE=603 có account[10], password[20], DWORD TickCount, version[5], serial[16], không LanguageCode. Profile 603 được khai báo ở native/game/CMakeLists.txt và cấu hình EX603 trong GameServer.vcxproj; không dùng default 803 ở stdafx.h.
- Main_EX603/Main/Protocol.h: struct PMSG_CONNECT_ACCOUNT_SEND cùng layout 59 byte.
- GameServer/Util.cpp: PacketArgumentDecrypt dùng chu kỳ FC CF AB.
- GameServer/Protocol.cpp CGConnectAccountRecv: kiểm version/serial, giải mã 10 byte account và 10 byte password dù trường password trên wire dài 20 byte; mismatch profile trả mã 6.
- GameServer/JSProtocol.cpp: result != 1 là không thành công; có thể thay bằng 8 theo số lần thử. GCConnectAccountSend trả C1 F1:01 + result. Không tự gán diễn giải ngôn ngữ cho các mã chưa đối chiếu.
- native/client/client.cpp login_packet: giới hạn ASCII, XOR 10 byte đầu mỗi field, 10 byte đuôi password zero; tick little-endian, version/serial raw. Đây là chính sách adapter native hiện có, không khẳng định mọi input khác đều bị server gốc cấm.

## Đã triển khai

Core `client-core/include/mu/account_protocol.hpp` tạo packet C3 3B F1 01 dài 59 byte trước packet-XOR/SimpleModulus/transport. Offset: account=4, password=14, tick=34, version=38, serial=43. Hai field 10 byte đầu có padding NUL trước credential-XOR; password đuôi 24..33 bằng zero theo native. Đây là credential-XOR riêng, không thay thế packet-XOR.

Account 1–10 ASCII chữ/số; password 1–10 printable ASCII không space theo adapter native, không cắt input âm thầm hoặc nhận Unicode tùy ý. Version yêu cầu Uint8Array đúng 5 byte đã được cấp từ profile, không tự suy ra từ chuỗi phiên bản hiển thị. Serial đúng 16 printable ASCII. Tick nhận uint32; JS kiểm trước Embind coercion để không wrap số âm/quá giới hạn/số lẻ.

Response parser chỉ nhận C1 05 F1 01 result sau khi channel đã normalize crypto; giữ code 0–255, accepted chỉ khi code=1. Không biến mã lạ thành success và không tạo text dịch chưa lấy từ Data/Language.

JS adapter nằm tại `web/client/src/protocol/account-codec.js`, checked-result bridge bắt lỗi C++. Không tự đọc credential, fetch config, gửi network hoặc giữ account/password trong state adapter. Không cam kết xóa string JS/WASM credential khỏi bộ nhớ; caller sau này phải giới hạn lifetime và không log packet.

## Bằng chứng kiểm thử 23/10/2026

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

WASM build qua; 32/32 test qua, không skip; WASM contract/bridge qua. Fixture trích struct nguyên từ Protocol.h với define 603 để static_assert size/offset và trích thân PacketArgumentDecrypt của server, không chỉ kiểm roundtrip cùng codec. Native và WASM đều chạy contract này. Input tổng hợp độ dài 1..10, padding/endian/profile offsets, input sai và toàn bộ 256 mã response được kiểm tra.

JS-WASM test kiểm validation trước numeric coercion và packet đi qua SimpleModulus thành C3 dài 90 byte rồi khôi phục payload fixture đúng. Roundtrip crypto dùng key tổng hợp; không phải server đăng nhập thật. Build Debug/Release allowlist và cú pháp module qua, không đổi layout UI.

## Chưa hoàn thiện

- Chưa state machine gửi login/chờ response/retry policy, chưa nối vào UI hoặc danh sách nhân vật.
- Chưa account thật, browser/MU interoperability, key/profile provisioning hoặc identity provider thật.
- Chưa giải mã version hiển thị sang byte profile hoặc lấy text thông báo từ Data/Language; không hardcode text UI thay source.
- Password trên 10 ký tự và encoding ngoài ASCII chưa hỗ trợ; không thay server để nhận chúng.
- Không bật server/DB, không sửa native production/Data/config MU. M02/M03 vẫn PARTIAL.

Bước sau: state machine đăng nhập trên channel sau identity-sent, với timeout/cancel và chặn gửi trùng; dùng fixture trước khi tích hợp UI.
