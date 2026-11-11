# W0102 — Orb, mesh cơ thể trong icon, số lượng inventory

Ngày 30/10/2026. Không thay đổi kích thước trang bị trên nhân vật hoặc dữ liệu item phía server.

## Đã sửa và căn cứ

1. **Orb bị phóng đầy ô:** `item-icons.js` trước chuẩn hóa bounding box mỗi model thành gần đầy canvas, làm mất tỷ lệ vật lý giữa item nhỏ và item lớn. Orb nhóm 12/7–14,16–19 nay dùng cùng tỷ lệ vật lý suy ra từ fallback `CScriptItem::FindItemRelative`: scale .0039, UI FOV 1°, depth 200, frame gốc 480 px. Giữ anchor .5/.8; không tự fit Orb cho đầy ô. Trang sức đã dùng cơ chế tương tự từ W0093. Số ô chiếm vẫn lấy Item_eng.bmd, 20×20 px/ô.
2. **RF dùng nhầm model đang mặc:** `ZzzInventory.cpp::RenderItem3D` và `MonkSystem.cpp` thay Armor 59/83 → Armor_inventory60; 60 → ArmorMale61_inventory; 61 → ArmorMale62_inventory; 73 → ArmorMale74_inven. Cả bốn file có trong Data/Player. Nay chỉ pipeline icon dùng các file này; armor ngoài map giữ model cũ.
3. **Pose của model inventory RF:** `RenderObjectScreen` vẫn dùng MODEL_PLAYER BoneTransform cho các MODEL_ARMORINVEN. Các file có bone name khác Player; chỉ API mới `inventoryArmorGeometryResult` cho đúng năm mã item được phép bind theo chỉ số bone, có kiểm tra bounds. Kiểm tra tên/parent của mọi trang bị thông thường vẫn giữ nguyên. WASM đã build lại.
4. **Chọn mesh, không chỉ lọc da:** `ZzzObject.cpp` có các nhánh HideSkin RenderMesh cho Helm 39–44 (2,1,0,2,1,2), Helm 59/60/82/83 (1), 61/73 (0). Armor inventory 73 giữ mesh 0 trong pass cơ bản. Nay chọn đúng ordinal trước tính bounds, tải texture và tạo upgrade overlays. Vẫn dùng bộ lọc skin/hair/nude/light texture cho các model khác. Không xóa các mesh này khỏi nhân vật ngoài map.
5. **Mặt trước giáp RF:** model inventory được author trên mặt XY, khác giáp đang mặc. Pipeline icon dùng góc [0,0,0] cho năm mã armor đặc biệt để giữ mặt trước của file Data, tránh nhìn từ trên xuống. Đây là quyết định dựa trên hình học Data; chưa xác nhận góc tương ứng trong bảng ItemTRSData tùy biến đang thiếu.
6. **Số lượng:** port `CNewUIInventoryCtrl::RenderNumberOfItem`. Các nhóm bình/potion gốc dùng durability >1; ngọc nén nhóm 12/30,31,136–143 dùng `(level đã decode + 1) × COMGEM::FIRST`, FIRST=10. Không shift level thêm lần nữa. Trang bị/pet không bị hiển thị độ bền thành số lượng.
7. **Vẽ số:** dùng atlas newui_number1.tga gốc, glyph 8.4×11.2, bước 6.72, vị trí quanh x=width−6/y=1, màu (1,.9,.7). Lớp số độc lập với canvas model nên không mất khi icon xoay/cập nhật hiệu ứng. Có trong túi nhân vật, personal store và shop NPC thông thường.
8. **Stack bổ sung:** Arrow/Bolt, nguyên liệu 13/32–34, Mark of Lord +3 và Lucky Coin lấy danh sách cho phép từ cấu hình GameServer Data/Item/ItemStack.txt hiện tại. Không diễn giải mọi durability thành số lượng. Cấu hình client GetMainInfo/ItemStack hiện rỗng; danh sách web này cần cập nhật nếu cấu hình server đổi.

## Kiểm tra

- [Kiểm kê toàn bộ resource](ITEM-RESOURCES-W0102.json): 924 item có tên/kích thước; 902 item / 903 model variant đã ánh xạ đọc được model và texture, không thiếu file trong tập đã ánh xạ; 22 item chưa có ánh xạ. Đây là kiểm kê resource, không phải chứng nhận hình ảnh của mọi item.
- Full suite client **283/283 qua**, debug/release build thành công. Sau sửa mặt trước RF, nhóm 59 test liên quan cũng qua. WASM build thành công.
- Test đọc BMD RF/Orb thật, đối chiếu tên file với MonkSystem.cpp và bảng ordinal mesh trong ZzzObject.cpp; kiểm tra giới hạn API inventory mới.
- Browser fixture `?scene=ui-check&map=0&x=140&y=125&itemreview=1`: tám Orb, Sacred Fire Helm/Armor/Boots, Storm Jahad Armor, Piercing Groove Armor, Violent Wind Helm; bình 43/52; Bless/Soul/Life bundle 10/20/30. Các icon nạp được, không có iconMissing; số lượng hiện bằng atlas gốc. Fixture không dùng account thật, không mua/bán/thay đồ server.

## Giới hạn để không báo hoàn tất sai

- `Data/Local/ItemTRSData.bmd` mà Main5.2/ScriptItem.cpp đọc **vẫn chưa tìm thấy** trong Data, thư mục bổ sung hoặc Downloads. Đây là bảng góc, vị trí, scale riêng từng item. Vì vậy chưa thể chứng nhận kích thước/góc của toàn bộ item trùng client gốc 100%.
- Lượt này sửa tỷ lệ nguồn cho Orb, các model/mesh cơ thể đã xác định và số lượng. Các nhóm khác ngoài trang sức/Orb vẫn dùng chuẩn hóa bounds hiện hữu, chưa được chuyển toàn bộ sang bảng TRS gốc. Không thay toàn bộ scale theo cảm tính khi thiếu bảng này.
- 22 item chưa ánh xạ và mọi góc/hiệu ứng chưa được đối chiếu trực quan đều còn trong phạm vi cần kiểm tra tiếp; không đánh dấu chúng đạt chỉ vì test decode qua.
- Các dấu số lượng trong kho/GUI NPC mở rộng và khi đang nhấc item chưa được kiểm tra trong lượt này.
