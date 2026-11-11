# W0099 — sửa thanh HP lệch và controller Unicorn

## HP

Bản W0098 vẽ thanh đỏ tại x=0 thay vì x=4 và co toàn bộ texture (gồm phần padding) vào khung. Đây là lỗi thật, không phải dữ liệu nhân vật.

Đối chiếu Main5.2/source/NewUIItemEnduranceInfo.cpp:565–568 và NewUICommon.cpp:221–232: khung 57×23 lấy texel từ (.5,.5) đến (56.5,22.5); thanh 49×3 đặt tại (x+4,y+16). RenderImage lấy vùng texture theo chiều rộng đang vẽ, không kéo toàn bộ ảnh thanh máu. `paintPetHealth` đã sửa cả inset lẫn crop; giữ nguồn máu Durability/255.

## Unicorn

W0098 đã đưa việc quay/di chuyển từ tick gốc sang từng frame. Việc tính lại vận tốc của tick trước ở mỗi frame làm thay đổi quỹ đạo; chỉ quay khi ngoài FlyRange cũng khác nhánh Stand của Unicorn (điều kiện gốc 80 >= FlyRange luôn đúng). Đồng hồ vòng bay còn lấy thời gian tuyệt đối của trang, trong khi PetObject::GetNowTick tính từ lúc pet được tạo.

Đối chiếu w_BasePet.cpp:60–118,145–185; w_PetActionUnicorn.cpp:60–126; ZzzAI.cpp:91–126,238–260:

- Tạo Unicorn cách chủ 100 đơn vị ở góc ngẫu nhiên và giữ góc của chủ như nguồn, thay vì đặt vào vòng 50 rồi dùng hướng không tương ứng.
- Dùng đồng hồ riêng từ lúc pet được tạo; target vòng 50 đơn vị/4000 ms.
- Mô phỏng 25 tick/s: quay 8 độ, dùng vận tốc tick trước để di chuyển, sau đó tính tốc độ mới log(Distance)×2.3 với ngưỡng10.
- UpdateModel tăng frame bằng velocity trước UpdateMove; velocity ban đầu .8, khi dừng ×.35, khi di chuyển ×1.2. Hình vẽ dùng nội suy pose và frame giữa các tick, không đổi quy tắc theo FPS màn hình.
- AlphaEnable=false của pet gốc tăng alpha theo (target-alpha)×.1/tick; đã thêm xuất hiện dần cho Unicorn.

Đã kiểm tra renderer pet giữ depth test/cùng camera với player; không thêm chế độ luôn vẽ pet lên trước player hoặc tự nâng độ cao/bán kính để che lỗi. Độ cao vẫn 200×Owner.Scale từ nguồn. Trong phối cảnh, hình pet vẫn có thể chiếu lên vùng màn hình của nhân vật khi bay phía trước; không đồng nghĩa hai model có cùng vị trí world.

## Kiểm tra

7 test pet/mount qua, gồm bất biến đường bay **và frame hoạt ảnh** ở30/60/144Hz, kiểm tra inset/crop HP, vòng tháo/đeo,43cặp mount/class. Nhóm hồi quy liên quan37/37 qua; không dùng con số269 của bản cũ để chứng nhận toàn bộ thay đổi mới.

Kiểm tra trình duyệt dùng cảnh offline riêng; không đổi trang bị hoặc refresh phiên game thật của người dùng. Cần xác nhận lại với người dùng sau khi phiên game nhận mã mới trước khi tuyên bố lỗi quan sát được đã hết trong mọi tình huống.

Bản publish cuối: W0090-20261030-143500. Ảnh HP Imp đầy255, đã kiểm tra cùng tỷ lệ HUD: `/tmp/mu-w99-hp-full.png`, `/tmp/mu-w99-hp-detail.png`. Thanh đỏ nằm gọn trong khung ở x+4; tên pet không bị kéo theo padding texture.

### Chốt với người dùng

Người dùng chọn **Giữ vị trí theo source gốc** sau khi đã được giải thích pet có thể che vùng đầu theo phối cảnh khi bay phía trước. Không thêm khoảng cách/độ cao chống che hình ngoài source. Cảnh cuối đứng/di chuyển với main và remote đã xem, mẫu60FPS/p9517.7ms: `/tmp/mu-w99-unicorn-idle.png`, `/tmp/mu-w99-unicorn-moving.png`. Ảnh tĩnh chỉ minh họa pose, kiểm tra nhịp dựa thêm test bất biến30/60/144Hz.
