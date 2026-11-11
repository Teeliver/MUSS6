# W0018 — SimpleModulus và packet serial

## Nguồn đối chiếu

- `Main_EX603/Main/PacketManager.cpp` (bên cạnh thư mục Emulator): EncryptBlock, DecryptBlock, AddBits, GetByteOfBit, Shift và layout key-file trong header cùng module.
- `native/client/client.cpp`: thứ tự packet XOR → serial/SimpleModulus → transport khi gửi; transport → framing → SimpleModulus/serial khi nhận.
- Fixture trích thân hàm Main vào namespace riêng, dùng BYTE/WORD/DWORD có độ rộng 8/16/32 bit. Không chạy Main hoặc đọc config/identity thật.

## Đã triển khai trong phạm vi core

`client-core/include/mu/simple_modulus.hpp` chứa parser key-file EX603 54 byte, phép toán DWORD 32 bit, block 8 → 11 byte, checksum và độ dài block. Core zero-pad block cuối để tránh đọc vượt bộ nhớ, theo native Mac hiện có. Không giữ lỗi kiểm tra kết quả block trong vòng lặp Decrypt của Main: checksum/length sai bị từ chối.

PacketCipher giữ serial gửi/nhận độc lập, vòng 0–255. Outbound C1/C2 dùng packet XOR; C3/C4 dùng XOR rồi chèn serial và mã hóa. Inbound C3/C4 kiểm serial rồi chuẩn hóa thành C1/C2; không tự gỡ packet XOR inbound vì server không gửi theo chiều XOR client. Plain inbound không tăng serial. Lỗi protocol khiến instance thất bại; phiên mới cần instance mới.

Adapter `web/client/src/platform/wasm/create-packet-cipher.js` bắt buộc hai key-file byte array được cấp rõ ràng, không fetch file hoặc dùng key mặc định. Bản sao JS tạm được xóa sau khởi tạo; không tuyên bố đã xóa mọi bản sao trong WASM heap. Bridge trả lỗi có kiểm tra thay vì để exception C++ thoát sang JS.

## Kiểm chứng ngày 23/10/2026

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Cả ba lệnh qua: 21/21 Node/native test, không skip; contract và bridge WASM qua. Contract so byte mã hóa và giải mã với thân hàm Main, gồm block cuối, DWORD overflow, key-file thiếu/thừa/sai header, checksum, block cuối, serial wrap/replay và biên kích thước C3. Test build kiểm allowlist Debug/Release và cú pháp các module JS xuất ra.

Key trong test là tổng hợp có cặp nghịch đảo, không phải Enc1/Dec2 thật. Loopback client-packet trong test có bước reverse XOR riêng để so fixture; không đổi quy tắc inbound server ở runtime.

## Chưa hoàn thiện

- Chưa provisioning/kiểm chứng Enc1/Dec2 thật và profile runtime.
- Chưa ghép PacketCipher vào phiên GameServer duy trì kết nối; chưa login, account hoặc gameplay.
- Identity provider browser vẫn thiếu theo W0017; không bypass ClientVerify/HWID.
- Chưa browser/server MU interoperability, packet capture hợp lệ hoặc benchmark gameplay 60 FPS. Test numeric không chứng minh toàn bộ game giống source.

Không bật MU server/DB, không đổi native production, Data hoặc config MU. M02/M03 tiếp tục PARTIAL. Bước sau: ghép pipeline theo đúng thứ tự layer bằng fixture session trước khi mở luồng đăng nhập thật.
