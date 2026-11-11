# W0085 — GUI Move và MuOptions test level300

2026-10-28. Theo yêu cầu sau cùng, bỏ phím S mở Skills nếu không có trong source.

## Đối chiếu và phạm vi

- Source `NewUIMainFrameWindow.cpp:CNewUISkillList` có GUI icon và thao tác click ô skill để mở danh sách, Ctrl+số để gán, phím số để dùng hotkey. Chưa tìm thấy đường S mở Skills trong bộ source đã đọc. `NewUIMyInventory.cpp` dùng S ở luồng cửa hàng cá nhân. Đã gỡ phím S web vừa thử sau khi người dùng yêu cầu bỏ qua chức năng không có source. Không thêm một panel Skills tự thiết kế.
- `NewUIMoveCommandWindow.cpp` có Move GUI: nền đen alpha0.8, tiêu đề, cột map/level/Zen, dòng đủ điều kiện màu trắng, dòng không đủ điều kiện màu tối, highlight vàng, scroll. Web thay bảng debug lớn bằng panel Move gọn; M/Esc đóng/mở, click gửi move index cũ và chờ ACK. Giữ server làm thẩm quyền điều kiện map/Gens/trang bị/reset/account.
- Offline links nằm trong details chỉ ở chế độ chưa vào nhân vật; không trình bày gate/resource audit trong flow Move online.
- Đây là port bố cục qua HTML/CSS. Scrollbar dùng browser với style gần resource; chưa port5texture scrollbar native nên không tuyên bố pixel parity100%. Level/Zen hiển thị từ catalog Move.txt; giảm cấp yêu cầu cho MG/DL/RF chưa port vào UI, không đánh dấu đủ mọi class.

## MuOptions

Người dùng xác nhận đã thoát. Kiểm tra MEMB_STAT.ConnectStat=0; cLevel7/class0, ô cánh7 trống. Backup riêng0600: `native/runtime/client/MuOptions-before-W0085-20261028-090044.json` (chỉ level, EXP, inventory; không credential).

- Nâng cLevel300, Experience377963080 là gLevelExperience[299], tính theo `User.cpp` với ConstA10/ConstB1000 trong config. Giữ nguyên stats và LevelUpPoint.
- Cánh cấp1 DW là Item(12,1), Wings of Heaven +0, durability100. Lấy serial qua procedure WZ_GetItemSerial; mã hóa16byte theo DBItemByteConvert; trang bị trực tiếp vào ô7 đang trống. Không đặt vào ô chứa item khác.
- Cập nhật transaction với kiểm tra offline và so sánh toàn bộ inventory trước sửa, từ chối nếu có concurrent change. Xác nhận level/EXP/16byte ô cánh đúng, các byte inventory khác giữ nguyên. Không sửa class/tiền/item khác.
- Phiên thật sau vào lại hiển thị Lv300, HP389/389, MP1442/1442, SD7652/7652, cánh trắng đã gắn trên lưng. Chưa xác minh mọi motion bay hoặc hiệu ứng cánh qua mọi map.

## Kiểm tra

233 test qua trước lần gỡ S (chỉ xóa nhánh phím S, không đổi protocol). Build debug116/release32. Browser fixture kiểm tra GUI Move mở bằng M, gồm Map/Level/Zen và scroll, không console warn/error. Không chuyển map thật vì có tốn Zen/đổi vị trí; server travel vẫn dùng flow đã có test request/ACK.

Ảnh GUI Move `/tmp/mu-w85-move.png`; ảnh nhân vật `/tmp/mu-w85-level300-wing.png`. Phím S đã thử trên fixture nhưng sau đó gỡ theo yêu cầu, không tính là chức năng hoàn thành.
