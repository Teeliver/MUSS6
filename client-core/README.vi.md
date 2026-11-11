# Portable client core — W0008

Hiện chỉ có `mu/frame_reader.hpp`: framing C1/C2/C3/C4 theo `Socket::try_frame` trong native/client/client.cpp. Không có socket, Cocoa, timer, crypto, UUID, DOM hoặc state gameplay.

`feed(bytes)` trả về các frame hoàn chỉnh theo thứ tự và giữ phần thiếu. Byte đầu vào phải ở tầng **sau transport decoding**; core không tự giải mã C3/C4 hoặc transport cipher. Frame trả về sở hữu bản sao byte riêng. Length C2/C4 big-endian, giới hạn frame 16.384 byte, buffer/input mỗi feed tối đa 262.144 byte, như giới hạn native hiện tại. Một chunk vượt cap bị từ chối kể cả chứa nhiều frame hợp lệ; transport phải chia chunk phù hợp.

Lỗi type/length/cap/EOF làm phiên reader thất bại, xóa buffer; cần reader mới khi mở phiên mới. Không dò tìm header để bỏ qua byte lỗi. Một feed có frame lỗi phía sau sẽ throw và không trả các frame trước trong cùng feed: caller phải đóng phiên, không retry chunk đó. `finish()` phát hiện EOF giữa frame. Không có timeout; adapter mạng phải thực thi deadline packet dang dở theo clock riêng. Chưa nối reader này vào native production hoặc web runtime.

Build/test độc lập từ Emulator:

```sh
cmake -S client-core -B web/.local-baselines/2026-10-23-m02-008/core-build -DCMAKE_BUILD_TYPE=Debug
cmake --build web/.local-baselines/2026-10-23-m02-008/core-build -j 2
ctest --test-dir web/.local-baselines/2026-10-23-m02-008/core-build --output-on-failure
npm --prefix web test
```

Test dùng fixture tổng hợp; chỉ handshake C1 04 00 01 lấy từ quy tắc handshake native. Fixture C3/C4 chỉ kiểm framing, không phải ciphertext đã xác thực. Chưa có capture đăng nhập đã loại credential hoặc đối chiếu output WASM/native bằng cùng fixture.

## Cập nhật W0018

Core hiện có thêm codec ConnectServer, profile/greeting, identity/XOR và SimpleModulus/key-file/serial. Mô tả W0008 phía trên giữ làm lịch sử framing; không còn đại diện toàn bộ core. Phạm vi mới và thứ tự layer: [SIMPLE-MODULUS](../web/docs/SIMPLE-MODULUS.vi.md). Chưa tích hợp native production hoặc login web thật.
