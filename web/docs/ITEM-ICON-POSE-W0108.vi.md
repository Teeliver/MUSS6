# W0108 — Rà lại pose cánh và Violent Wind Helm

Ngày: 2026-10-31.

## Đã triển khai và kiểm tra
- Wings of Heaven: Data `Item/Wing02.bmd`, texture `angel_wing.tga`, action0 frame0. W0105 dùng X=90 thay X=270 để sửa hướng nhưng thay đổi cả mặt trước/sau. Nay giữ X=270, Y=-10, đổi roll Z=180 trong mặt phẳng icon. Điều này giữ tọa độ depth của góc cơ sở thay vì lật depth như cách trước. Góc roll là hiệu chỉnh của web theo lỗi hiển thị, không được gọi là giá trị lấy từ ItemTRSData.
- Cảnh offline `itemreview=1&iconreview=1`: cánh ở slot7 và12, nón slot16, Apple18, nhẫn19. Không thay đổi item server. Đã mở trực tiếp trên trình duyệt, xác nhận canvas Wing02.bmd ở cả hai chỗ, hover data-rotating=true, rời chuột false và trở về góc tĩnh.
- 97 test mục tiêu qua cho bản cuối; debug142 file/release38. Không thay đổi renderer cánh trên nhân vật ngoài map.

## Thử nghiệm đã rút lại
- W0107 thử hệ số fallback .0039 cho nón hở mặt 39..44. Trình duyệt cho thấy nón thành chi tiết quá nhỏ, không đạt yêu cầu. Đã rút thay đổi này trước khi chốt W0108; nón hiện giữ auto-fit như trước. Không khẳng định tỷ lệ native đã đúng.
- `HelmMale40.bmd` có mesh0 đầu, mesh1 tóc, mesh2 phần nón. HideSkin gốc ở ZzzObject chọn mesh2; web giữ cùng lựa chọn, không lộ đầu/tóc.

## Source đã rà lại
- ScriptItem.cpp::FindItemRelative: X=.5/Y=.8/scale=.0039 và góc fallback; per-item lấy ItemTRSData.
- ZzzInventory.cpp::RenderItem3D/RenderObjectScreen: model riêng, Player action0, BodyHeight, ScaleMA(.10/.07), phép chiếu UI FOV1 độ.
- Math/ZzzMathLib.cpp::AngleMatrix: (Z*Y)*X.
- ZzzBMD.cpp::Animation/Transform: ma trận bone và scale.
- ZzzObject.cpp::RenderPartObject và HideSkin của Helm39..44.
- Kiểm tra danh sách Source.rar, SOURCE-2.0.7.zip, ServerNew_up.zip; không thấy bảng TRS. Release2/ZzzInventory.obj trong Source.rar cũng dùng signature renderer mới. main.exe trong MuServer Tools là PE đã đóng gói VMP; không chạy file này. Main5.2/Release/Main.exe có mã gốc nhưng bảng TRS là dữ liệu ngoài.

## Còn thiếu để xác nhận native parity
Chưa có thông số per-item của đúng client để chốt tỷ lệ nón và so sánh tuyệt đối icon cánh. W0108 sửa sai mặt nhìn của lần sửa web trước, không phải chứng nhận khớp từng pixel với client native. Giữ giới hạn này tách biệt với các kiểm tra resource/mesh/hover đã qua.
