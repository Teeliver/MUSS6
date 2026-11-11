# W0092 — Cửa tự mở theo source

2026-10-29. Đối chiếu Main5.2/source/ZzzObject.cpp:4586–4632, nhánh WD_2DEVIAS. Type86 trượt theo hướng0/90/180/270; type20/65/88 xoay theo góc gốc. Khoảng cách kích hoạt nhỏ hơn200 đơn vị nguồn (2 tile), đo tới vị trí ban đầu của cửa. Đến gần áp công thức vị trí/góc gốc; ra xa xoay về tối đa10 độ/tick và vị trí hồi0.2/tick, chuẩn hóa25Hz. Chạy theo camera focus nhân vật hiện tại, không theo điểm spawn.

Renderer cập nhật riêng geometry các instance cửa dù model chỉ có1 key. Phát SOUND_DOOR02 cho cửa trượt, SOUND_DOOR01 cho cửa xoay lúc vào phạm vi; không lặp tiếng mỗi draw pass. Mô hình giữ tài nguyên gốc.

Đã kiểm kê14 placement các loại này trong World3, gồm2 placement type88 gần tọa độ0 có dữ liệu bất thường sẵn trong file; không tự di chuyển/chế vị trí. Nhánh Lorencia MODEL_DOOR_WOOD01/STEEL01 ở source bị comment nên không kích hoạt. Cổng sự kiện/công thành phụ thuộc packet và trạng thái riêng, không coi là cửa Devias.

Source nhánh này chỉ thay Position/Angle và phát Sound, không thay terrain attribute. Vì vậy giữ collision/pathfinding theo terrain gốc, không tự mở tile tường hoặc bỏ qua collision.

Test cửa kiểm tra khoảng cách biên200, trượt đúng hướng, góc xoay và quay về vị trí gốc, không sửa object nguồn, không áp sang map khác. Các test environment hiện hữu qua.

Full suite253/253 qua, Debug/Release đã publish. Browser offline dùng Data thật: ở215,27 cửa mở; đi tới220,27 cửa trở về đóng. Bằng chứng /tmp/mu-w92-door-open.png và /tmp/mu-w92-door-closed.png. Chưa xác nhận âm thanh bằng nghe trực tiếp hoặc cửa sự kiện online.
