# W0098 — máu pet, chuyển động pet và thú cưỡi

## Nguồn đối chiếu

- Main5.2/source/NewUIItemEnduranceInfo.cpp:514–713,1078–1079: khung HP helper, lấy Durability từ ô EQUIPMENT_HELPER, mẫu số 255; ảnh `Interface/newui_Pet_Back.OZT` và `newui_Pet_HpBar.OZJ`. NewUISystem.cpp:459 đặt (2,26); khung 57×23, thanh 49×3 tại (x+4,y+16).
- w_PetActionUnicorn.cpp:60–126, w_PetActionCollecter.h:9–10: vòng đứng bán kính 50 đơn vị, chu kỳ 4000 ms, ngưỡng 10, quay 8 độ/tick, tốc độ log(Distance)×2.3; Unicorn cao 200×Owner.Scale. Không tăng bán kính tùy ý.
- w_PetActionDemon/Stand.cpp: độ cao 230×Owner.Scale, quay 10 độ/tick, tốc độ log(khoảng cách bình phương)+5. Collector cao 20, Skeleton 50×Owner.Scale.
- GOBoid.cpp:121–220,270–693: tạo model thú cưỡi, tỷ lệ, sao chép vị trí/góc chủ, action đứng/đi/chạy/đánh và ẩn ở safe zone.
- ZzzOpenData.cpp:4211–4225: Rider01, Rider02, DarkHorse, fenril_red/black/blue/gold trong Skill; Uniria dùng texture unicon của Item, không dùng hình sừng inventory để dựng thú cưỡi.
- WSclient.cpp:2364–2388: Fenrir option 0 đỏ,1 đen,2 xanh,4 vàng. ZzzCharacter.cpp và _enum.h: pose cưỡi riêng của player, gồm nhánh RF.

## Thay đổi

Thanh HP nằm góc trái, cập nhật theo packet độ bền hiện có C1:2A. Tháo pet ẩn thanh; độ bền 0 vẫn hiện thanh rỗng. Khi pet chết, signature trang bị thay đổi một lần để bỏ tư thế/model helper; không dựng lại thân mỗi lần trừ một điểm độ bền.

Pet bay dùng độ cao của **chủ** thay vì lấy terrain tại vị trí pet, tránh trồi/sụt khi bay qua địa hình. Bỏ khởi tạo cộng thêm nửa ô, quay có giới hạn thay vì đổi hướng tức thì và cập nhật pose/animation mỗi frame thay vì giữ nguyên hình 40 ms. Chiều cao có Owner.Scale của DL/RF. Texture TGA bù V cho shader như nhánh cánh/Satan đã đối chiếu trước đó.

Thêm renderer thú cưỡi từ model world gốc, clip riêng, giữ vị trí/góc theo chủ trên main và remote; rider dùng pose cưỡi tương ứng. Dinorant có cao độ và clip khác ở Tarkan/Icarus. Dark Horse chỉ có class DL trong bảng item. Bộ thử Fenrir chọn đúng màu và bậc class theo Item_eng.bmd.

## Kiểm tra

- Bộ hồi quy đầy đủ 269/269 qua. Sau đó chạy lại nhóm liên quan 34/34 với ma trận mở rộng 43 cặp mount/màu/class hợp lệ và các sửa vòng đời cuối.
- Audit `PET-MOUNTS-W0098.json`: 7 model world, toàn bộ texture được đọc/giải mã/hash từ Data hiện tại, 30 mẫu/model =210 mẫu hữu hạn.
- Trình duyệt offline: Uniria, Dinorant, Dark Horse, Fenrir đỏ/đen/xanh/vàng trên main và remote; kiểm tra di chuyển, thanh HP, tháo/đeo. Ảnh `/tmp/mu-w98-*.png`; các file có hậu tố `verified` là Dark Horse được chụp ở cảnh riêng sau khi đối chiếu lại URL/menu của cảnh riêng.

## Giới hạn còn lại

Đây là xác minh renderer offline/Data, chưa chứng nhận sát từng pixel hoặc mọi tình huống chiến đấu server thật. Các nhánh particle/joint lửa-sét của mount, skill đặc thù, chuyển trạng thái hide/teleport/event đầy đủ và toàn bộ lệnh tự gom Zen của collector còn cần port/đối chiếu riêng. Dark Raven thuộc controller chiến đấu khác, không phải pet cưỡi của lượt này.

Độ mượt không suy ra từ FPS: cảnh nhỏ có mẫu gần60 FPS; khi nhiều tab cùng render, mẫu Dark Horse từng xuống khoảng39–54 FPS/p95 33–50ms. Cần đo hiệu năng riêng sau khi đóng các cảnh test song song; chưa tuyên bố giữ60 FPS ở mọi thiết bị.

### Vòng đời HP kiểm chứng cuối

CSS canvas toàn cục có thể thắng thuộc tính hidden. Thanh HP đã dùng display:none tường minh khi tháo/clear; thêm test hồi quy riêng (5/5 test pet/mount qua, gồm43cặp). Trình duyệt đã xác nhận `[data-pet-health]` hidden sau tháo, nhân vật chính về tư thế đứng, remote vẫn cưỡi pet riêng: `/tmp/mu-w98-pet-removed-verified.png`. Bản publish cuối W0090-20261030-140622.
