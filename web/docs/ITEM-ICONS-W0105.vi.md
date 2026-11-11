# W0105 — Icon Wings of Heaven, Apple và trang sức

Ngày: 2026-10-31.

## Phạm vi
- Chỉ renderer icon inventory/equipment; không sửa cánh, pet hay trang bị trên nhân vật ngoài map.
- Wings of Heaven (12:1, 6145): vẫn `Item/Wing02.bmd`. Đổi góc X icon 270 → 90 độ để sửa hướng trên–dưới được người dùng đánh dấu. Đây là hiệu chỉnh riêng cho bộ Data hiện tại, không phải giá trị trích từ bảng ItemTRSData còn thiếu.
- Apple (14:0, 7168): vẫn `Item/Potion01.bmd`. Bỏ auto-fit phóng model lấp ô; dùng hệ số vật lý .0039 dự phòng của `CScriptItem::FindItemRelative`, UI FOV 1 độ/depth 200 giống các item đã xử lý. Canh theo tâm hình học sau phép xoay.
- Ring of Ice (13:8, 6664) và các trang sức đã dùng physical scale: vẫn model gốc (`Item/Ring01.bmd` cho Ring of Ice), giữ nguyên tỷ lệ đã có, chuyển tâm xoay/hiển thị về midpoint bounds, bỏ anchor Y=-.6. Không thu nhỏ đồng loạt ngọc hoặc Orb.

## Kiểm tra
- 81 test mục tiêu qua; build debug 140 file, release 38 file, xuất bản bằng thư mục build cách ly để không xóa dist đang phục vụ.
- UI offline `?scene=ui-check&map=0&x=140&y=125&itemreview=1` bổ sung cánh slot7, Apple slot60, Ring of Ice slot61, không thay đổi dữ liệu server.
- Trình duyệt xác nhận canvas dùng Wing02.bmd/Potion01.bmd/Ring01.bmd và quan sát icon sau khi mở túi. Không giao dịch hay thay đồ của nhân vật thật.

## Giới hạn
Bộ source dùng ItemTRSData.bmd cho góc/vị trí/tỷ lệ riêng từng item nhưng file này chưa có trong dữ liệu được cung cấp. Vì vậy không khẳng định tỷ lệ Apple hoặc góc icon là trích chính xác từ bảng gốc; phần sửa dựa trên fallback source, model Data và lỗi ảnh người dùng. Chưa cần thay đổi renderer thế giới.
