# Item UI 0.28.5

Inventory và Warehouse tô nền theo toàn bộ footprint của item. Nền thường
RGBA (0.3,0.5,0.5,0.6); item thuộc nhóm có cảnh báo độ bền chuyển vàng/cam/đỏ
theo các mốc 50%, 30%, 20%, 0. Equipped slot chỉ tô cảnh báo độ bền, không phủ
nền thường lên hình ô trang bị. Hover không cộng thêm một lớp màu trùng.

Đối chiếu CNewUIInventoryCtrl::Render/SetItemColorState và ZzzInventory.cpp
trong engine tham khảo /tmp/muonline-client-sources-master. Đây là engine tham
khảo khác đời; chưa đủ bằng chứng để gọi toàn bộ client giống EX603 100%.

ItemAttribute EX603 84 byte đọc MagicDur tại 48, sáu yêu cầu WORD tại 50..61,
và bảy class tại 69..75. Công thức yêu cầu đối chiếu thêm GameServer/Item.cpp
của source được cung cấp. Không sửa Data, font Gulim hoặc database để tạo kết quả.

Tooltip dùng Text.bmd của ngôn ngữ đang chọn: 76/73/75/1930/77/698 cho level,
Strength, Dexterity, Vitality, Energy, Leadership; 74 cho lượng thiếu, 61 cho
class được mặc. Thiếu chỉ số hiện đỏ; sai class/tier dùng chữ trắng nền đỏ sẫm
theo TEXT_COLOR_DARKRED của source tham khảo. Nội dung vẫn căn giữa.

Chỉ số lấy từ packet F3:03/E0 (base View 32 bit), E1 (bonus), 05 (level),
06 (tăng điểm). So sánh base + bonus, không dùng giá trị giả của UI. Mất context
thì không tự kết luận thiếu điều kiện. Server vẫn quyết định việc mặc đồ.

Giới hạn: chưa chuyển hết giảm yêu cầu Harmony/socket, lucky item và mọi pet/
custom wing đặc biệt. Không dùng phép so sánh tooltip hiện tại để chặn packet
mặc đồ; tránh chặn sai item đã được server chấp nhận.

Contract kiểm tra packet thật được serialize, chỉ số vượt 65535, màu nền theo
độ bền, pixel nền ô item, màu/độ thiếu yêu cầu và font. Ảnh QA là fixture
offscreen; không phải bằng chứng đo FPS trong phiên game thật.

## 0.28.6 — viền ô và lớp hiển thị

Nền bag/warehouse tô từng ô 18x18 bên trong ô 20x20, chừa đường viền. Trang
bị tô trong rect inset 2, không phủ ra ngoài khung. Inventory/Warehouse host
layer-backed để AppKit không vẽ nền đè lên Metal child khi redraw. Hover
camera tính thêm bán kính X/Z khi xoay, tránh model giày bị cắt ở mép viewport.
Nền xanh xám và cảnh báo độ bền vẫn dùng cùng policy; không phân màu theo
Excellent hoặc level cộng. Hình dáng/color resource không bị sửa.

## Bảng nhân vật 0.28.6

C hoặc nút đầu HUD mở bảng cố định bên trái, Esc/X đóng; không đổi camera.
Khung 190x429, vị trí stat 120/175/240/295/350 và ảnh lấy từ
CNewUICharacterInfoWindow::LoadImages/RenderFrame của engine tham khảo.
Chỉ số thưởng xanh (100,150,255), cơ bản vàng (230,230,0), font Gulim;
Text.bmd keys 166/1702/169/168/1900 cho stat, 200/201/217 cho level/exp/point.
Leadership chỉ hiện cho DL. Inventory vẫn mở được cùng bảng nhân vật.

Điểm còn lại dùng ViewPoint 32 bit; combat nhận từ F3:E1. Damage áp Mul trước
Div như Main_EX603/Main/PrintPlayer.cpp::GetViewPhisyDamage; HP/MP lấy vitals.
Nút cộng điểm dùng C1:F3:06, type 0 STR / 1 DEX / 2 VIT / 3 ENE / 4 LEAD; mỗi click một điểm, chờ ACK rồi cập nhật ViewPoint/base. Chưa có quest/pet/master UI.
Chưa bổ sung damage của skill đang chọn, curse và các dòng class đặc biệt.
Không coi ảnh fixture QA là dữ liệu nhân vật thật hay phép đo FPS.

Armor icon: pose từ Player.bmd action 0 theo RenderObjectScreen MODEL_PLAYER,
không dùng skeleton bind của BootMale riêng. Cùng một scene/camera scale cho
ảnh tĩnh, effects và hover; camera envelope tính trước lúc cache, không thay
scale khi rê chuột. Pair boots xoay cùng root/pivot, không tách/xoay từng mesh.
Kích thước hiện tại vẫn cần so hình Windows cùng resolution để xác nhận pixel
parity; chưa chuyển hết bảng Scale/Position ngoại lệ của RenderObjectScreen.
