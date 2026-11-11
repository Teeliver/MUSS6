# W0076 — Vật phẩm rơi, nhặt/thả và thay trang bị không dựng lại map

## Chức năng

- Đồ rơi từ server: C2:20, mỗi bản ghi16byte gồm key của vật phẩm trên map, tọa độ,12byte item. Key map khác index loại item. Bit0x80 là hiệu ứng vừa rơi. Despawn dùng **C2:21** header5byte, không phải C1.
- Hiển thị BMD/texture thật, tên item/mốc+, Zen, màu tên và các pass nâng cấp đang có. Góc/tỷ lệ được compile trực tiếp từ `Main5.2/source/ZzzObject.cpp::ItemObjectAttribute/ItemAngle`, kèm `Check_LuckyItem`;62 nhóm pose cho8192index, hash nguồn trong ground-item-policy.js. Đồ mới có động tác rơi theo Gravity20, giảm6 mỗi tick gốc; độ cao dừng70 cho vũ khí,30 cho nhóm khác. Áo/nón dùng BodyHeight gốc và ẩn skin/hair như item-only view.
- Hover dùng `Interface/CursorGet.ozt` theo RenderCursor khi SelectedItem!=-1;24logicalpx, hotspot(2,2). Click đồ xa đi đến gần rồi gửi C3:22. Space chọn đồ gần nhất trong vùng2ô; không gửi lặp khi đang chờ ACK.
- Giữ Alt: hiện tên. Alt+Space bật hiện tên liên tục, thả tổ hợp vẫn hiện; bấm Alt lần nữa tắt. Option của Mac được browser ánh xạ Alt nên Option+Space dùng cùng luồng. Không kích hoạt khi nhập input/textarea, hay Space đang dùng cho button.
- Nhấc đồ trong túi/trang bị rồi click đất: raycast đúng tile và gửi C3:23; đồ quý có bước xác nhận. Không xóa đồ cục bộ trước ACK. Giữ khóa khi timeout chờ phản hồi muộn để tránh gửi giao dịch trùng.
- ACK nhặt20byte, EXTRA1 ViewIndex ở16; resultFE là Zen, FD là gộp stack, FF thất bại. Inventory/money cập nhật theo server. Đồ dưới đất chỉ mất theo gói despawn của server.

## Quyền nhặt — source server là quyết định cuối

`Monster.cpp::gObjMonsterDieGiveItem()` gọi `gObjMonsterGetTopHitDamageUser()`: thông thường chọn người/party có tổng damage cao nhất còn hợp lệ, không chỉ LastAttackerID/đòn cuối. Có kiểm tra thời gian hit hợp lệ, cùng map, khoảng cách20. Event/script có luồng rơi riêng.

`MapItem.cpp::CreateItem/Convert` đặt `m_LootTime = GetTickCount() + ItemDropTime * 500`; `m_Time = now + ItemDropTime * 1000`. File đang đối chiếu `/Users/angelale/Downloads/MuServerEX603/GameServer/DATA/GameServerInfo - Common.dat` có ItemDropTime30: **15giây ưu tiên**, đồ tồn tại30giây theo config. MoneyDrop đặt LootTime0. Không thay config này.

`Map.cpp::CheckItemGive` kiểm tra trong ±2ô mỗi trục, item còn sống/chưa được nhặt. Khi còn khóa, người ngoài owner bị chặn; cùng party được nhặt đồ thường, quest item không có ngoại lệ party này. `ItemLoot.txt` có thể cấm loại item với người khác bằng LootUserIndex kể cả sau thời gian ưu tiên.

Khi bị chặn, server gửi Message260: `You can not pick up this item at this time` bằng C1:0D type1 rồi ACK22 resultFF. Client hiển thị nguyên văn trong log thông báo; màu TYPE_SYSTEM_MESSAGE gốc RGB(100,150,255), nền đen. Không tự phỏng đoán thông báo về chủ sở hữu khi FF có nguyên nhân khác. Packet đồ rơi không gửi owner/deadline, nên client không dựng đếm ngược giả hoặc tự cho phép nhặt.

## Tháo/mặc trang bị

Trước đây `equipment()` gọi lại `world()`, dispose renderer/map/NPC/motion và dựng toàn cảnh. Nay nhân vật có renderer riêng: giữ nhân vật cũ khi tải phần thay thế; chuẩn bị BMD/texture/animation rồi swap riêng actor. Map, terrain, NPC, camera và hàng đợi chuyển động giữ nguyên. Chỉ phiên thay đồ mới nhất được commit; build cũ/map cũ bị hủy và giải phóng. Cập nhật cả tư thế cầm vũ khí và khả năng chạy theo bộ trang bị. Chỉ kích hoạt sau snapshot/ACK hợp lệ, không mặc đồ lạc quan trước server.

Có diagnostic DOM `data-world-scene-generation` và `data-actor-equipment-revision` để kiểm chứng thay actor mà không dựng lại scene. Không có cache lưu bền hay prewarm toàn bộ resource mới trong mốc này.

## Kiểm tra

Fixture riêng, không kết nối GameServer: `?scene=terrain-check&map=0&x=140&y=125&walk=1&loot=1&gear=1`. Có nút rơi lại đồ và tháo/mặc Kris. Đã nhặt bằng click và Space, kiểm tra Alt+Space latch/tắt bằng Alt, hiển thị Kris+9/Small Shield/1234Zen. Tháo/mặc: actor revision0→1→2, scene generation giữ1; FPS quan sát khoảng60, p95 khoảng17.5ms trong fixture. Ảnh `/tmp/mu-w76-ground-names.png`.

Test protocol cover wide spawn/despawn, Zen encoding, ACK-only inventory, thất bại/timeout/phản hồi muộn, notification quyền nhặt nguyên văn. Test actor swap cover late/stale completion và invalidate khi đổi map. ABI sinh từ64 struct thực tế UPDATE603/EXTRA1.

Giới hạn: chưa kiểm chứng quyền nhặt bằng hai tài khoản thật, không agent thả/tiêu hao đồ thật. Item thiếu model theo audit W0074 vẫn báo thiếu; chưa full shiny particle/sound/nhấp nháy tên như native. Đây là log thông báo server tối thiểu, chưa full chat UI. FPS fixture không phải cam kết mọi máy/map đều60FPS.

## Bản đưa lên8080

Toàn bộ203 test client qua; build97 debug/32 release, stage179 preview assets. Bản chỉnh thêm tránh lấy packet chia Zen party làm ACK cho yêu cầu đang nhặt đồ thường đã chạy lại13 test session và rebuild. Helper nạp63 UI/table assets, giữ nguyên giới hạn Origin/Host. Mở fixture bằng chính8080 xác nhận tên Kris+9/Small Shield/1234Zen, cursor native nạp được; tháo/mặc actor revision2 trong scene generation1, FPS quan sát60/p9517.6ms, không console error. Ảnh `/tmp/mu-w76-live-build-ground-equipment.png`. Đây là kiểm tra renderer/fixture, không phải giao dịch đồ thật.
