# W0023 — Danh sách nhân vật và policy EX603

## Nguồn đối chiếu

GameServer/Protocol.h: PMSG_CHARACTER_LIST_SEND, PMSG_CHARACTER_LIST, PMSG_CHARACTER_CREATION_ENABLE_SEND; định nghĩa UPDATE=603/EXTRA=1 theo native/game/CMakeLists.txt. GameServer/Protocol.cpp CGCharacterListRecv chỉ xử lý khi OBJECT_LOGGED. GameServer/DSProtocol.cpp DGCharacterListRecv tạo list và gửi policy flag=0 ngay sau list. Native/client/client.cpp Session::characters gửi C1 04 F3 00, đợi F3:00 rồi DE:00; parser native làm nguồn kiểm input bổ sung.

## Đã triển khai

Core `client-core/include/mu/character_protocol.hpp` và adapter `web/client/src/protocol/character-codec.js`:

- Request C1 04 F3 00.
- List sau chuẩn hóa crypto: C1 F3:00, header 8 byte, ClassCode/MoveCnt/count/ExtWarehouse raw; tối đa 5 dòng, mỗi dòng đúng 34 byte.
- Dòng gồm slot offset0, Name[10] offset1, byte đệm11, level WORD LE offset12, control14, CharSet[18] offset15, guildStatus33. Không xóa/diễn giải padding, không cho byte đệm làm lệch level/trang bị.
- Giữ charset raw 18 byte với bản sao sở hữu riêng; chưa giải mã class, item, wings hoặc effect từ charset. Không suy ra class chỉ từ vị trí slot.
- Kiểm length/count, slot0–4 và trùng slot/tên. Tên 1–10 ASCII chữ/số theo giới hạn adapter native hiện tại; tên đủ10 byte không cần NUL. Byte sau NUL không dùng làm tên. Encoding ngoài ASCII chưa hỗ trợ, không âm thầm thay tên.
- Policy C1 DE flag0, EXTRA1 ABI 14 byte: result/classCode offset4, byte đệm5, Experience A/B offset6/8, maxLevel10, deleteMaxLevel12. Giữ giá trị server raw; classCode trong policy không tự ghi đè list trước khi caller ghép cả hai gói.

Source tạo list bằng setE (C3); channel giải mã thành C1 trước parser. Parser không nhận ciphertext C3 trực tiếp. Các giới hạn là bảo vệ parser, không sửa cấu trúc Data/server.

## Kiểm thử 23/10/2026

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
node --test web/tests/character_codec.test.mjs
```

Build WASM, 38/38 test và test:wasm qua, không skip; targeted JS test qua sau bổ sung ca malformed. Contract trích ba struct từ Protocol.h vào fixture với define603/EXTRA1, dùng static_assert size/offset, so dữ liệu fixture với struct server native/WASM. Fixture có 0–5 nhân vật, level endian, charset/padding khác0, mọi prefix bị cắt và duplicate slot/name. JS-WASM kiểm tên đủ10byte, charset copy ownership, policy, header/count/slot/name/trailing bytes sai. Không có packet live MU hoặc dữ liệu tài khoản thật.

Debug/Release allowlist và cú pháp module mới qua. UI không thay đổi, chưa hiển thị scene nhân vật web.

## TODO / chưa hoàn thiện

- Chưa ghép request/list/policy vào lobby session, chưa timeout/hủy/chặn request trùng cho roster.
- Chưa decode CharSet sang model/trang bị/hiệu ứng hoặc UI chọn/tạo/xóa nhân vật.
- Chưa MU/browser/gateway interoperability, provider/key provisioning thật; M02/M03 vẫn PARTIAL.
- Chưa Language/font UI hoặc benchmark gameplay; không coi parser qua là màn chọn nhân vật đã giống source.
- Không bật MU server/DB, không đổi Data/config hoặc native production.

Bước tiếp theo: lobby request chờ đủ list và policy theo thứ tự source, có deadline/cancel và xử lý packet ghép cùng chunk, trước khi dựng UI.
