# W0084 — Lưu hotkey nhân vật và nhấc liên kết bình

2026-10-28. Đã deploy debug8080, build116debug/32release. 233 test Node qua.

## Source và giao thức

Đọc `Main5.2/source/ZzzOpenData.cpp:SaveOptions`, `WSclient.cpp:ReceiveOption`, `wsclientinline.h:SendRequestHotKey`, `NewUIMainFrameWindow.cpp` phần CNewUIItemHotKey/skill list; server `Protocol.h:PMSG_OPTION_DATA_RECV/SEND`, `Protocol.cpp:CGOptionDataRecv`, `DSProtocol.cpp:GDOptionDataSaveSend`, `DataServer/DataServerProtocol.cpp`.

Packet C1:F3:30 dài34 byte, packed. SkillKey20byte: 10skill ID big-endian, FFFF là chưa gán. Giữ nguyên GameOption/ChatWindow; Q/W/E/R là offset từ ITEM_POTION và FF là trống. QWERLevel là DWORD little-endian, giải mức Q ở byte cao theo ReceiveOption. Lưu theo nhân vật qua GameServer→DataServer→OptionData. Không dùng localStorage hoặc cache trình duyệt.

Web nhận F3:30 vào game state, khôi phục HUD; gửi lại khi gán/bỏ/chuyển liên kết hoặc Ctrl+digit. Không gửi trước khi đã nhận options server, tránh ghi đè cấu hình khác. Server không có ACK riêng cho save; trạng thái phản ánh request đã gửi, việc tồn tại sau đăng nhập dựa vào F3:30 tải lại. Không hứa đã lưu nếu transport lỗi.

## Thao tác và hiển thị

- Nhấc bình trong túi rồi click ô HUD để gán. Click bình đang ở HUD để nhấc liên kết: click ô HUD khác để chuyển/đổi chỗ; click ngoài HUD (kể cả ô túi) để bỏ liên kết; Esc giữ nguyên. Q/W/E/R mới là thao tác dùng bình. Bỏ liên kết không gửi drop/move inventory, không tiêu hao bình.
- Native source đã đọc dùng Ctrl+Q/W/E/R trên vật phẩm trong inventory để đăng ký và lưu loại/level. Chưa tìm thấy native đường xử lý click-nhấc trực tiếp từ HUD; thao tác nhấc liên kết ở web là theo yêu cầu người dùng, không tuyên bố đã chứng minh giống hoàn toàn mọi client gốc.
- Số skill 1–5 chuyển từ chữ canvas nhỏ/vàng sang cùng lớp chữ trắng đậm `.mu-hotkey-count` như số lượng bình. Số SD canh mép phải trước ô skill thay vì căn giữa kéo dài dưới icon.

## Bằng chứng

- 233 test qua: codec roundtrip với byte golden/endian, giữ nguyên game/chat options, binding trống, tái tạo game state từ F3:30 và tách nhân vật. Các test trước gồm request ABI native/opcode ENG.
- Browser fixture: gán Q có200 bình; click Q→E chuyển liên kết, Q trống/E200; nhấc E rồi click ô túi19 bỏ liên kết, hình Large Healing Potion vẫn ở túi. Ctrl+1/2 hiển thị Fire Ball/Energy Ball, số1–5 là DOM text dùng cùng lớp chữ với potion. Không console error/warn.
- Người dùng trả lời “ok đã test” sau refresh. Quan sát phiên MuOptions có skill1 Evil Spirit/2 Energy Ball/3 Fire Ball; Q43 và W46. Truy vấn chỉ đọc `OptionData WHERE Name=MuOptions` thấy SkillKey tương ứng9/17/4 và QKey3; không sửa DB trực tiếp. Đây xác nhận save thật; chưa tự thực hiện một vòng logout/login độc lập dưới điều khiển agent.
- Ảnh `/tmp/mu-w84-hotkeys.png`; phiên vẫn kết nối, FPS khoảng57–60.

Trong deploy hết dung lượng ở bước copy staging; bản đang chạy không bị đổi. Dọn partial build và5backup W0082 cũ do agent tạo, giữ các bản gần nhất; deploy lại thành công, còn khoảng1.8GiB. Data/resource gốc không đổi.
