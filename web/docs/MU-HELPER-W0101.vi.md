# W0101 — Hai nút MU Helper trên thanh tọa độ

Ngày 30/10/2026. Tiếp theo W0100; người dùng đã yêu cầu triển khai cả cấu hình và bật/dừng.

## Đối chiếu

- Main5.2/source/NewUIHeroPositionInfo.cpp: hai nút Setup và Start/Stop.
- NewUIMacroMain.cpp: frame 190×429, Hunting / Obtaining / Other, Save/Initialization, lớp cấu hình theo class.
- CAIController.cpp: Pack/Unpack 256 byte, chu kỳ WorkAI 150 ms, potion 500 ms, RateItem, COMGEM::Check_Jewel và matchesPattern.
- wsclientinline.h, WSclient.cpp: C2 AE lưu/nhận cấu hình; C1 BF:51 bật/dừng và kết quả.
- GameServer/Helper.h/.cpp: ABI thực tế, điều kiện map/safe zone/level, phí và lưu DataServer.

## Đã nối

- Setup mở panel cấu hình. Hunting chọn skill chính, hai skill theo delay, range, bình máu theo ngưỡng, quay lại vị trí xuất phát. Obtaining chọn nhặt tất cả hoặc Jewel/Ancient/Excellent/Zen/tên bổ sung, range và sửa đồ độ bền thấp.
- Lưu đúng 256 byte vào C2 AE tổng 261 byte. Giữ các bit điều kiện và vùng dự trữ chưa triển khai. Không dùng localStorage để giả lập lưu server.
- Start/Stop gửi C1 BF:51 tổng 5 byte. Chỉ chạy sau result 0 từ server; result 1/2 dừng. Đọc DWORD time/money tại offset 4/8, result tại 12 trong struct native 16 byte. Test ABI biên dịch từ Helper.h thực tế.
- Timeout không đóng phiên và không tự giả lập ACK. Fee/Zen do server quyết định. Bản cấu hình server hiện tại yêu cầu level 80, phí ban đầu level×20 (MuOptions level 300: 6.000 Zen), server tiếp tục thu theo stage/delay.
- Tự đánh đi qua world-controls, đường đi ACK, animation/skill effects và world-session.cast sẵn có, gồm packet đa mục tiêu Evil Spirit. Chỉ chọn quái đã biết có thể đánh, không chọn player/NPC/quái chết. Không tự tạo damage.
- Dùng bình máu theo thứ tự source: 14/0,1,2,3,38,39,40,70,94, tìm ngược trong túi. Không nhân bản/di chuyển bình.
- Nhặt dùng luồng pickup thật, có kiểm tra ô túi đủ chỗ; quyền sở hữu/thời gian bảo vệ vẫn do server. Jewel gồm Gemstone, lower/higher refining stone và bundle theo CComGem.cpp. Extra-name matching phân biệt hoa thường, token quanh dấu + phải xuất hiện theo thứ tự.
- Nút Start đổi resource Stop khi đã nhận ACK chạy. Safe zone/Chaos Castle, mở cấu hình/túi hoặc đang nhấc đồ không cho khởi động từ GUI. Panel mở chặn world input; stop/close dọn intent/timer.
- Resource gốc: MacroUI_Start/Stop/Setup; frame Inventory dùng sẵn; newui_option_check và InGameShop/Ingame_Bt03. Atlas mới được hash và allowlist; chỉ restart Identity asset service, không restart GameServer.

## Chưa đạt parity toàn bộ Helper

- Party heal/buff, self-buff, combo, lệnh Dark Raven, auto friend/guild, elite mana potion và offline train chưa triển khai. GUI khóa và giải thích; các byte/bit cấu hình cũ vẫn giữ.
- Điều kiện phụ của activation skill chưa thực thi; runtime không dùng auxiliary skill có cờ condition đang bật. Hiện chỉ nhánh delay.
- Bộ chọn skill là select HTML của web, chưa phải popup icon native 32×38 và chưa nhập đủ SkillUseType/TypeSkill/IsCanBCSkill. Không công bố giao diện Helper đúng 100%.
- Repair hiện chọn từng trang bị có durability 1–4, chưa port đầy đủ CheckRepairItem/MaxDurability của CAIController. Long-distance counterattack, ưu tiên kẻ đang đánh mình, combo và mọi trường hợp target không được chứng nhận.
- Nhặt thất bại có cooldown 5 giây trên từng key; source native dùng prev_item. Đây là khác biệt đã biết.
- GameServer không có ACK riêng cho Save Setting; thông báo chỉ nói đã gửi. Chỉ lần đăng nhập sau nhận C2 AE mới xác nhận lưu bền vững. Chưa có bài kiểm tra tự động relog + DB cho Helper.
- Source client gán Ancient bit 4 / Excellent bit 5; HelperInfoData phía server đặt tên hai bit ngược. Giữ Pack của client gốc cho điều khiển online, chưa bật offline train.

## Kiểm tra

- Toàn bộ suite client 280/280 qua trong thư mục build cách ly. Không chạy scaffold test trên dist đang phục vụ. Debug 136 file/release 38 file build thành công.
- Offline UI: mở Setup, chọn Fire Ball, lưu/đóng, mở các tab. Không mở socket hoặc giả lập server cho nút Start.
- Phiên MuOptions thật: quan sát log UI có `MU Helper: 6,000 Zen.`, `Scale Pants Obtained`, `45 Zen Obtained`, `MU Helper stopped.`; safe-zone guard hoạt động. Đây là bằng chứng tích hợp cơ bản, chưa chứng nhận tất cả skill/map/thao tác.
- Tab thật sau đó hiển thị cấu hình range 8, Basic Skill Evil Spirit. Người dùng đang thao tác nên không tự đăng xuất/ghi đè cấu hình của họ để test lại.
- FPS tại thời điểm quan sát khoảng 60, frame p95 ~17 ms; không suy ra bảo đảm hiệu năng trên mọi map/thiết bị.
