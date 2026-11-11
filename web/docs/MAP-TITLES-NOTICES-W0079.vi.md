# W0079 — tách thông báo rõ vùng và bảng tên map

Thương hiệu cuối theo yêu cầu mới nhất: `Phát triển T-NLabs - Mu Online` (cùng vùng sự kiện).

## Đối chiếu source/ảnh người dùng

- `ZzzInterface.cpp::RenderNotices`: X=center+320, Y=bottom+300 trên khung640x480. Vùng notice đặt giữa ngang và Y62.5%, dưới giữa màn hình; vàng RGB255/200/80, nền đen alpha128. Đếm ngược do server phát text, không tự tạo timer đếm ngược giả.
- `wsclient.cpp::ReceiveNotice`: type1 dùng TYPE_SYSTEM_MESSAGE. Theo yêu cầu người dùng, Welcome/Welcome back/Welcome to/Chào mừng luôn vào log hệ thống kể cả server gửi type0, màu xanh hệ thống. Các notice sự kiện type0 còn lại, guildtype2 và slide10..15 vẫn vào vùng notice. Branding luôn ở cùng vùng sự kiện, kể cả nếu caller không truyền type0; chỉ đổi đúng chuỗi thương hiệu yêu cầu.
- `UIMapName.cpp`: ảnh166x90, X giữa màn hình, Y220/480, fade từalpha0.2, tăng/giảm0.015 ở nhịp native25Hz; giữ5giây ở alpha1. Map40ẩn. Web dùng elapsed time để cùng thời lượng khi browser60FPS. Bắt đầu sau khi scene tải xong, hủy khi đổi scene/map.

## Resource và triển khai

66 mapID ánh xạ vào32 ảnh gốc trong `Data/Interface/ImgsMapName/*.ozt`; filelocalized source yêu cầu trong `Local/Eng/ImgsMapName` không có bộ tên map (chỉ có AddStrife). Vì vậy dùng ảnh Interface hiện có, với alias rõ như source dungeun->Dungeon, stadium->Arena, kantru->Kanturu, lostkalima->Kalima, doppelganger->DoubleGoer. Không vẽ chữ thay ảnh, không tạo asset giả. Cấu hình custommap ngoài bảng chưa được suy đoán.

`audit_map_titles.py --source <UIMapName.cpp> --data <Data>` sinh bảng ID/path/bytes/SHA256. `stage_preview.py` chỉ chép đúng các ảnh trong bảng sau kiểm hash và containment. Giữ nguyên domain model manifest, không mở toàn bộ Data và không thay allowlist helper. Ảnh phục vụ cùng origin8080 trong debug; không cần restart helper/GameServer. Client kiểm hash/size trước decode. Ảnh OZT thực có kích thước166x90; tỷ lệ UV166/256,90/128 trong OpenGL gốc là padding power-of-two, web dùng đầy đủ166x90 tránh cắt mất chữ.

## Kiểm tra và giới hạn

213 test client qua, build debug/release trong thư mục tách riêng rồi deploy nguyên tử. Kiểm tra browser fixture: Welcome back trong log trái, sự kiện/thương hiệu chung vùng Y62.5%, ảnhLorencia đầy đủ, khoảng60FPS. Ảnh `/tmp/mu-w79-map-notices.png`. Chờ đủ thời lượng: bảng tên map tựẩn; không lỗiconsole. Không mua/bán/chuyển map trên tài khoản thật trong lượt này; cần refresh/vào lại nhân vật để nhận bản mới.

Không khẳng định tên map custom/Strife header/guild notice/slide animation đã fullparity; các giới hạn chat và multiplayer W0078 còn giữ nguyên. Ảnh map trong bộ Data có phong cách riêng, không sửa ảnh để giống bộ client khác trong ảnh tham khảo.
