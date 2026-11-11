# W0074 — NPC, shop và resource item

Phạm vi: đối thoại cơ bản, shop NPC thường, mua/bán/repair theo ACK; sửa resource và màu đồ cộng. Không đánh dấu toàn bộ giao diện quest/kho/chaos mix là hoàn tất.

## Source đối chiếu

- `Main5.2/source/wsclientinline.h:698–783`: nói chuyện, mua, bán, sửa dùng `Send(TRUE)` (wire C3); đóng dùng `Send(FALSE)` (C1:31). Chú thích C1 trong server struct chỉ thể hiện layout, không thay thế chính sách mã hóa. Bản đầu W0074 đã gửi nhầm C1, gây Session closed; đã sửa và thêm test so với client native đang chạy.
- `GameServer/NpcTalk.cpp:1428–1575`: chọn NPC, kiểm tra điều kiện, result0 mở shop, đóng interface. `NpcTalk.h`: reply11byte.
- `GameServer/Shop.cpp:235–375`, `Shop.h`: stock C2:31, header6, mỗi item13byte, 120ô; giá custom F3:E7 mỗi record20byte. Thuế B2:1A type2.
- `GameServer/ItemManager.cpp:4255–4590`: mua C3:32 trả16byte; bán C3:33 trả8byte và Zen LE; item mua stack/pet có thể trả FF đồng thời gửi thay đổi túi/Zen riêng. Không trừ tiền hoặc thêm/xóa đồ theo dự đoán phía web.
- `Main5.2/source/NewUINPCShop.cpp`: khung190×429, lưới8×15 tại15,50, ô20×20; repair tại54,390 và98,390. `ZzzInterface.cpp:3174`: NPC có sửa đồ243/246/251/416/578. Sửa tại NPC type0, không áp điều kiện tự sửa cấp50.
- `ZzzInterface.cpp:8640`, `ZzzOpenData.cpp:5444–5452`: CursorTalk sheet2×2, sáu nhịp100ms (0,1,2,3,2,1), kích thước24, hotspot2,2. Kanturu38 dùng CursorAttack2. Con trỏ thay khi hover NPC được render, không dùng ảnh CSS vẽ lại bằng cảm tính.

## Luồng đã triển khai

Click NPC đi gần trong khoảng3tile rồi gửi yêu cầu. Giao diện shop/Inventory dùng ảnh frame gốc; item giữ footprint, xoay hover và tooltip. Chọn item shop để mua; nhấc item từ túi/trang bị rồi chọn ô shop để bán. Vật phẩm quý có xác nhận; hủy không gửi packet. Đóng/Esc gửi C1:31 và trả item đang nhấc.

Mỗi lúc chỉ một giao dịch. Đang chờ mua/bán thì không đóng, không gửi lặp. Timeout mua/bán giữ khóa tới ACK muộn hoặc đồng bộ bằng đăng nhập lại; timeout nói chuyện gửi đóng interface rồi cho thử lại. NPC chat target01 hiển thị nội dung server; các result giao diện chuyên biệt chưa port chỉ có thông báo và Đóng, không giả lập quest thưởng.

Giá dùng metadata Item.bmd và 208 rule ItemValue/36 rule stack từ config public. Rule giá/stack giữ thứ tự như server; thuế cộng sau làm tròn. Có hỗ trợ giá coin gửi theo packet. Các nhánh pet/socket/custom giá đặc biệt chưa thể hiện đủ mọi công thức; nếu không biết hiển thị giá do server xác nhận. Zen/giao dịch thật luôn theo server. Không xuất config account/DB.

## Resource và đồ cộng

Armor of Guardsman13:29: model Item/EventChaosCastle.bmd nhưng texture Npc theo OpenTexture gốc. Sửa ngoại lệ này, shared flareRed của Crimson Glory và các thư mục texture khác.

Generator đọc AccessModel/OpenTexture trong ZzzOpenData, MonkSystem, ChangeRingManager; mở rộng vòng lặp hằng, chỉ nhận model tồn tại, bỏ trường hợp nhánh mâu thuẫn. Data một số file đã chuyển thư mục: chỉ nhận khi tên model đúng source và duy nhất trong cùng họ Item/Player. LuckyItem giữ đường dẫn thư mục62–72. Tên texture có dấu cách được xử lý hợp lệ; vẫn chặn traversal.

941 binding model hiện có, 131 ngoại lệ thư mục texture. Helper chỉ mở thêm đúng file đã được generator xác thực; không mở tùy ý thư mục con. Audit toàn bảng924item có tên: 902item có đủ model/texture (903biến thể model), 22item chưa có binding chốt được, 0lỗi thiếu texture trong các binding đã hỗ trợ. Xem [báo cáo đầy đủ](artifacts/item-resources-W0074.json). Số liệu này là audit byte/path, không khẳng định mọi model đã QA từng pixel.

Bỏ chặn hiệu ứng theo group>11. Mốc3/5/7/9/11/13 dùng các lớp màu/chrome/metal như source, áp cho shop, inventory và đồ cầm chung policy. Generator biên dịch trực tiếp switch RenderPartObjectEffect ra70 ngoại lệ: DevilEye/Key/Invitation đổi cấp hiển thị; wings và một số item giữ cấp hiệu ứng riêng. Oath/seed/helper có các lớp đặc biệt tương ứng. Các loại item đặc biệt còn nhánh render/particle/custom khác chưa port trọn vẹn; không khẳng định parity100%.

Cache icon tối đa256model đang dùng/cached (đủ120shop+76inventory), loại cache không còn view trước khi báo budget; cache bytes được quay vòng thay vì biến item thành lỗi khi test nhiều shop. Thả view icon shop khi đóng.

## Kiểm chứng

- ABI30struct sinh từ header server UPDATE603/EXTRA1 bằng clang; test mã hóa đối chiếu hàm gửi và native client.
- Test mua/bán thành công, bị từ chối, ACK muộn/timeout, khóa thao tác, stock lỗi/trùng, repairNPCtype0, cursor crop/frame, hướng chọn NPC `npc:true`, resource đặc biệt và bảng màu.
- Trình duyệt live: mở shop Lumen thành công, nhận RTT ACK talk27.9ms, hiển thị tax0%, danh sách hàng thực và Inventory; không mua/bán/sửa đồ thật trong lượt kiểm tra của agent.
- Offline UI: kiểm tra120ô, frame gốc và lưới cạnh Inventory; fixture không kết nối server. Ảnh QA lưu trong /tmp.
- Bộ cuối:182/182client test qua,4/4helper test qua; build debug88/release32, stage179asset debug. Người dùng xác nhận hình Guardsman và màu item đã ổn sau refresh.
- Giới hạn trải nghiệm vừa được người dùng chỉ ra: icon lần đầu mở túi/shop hiện dần do fetch/decode và queue build tuần tự; cache giúp các lần sau nhưng chưa có prewarm trong giai đoạn vào game. Chưa coi first-open latency này là hoàn tất tối ưu; cần đo riêng thời gian tới icon đầu/cuối, preload theo inventory hiện tại và NPC stock, tránh tải cả924item ngay khi mở game.

## Giới hạn còn lại

22binding chưa chốt ghi đầy đủ trong báo cáo (Divine/Test armor, Grand Soul Pants nhánh t_, vài item cash/event cũ). Không gán model gần giống để che lỗi. Không port toàn bộ quest, warehouse, chaos mix, gambling shop hoặc buyer UI cá nhân trong mốc này. Xác nhận đồ quý hiện dùng hộp HTML; skin popup chưa phải bản sao hoàn chỉnh native. Các custom item shader/particle và dữ liệu ItemRelative còn cần đối chiếu tiếp.
