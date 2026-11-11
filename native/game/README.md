# GameServer C++ EX603ENG trên macOS

Mở `native/Start-GameServer.command` hoặc chạy executable trực tiếp. Runtime mặc định được cấu hình tại `native/config/game/runtime.ini`, hiện trỏ `/Users/angelale/Downloads/MuServerEX603/GameServer`. `--config DIR` vẫn ưu tiên. Kiểm tra INI bằng `native/Start-GameServer.command --check-config`; preflight không xác nhận Data/schema hoặc gameplay.

`mu_game_logic` biên dịch module gameplay C++ gốc và được link vào `GameServer-native`. Entry point console: `main.cpp`; POSIX transport: `socket_manager.cpp`, `socket_manager_udp.cpp`, `connection.cpp`; timer: `queue_timer.cpp`. Lua 5.2.4 link từ source trong native/vendor, QueryManager dùng libmysqlclient.

Build, cấu hình VPS và giới hạn được mô tả trong [hướng dẫn native](../README.md). Chưa có xác nhận chạy client thật. Bộ Data hiện có cần đối chiếu/mapping: `runtime-data-audit.json`; đường dẫn literal source EX603: `asset_paths.hpp`.

Các sửa gồm pointer đầy đủ trong RandomManager, atomic LONG32, packed records, endian/unaligned cipher và token callback thay pointer DWORD. Giữ kích thước packet; chi tiết callback ở [CALLBACKS.md](CALLBACKS.md).

Các lớp Windows được giữ riêng bằng MU_NATIVE_MACOS. Native thay hoặc loại WinMain/GDI, IOCP, ODBC, DbgHelp; không dùng implementation giả báo thành công. Phương thức thread Win32 cũ trên native adapter trả lỗi unsupported nếu bị gọi.

Các adapter POSIX kiểm tra frame hoàn chỉnh, loại C1/C2 hoặc C1–C4 phù hợp, kích thước header, giới hạn buffer và checksum từng block EX603 trước khi chuẩn hóa packet. Callback Join/Data nhận backing buffer 524288 byte với phần đuôi zero; queue client giữ backing 8192 byte. Đây là validation framing và cipher: chưa kiểm tra đầy đủ độ dài từng struct hoặc số record do field `count` điều khiển trong handler gốc. Buffer có phần đuôi zero tránh lỗi đọc quá allocation do việc đổi buffer source sang vector vừa bằng packet; không chứng minh packet thiếu body hoặc count giả sẽ bị từ chối theo schema.

`packet_crypto_test.cpp` dùng implementation `PacketManager.cpp`, `HackCheck.cpp` và `ServerInfo.cpp` thật. Hai argument bắt buộc là thư mục chứa `MuServer/Data/Hack/{Enc2,Dec1}.dat` và `MuClientEX603/Data/{Enc1,Dec2}.dat`. Test kiểm tra cả hai chiều Enc1→Dec1 và Enc2→Dec2 với payload 1–513 byte, header key 6 byte/file 54 byte, file thiếu/truncated/corrupt, modulus/key zero, checksum sai ở block cuối, unaligned decode ở offset1, XOR inbound, serial/C1–C4 và transform outer/MHP theo từng chunk. Run trực tiếp và run với AddressSanitizer + UndefinedBehaviorSanitizer đã pass trên ARM64, không mở socket hay database. Khi tạo target test riêng, link `mu_platform` và các source trên; dùng `-ffunction-sections -fdata-sections` cùng `-Wl,-dead_strip` trên macOS để loại các method `ServerInfo` không được test gọi.
