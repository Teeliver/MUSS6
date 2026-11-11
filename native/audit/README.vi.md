# Đối chiếu source EX603 — 2026-10-15

Đây là điểm bắt đầu kiểm kê lại, không phải chứng nhận đã đọc và chuyển đủ toàn bộ source.

## Phạm vi và bằng chứng

`source-manifest.json` ghi đường dẫn tương đối, dung lượng và SHA-256 của 2.418 file gốc (1.968 file code/header). Không tính cây đích Emulator/native, .git và .DS_Store. Có 285 file nguồn được nhắc theo đường dẫn tuyệt đối trong build.ninja hiện tại. Con số này bao gồm dependency/test và không đồng nghĩa tất cả đều thuộc executable vận hành hoặc đúng hành vi.

Tạo lại bằng `python3 native/tools/audit_source.py` từ Emulator. Mỗi file mặc định `not_individually_verified`: kiểm kê không được đánh dấu thành đã đọc, đã port hay đã test. Manifest không chứa mật khẩu hoặc nội dung runtime native.

## Bản đồ hiện tại

| Phần gốc | Cách dùng hiện tại | Việc còn phải xác minh |
|---|---|---|
| Emulator/GameServer | CMake lấy phần lớn .cpp gốc; thay entrypoint, socket, timer, DB, hiển thị bằng adapter | ABI, thời gian, đường lỗi và từng nhánh gameplay; compile thành công chưa chứng minh parity |
| Emulator/JoinServer, DataServer | Giữ logic C++ và thay hạ tầng Windows/SQL | Procedure, transaction, binary layout và lỗi DB |
| Emulator/ConnectServer | Dùng protocol/list/blacklist/IP/client logic gốc với transport native | Các đường timeout, reconnect, tải lớn |
| Main_EX603/Main | DLL mở rộng: protocol, custom config, hook địa chỉ main.exe | Không có implementation engine tương ứng các địa chỉ hook trong project này |
| GetMainInfo_EX603 | Nguồn schema cấu hình EX603 | Từng bảng custom phải có mapping rõ ràng khi dùng |
| Các Main phiên bản khác, MHP, tool và Util | Kiểm kê giữ nguyên; không tự trộn vào EX603 | Chưa review từng file; phân biệt không áp dụng với còn thiếu |
| Client native | SceneKit renderer mới, parser tài nguyên MU, protocol EX603 | Chưa phải engine gốc 100%; nhiều class, equip, combat, shop, skill, event còn thiếu |
| Tham chiếu engine trong /tmp/muonline-client-sources-master | Đối chiếu animation, rendering, UI và thao tác item | Là nguồn tham chiếu ngoài bộ người dùng, chưa chứng minh trùng phiên bản EX603 |

## Chức năng vừa kiểm chứng và giới hạn

Client 0.19 đã có click nhấc item, item đi theo con trỏ, click đặt; kiểm tra footprint trước gửi và cập nhật sau phản hồi server. Live test đã kiểm tra deposit/withdraw, từ chối ô bận và persistence qua đăng nhập lại; khôi phục vị trí fixture. Chưa triển khai đủ equip/trade/chaos/stack/swap/world drop hay túi mở rộng. Không suy rộng kết quả test kho sang toàn bộ giao dịch item.

Các tài liệu cũ trong native/client/SOURCE-AUDIT.vi.md và native/db/MIGRATION-STATUS.vi.md chứa mốc lịch sử; cần đối chiếu lại trước dùng như trạng thái hiện tại. Đặc biệt số procedure đã chuyển trong tài liệu cũ chưa được kiểm toán lại lần này.

## Ưu tiên kiểm toán tiếp

1. Đối chiếu movement, góc quay, walk/run, thời gian animation với hàm và dữ liệu gốc; ghi rõ phần nội suy mới của macOS.
2. Đo nhịp callback render và ACK trong cùng phiên chơi. Log render bổ sung p50/p95/p99, max, số khoảng >25ms và >250ms. Đây là khoảng callback, không phải GPU time; khoảng lớn chưa được quy cho server hay cảnh nặng khi chưa có ngữ cảnh foreground/resume.
3. Kiểm tra item model/orientation/effect, font, tooltip, layout và trạng thái input theo từng ca gốc.
4. Kiểm toán platform/DB adapter trước mở rộng gameplay. Mỗi phần cần nguồn hàm/file, sai khác, test và phần chưa biết.

Giữ mục tiêu 60 FPS nhưng không cam kết mọi frame 16,67ms khi chưa đo. Không đổi quy tắc gameplay chỉ để che khựng hình, không đánh dấu 100% dựa vào FPS trung bình hoặc build pass.
