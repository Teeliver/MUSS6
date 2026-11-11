# ClientVerify/HWID và packet XOR — W0017, 2026-10-23

## Luồng source đã đối chiếu

- Main_EX603/Main/Protocol.cpp GCConnectClientRecv gửi F3:09 sau greeting, HardwareId lấy từ GetHardwareId.
- Main_EX603/Main/Util.cpp GetHardwareId dùng Win32 volume/system information, tạo5 nhóm DWORD dạng uppercase `%08X`, nối4 dấu `-`:44 ký tự + NUL.
- native/client/client.cpp dùng SHA-256 của host UUID macOS và định dạng5 nhóm8 hex tương ứng. Đó là adapter native đã có, không phải Windows ID hoặc browser ID.
- GameServer/HidManager.h: PSBMSG_HEAD4 byte + HardwareId[45] =49 byte C1:F3:09.
- GameServer/Protocol.cpp dispatch F3:09 tới CGHardwareIdRecv.
- GameServer/HidManager.cpp kiểm tra format, blacklist, số kết nối theo HWID rồi đặt ClientVerify=1, lưu HWID và tăng count.
- GameServer/User.cpp đóng client quá MaxTimeConnectionVerify nếu ClientVerify còn0. Source không gửi ACK thành công riêng cho F3:09.

Giữ nguyên blacklist, MaxConnectionPerHID, verify timeout và code server. Theo implementation hiện có, MaxConnectionPerHID=0 **không phải unlimited**: CheckHardwareId trả từ chối cho ID chưa có. Không tự đổi ý nghĩa cấu hình này.

## Đã triển khai

`client-core/include/mu/client_identity.hpp`:

- hardware_identity_packet: C1 31 F3 09 +44 byte identity +NUL cuối, đúng49 byte.
- Validate đúng5 nhóm uppercase hex8 ký tự của producer Main/native. Đây là kiểm tra chặt theo format producer, không mô phỏng lỗi điều kiện && ở kiểm tra dấu '-' của server.
- filter_plain_packet: lớp XOR chain client→server, encode tiến từ sau opcode, decode ngược. Chỉ C1/C2 đã có length/opcode hợp lệ và size<2048; từ chối C3/C4 vì chưa có SimpleModulus/serial, không gửi chúng như packet plain.

WASM có checked-result identityPacketResult/packetFilterResult; `client/src/protocol/client-identity.js` cung cấp codec và prepareIdentityPacket nhận provider explicit. Không có random ID, localStorage identity, đọc HWID máy, persistence, tự gửi network hoặc fallback khi provider thiếu. Chưa cài provider thật.

Pipeline fixture F3:09: identity packet → packet XOR → transport encode; đảo transport → XOR ngược khôi phục đủ49 byte. Không áp XOR client-outbound này lên greeting server-inbound.

## Kiểm thử bằng source và dữ liệu giả

Reference fixture lấy nguyên thân XorData từ Main_EX603/Main/PacketManager.cpp và GameServer/PacketManager.cpp, xác nhận table EX60332 byte trùng nhau trước compile. Kiểm output core với hàm Main gốc và reverse với hàm server gốc; không chỉ roundtrip cùng implementation. Reference chỉ chạy trong test build/native/WASM, không link startup hoặc config server.

- Packet49 byte, opcode/subcode/NUL và nội dung field đúng.
- Sửa từng vị trí trong44 ký tự thành ký tự sai đều bị từ chối.
- XOR C2 ở các kích thước4/255/256/2047, packet sai length và C3 chưa hỗ trợ.
- Chuỗi packet-XOR/transport với MHP tắt/bật khôi phục bytes.
- JS-WASM từ chối provider thiếu, identity không đúng format và C3/plain filter.

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Kết quả thực: build WASM qua; npm test **20/20 qua**, không skip; test:wasm qua identity/XOR/source-reference và các contract trước. Fixture identity là chuỗi giả ghi trong test, chưa bao giờ gửi tới server MU. Không lấy hoặc ghi host UUID/HWID thật.

## Ranh giới browser còn mở

Browser hiện không có adapter gọi GetVolumeInformation/GetSystemInfo Windows hoặc gethostuuid macOS của source. Không tạo ID theo lượt mở tab hoặc storage để thay HWID vì sẽ đổi blacklist/count theo thiết bị. Identity provider thật **chưa triển khai** và luồng account chưa được mở.

Hướng giữ kiểm tra server gần source nhất là provider native phù hợp OS (ví dụ helper của bản Mac sử dụng cùng derivation đã có), được cấp quyền và ràng buộc với client/browser origin. Chưa có helper, trust/provisioning hoặc browser integration; đây là hướng thiết kế, chưa phải tính năng an toàn đã được xác minh. Phương án browser-only installation ID sẽ thay semantics physical-device và cần quyết định riêng, không tự áp dụng.

Trạng thái gửi identity sau này chỉ được gọi identity-sent, không gọi verified do send thành công. Server mới quyết định ClientVerify; source không có ACK riêng. Greeting probe W0016 vẫn không gửi F3:09 hoặc login.

M02/M03 vẫn PARTIAL. MU server/DB, Data/config và native production không thay đổi. Có thể tiếp tục tách SimpleModulus/packet serial bằng fixture trước khi phụ thuộc provider; không bypass identity để thử login.
