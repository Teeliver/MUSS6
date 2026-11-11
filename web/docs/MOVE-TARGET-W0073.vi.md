# W0073 — Con trỏ, dấu chọn đích và hiệu ứng tới nơi

## Yêu cầu đã chốt bằng ảnh

Ảnh người dùng ngày 26/10/2026 cho thấy dấu chỉ điểm đến có vòng hoa văn vàng, bốn tia và mũi tên trắng. Dấu này hiện khi chọn điểm. Hiệu ứng vàng cũ giữ riêng khi nhân vật tới nơi. Không đồng nhất dấu đích với con trỏ chuột.

## Đối chiếu source

- Main5.2/source/ZzzOpenData.cpp:5444–5445 nạp Cursor.tga và CursorPush.tga; RenderCursor trong ZzzInterface.cpp dùng kích thước24, điểm nóng2,2. Web giải mã OZT gốc rồi tạo native CSS cursor cho canvas; cập nhật khi resize/nhấn/thả/blur, không tạo ảnh mỗi frame.
- ZzzInterface.cpp:7420–7448 tạo MODEL_MOVE_TARGETPOSITION_EFFECT khi gửi đường đi.
- ZzzEffect.cpp:4848 tạo composite model, BITMAP_MAGIC subtype11 và cursorpin02; lifetime30tick. Subtype11 thực sự render BITMAP_SUMMON_IMPACT, không phải Magic_Ground1. ZzzOpenData.cpp:5524 ánh xạ sang Effect/empact01.jpg.
- ZzzEffect.cpp:1339/8389/19700: vòng hoa văn scale0.8 và0.96, xoay hai chiều2độ/tick, lifetime24tick. cursorpin02 co/giãn0.8–1.8. Web dùng đúng empact01.OZJ và cursorpin02.OZJ; quad dùng nửa chiều rộng native để tránh phóng đôi.

## Thay đổi

Click đất hợp lệ hiện dấu đích ngay; nhấn chuột dùng CursorPush riêng. Khi ACK xác nhận và nội suy chạy hết đường mới hiện hiệu ứng tới nơi. Đổi đích khi request cũ đang chờ sẽ lập lại đường từ endpoint ACK, tránh đi quá đích mới. Esc, phím hướng, correction, chết, blur và dispose xóa dấu cũ. Click ô đang đứng không tạo đường giả.

Hai giai đoạn dùng cùng renderer và resource, không tạo WebGL context mỗi lần click. Vòng được chia lưới và lấy cao độ địa hình tại mỗi đỉnh để tránh bị đất che mất một phần. Hiệu ứng đích có vòng hoa văn và model màu trắng; hiệu ứng tới nơi giữ hình vàng trước đó theo yêu cầu người dùng. Thời điểm tới nơi là tùy chỉnh được yêu cầu, không khẳng định đây là thời điểm nguyên bản source.

## Kiểm tra và giới hạn

172 client test qua; 4 helper test qua. Test mới kiểm tra hai cursor, pointerup ngoài canvas, resize/hotspot, cleanup; dấu đích xuất hiện ngay, arrival chỉ sau ACK và hoàn tất nội suy; đổi đích trong lúc chờ ACK. Build debug83/release31, stage179asset debug; không sửa Data gốc.

Đã mở Lorencia offline để kiểm tra resource cursor và click di chuyển. Đây là mô phỏng không packet/damage, không phải kiểm chứng server thật. Source còn các spear joints và spark particles chưa được port; composite chưa đạt toàn bộ pixel/particle parity. Cursor NPC/attack/get/talk riêng chưa nằm trong thay đổi này.

## Điều chỉnh kích thước sau đối chiếu ảnh

Theo yêu cầu tiếp theo của người dùng, toàn bộ hiệu ứng tới nơi nhân scale0.3 so với bản cũ: vòng lớn nhất khoảng1.08tile, gần vòng chọn đích0.96tile. Dấu chọn đích không đổi kích thước. Build lại debug/release sau thay đổi hệ số; không thay đổi packet hay điều kiện arrival.
