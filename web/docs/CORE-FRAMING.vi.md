# Core framing — W0008, 2026-10-23

Bắt đầu M02 bằng thành phần nhỏ độc lập: `client-core/include/mu/frame_reader.hpp`. Quy tắc type, header và length tham chiếu trực tiếp `Socket::try_frame` trong native/client/client.cpp. Đây là implementation portable mới của framing, chưa tách toàn bộ Socket/codec/state native.

Đã kiểm tra bằng fixture tổng hợp:

- Mọi vị trí cắt của stream có đủ C1/C2/C3/C4, feed từng byte và nhiều frame dính nhau.
- Frame cực đại 16.384 byte, byte length cao trước của C2/C4.
- Type/length sai, vượt frame/buffer cap, EOF ở header/body dở.
- Byte đầu vào sửa sau feed không thay frame đã trả; phiên lỗi không tiếp tục nhận data.
- Frame lỗi cuối chunk làm cả feed thất bại; không trả kết quả từng phần.

Core không có timer hoặc I/O. Transport phải áp timeout/backpressure và cấp byte đã decode transport đúng tầng. Chưa có crypto, session/serial, state, login hoặc gateway; không dùng framing pass để gọi protocol đã hoạt động.

Kết quả: `npm --prefix web test` **3/3 qua** (bao gồm compile C++17 với Wall/Wextra/Werror/pedantic); standalone CMake build và CTest **1/1 qua**. Các check C++ dùng hàm check có throw, không bị tắt bởi NDEBUG.

Không tìm thấy `emcc` trong PATH tại máy. Chưa build WASM, chưa có native/WASM differential test hoặc browser runtime. Không tự thêm toolchain hoặc dependency mạng trong đợt này. Native production, Data, config và server/DB không thay đổi; build web vẫn chỉ là diagnostics.

Bước tiếp theo: chuẩn bị toolchain WASM và bridge feed/output, chạy cùng fixture trên native và WASM; kiểm tra browser thật trước khi đóng yêu cầu portable tương đương. Các mục M00/M01 chưa xác minh vẫn mở.
