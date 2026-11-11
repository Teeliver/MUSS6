# Character resources — native 0.28.2

`character_resources.cpp/.mm` là nơi dùng chung để chọn và nạp thân nhân vật.
Cả `character_scene.mm` và `world_view.mm` sử dụng module này; không giữ một
bộ model DW cố định riêng cho game nữa.

- Decode class, chuyển chức và 5 loại giáp từ CharSet[18] của server.
- Base skin: Helm/Armor/Pant/Glove/BootClass01..07; chuyển chức Class2/3
  theo GetSkinModelIndex, không tìm Class2 cho MG/DL/RF.
- Giáp đối chiếu ZzzOpenData::OpenPlayers, RenderPartObject và MonkSystem:
  Male/Elf, MaleTest, t_PantMale19, HDK, CW, Summoner ElfC, RF Monk,
  Dark Lord mask; không đoán model cho item custom chưa có mapping.
- Vào game dùng chính Character từ danh sách Session vừa đọc lại; kiểm tra
  base class khớp trường Class của F3:03. Không lấy class qua byte cuối packet.
- Đợi inventory thật của server, dùng slot 2..6 để chọn giáp khi vào map.
- Cache thân đã kiểm tra bone/texture, tối đa 16 tổ hợp; cache key gồm class,
  chuyển chức, tên model, animation và metadata nâng cấp. Không đọc Data trong vòng render.
- Single click chỉ chọn đúng row; double-click hoặc Connect mới vào game.
  Cờ busy chặn click tiếp trong khi đang tải/kết nối.

Kiểm thử nạp 18 tổ hợp class/chuyển chức có resource, tái sử dụng cache,
đối chiếu model selection/world, kiểm tra slot giáp server và mapping đặc biệt.

## Giới hạn còn lại

- Player.bmd có 284 actions; chưa xác định chắc action đứng RF tương ứng với
  cấu hình engine EX603 này. RF dùng thân Class07 đúng, tạm idle nam chung;
  cảnh báo này được giữ lại, không tuyên bố RF animation hoàn chỉnh.
- Action theo từng vũ khí, các effect Excellent riêng từng model,
  skin texture biến thể và cloth physics chưa port đầy đủ.
- Thay giáp online đã cập nhật mesh ngay; xem mục 0.28.0 bên dưới.
- Nhân vật người chơi khác trong viewport vẫn chỉ hỗ trợ DW không trang bị.
  Catalog DW được nạp riêng để không dùng nhầm thân của class đang chơi.
- Không thay đổi hoặc copy tài nguyên trong Data gốc.

## 0.26.1 — thao tác chọn và tốc độ

Khôi phục một click chỉ highlight; double-click/Connect mới gửi chọn nhân vật.
Giữ nguyên bộ resource dùng chung và giáp đọc từ server.
CharacterMoveSpeed trong code tham chiếu là 12/15 đơn vị mỗi tick 40 ms,
TERRAIN_SCALE=100: tương ứng 3/3.75 ô mỗi giây. Không đổi tốc độ theo FPS.
Render contract kiểm tra cùng quãng đường sau một giây ở 25 và 60 frame.
Nhịp animation/camera và executable Windows thực tế vẫn cần đối chiếu thêm
trước khi kết luận cảm giác di chuyển đã trùng bản gốc.

## 0.26.2 — hiệu chỉnh cảm giác theo phản hồi “cả hai đều nhanh”

`native/config/client.ini` có `MovementRate=0.90`: giảm 10% tốc độ thân,
nhịp animation đi/chạy và lịch gửi bước di chuyển. Tốc độ gốc vẫn được giữ
trong MotionTrack; đặt 1.00 để so sánh lại với mức tham chiếu.
Đây là mức thử theo cảm giác người dùng, không phải số liệu đã xác nhận từ
executable Windows tương ứng. Không đổi 60 FPS, thời gian quay hướng,
idle animation, chính sách chống hack hoặc cấu hình GameServer.
Tốc độ thử: đi 2.7 ô/s, chạy 3.375 ô/s. Rate được giữ khi server sửa vị trí.
Kiểm thử rate 1.00 và 0.90 cho cùng quãng đường ở render 25/60 FPS;
profile từ chối số không hợp lệ/ngoài phạm vi 0.5..1.25.
Nguồn cấu hình được CMake copy vào app: sửa native/config/client.ini rồi build
lại, hoặc sửa Contents/Resources/client.ini của app hiện tại và khởi động lại.

## 0.28.0 — đồng bộ giáp khi đang chơi

Inventory authoritative từ server, slot 2..6, là nguồn chọn model. Khi revision
thay đổi, resolve resource trên network queue, nạp body qua cache có giới hạn.
Không gửi giả thông tin trang bị, không sửa Data hay cấu trúc thư mục gốc.

World viewer kiểm tra tên/parent của toàn bộ skeleton trước khi nhận body mới.
GPU prepare hoàn thành mới trao đổi mesh ở đầu frame. Generation token bỏ kết
quả cũ sau yêu cầu mới hoặc đóng scene; giữ nguyên actor, skeleton, animation,
camera và map. Mức chân lấy từ mesh boots hiện tại. Lỗi nạp body được ghi vào
log, giữ body hợp lệ trước đó và không đóng kết nối mạng.

`item_upgrade.mm` dùng chung cho item preview và giáp đang mặc: texture
Effect/Chrome01, Shiny01, Chrome02, các pass cộng màu theo mức nâng cấp đang
được hỗ trợ. Overlay dùng cùng skinner với giáp, không chạy skinning CPU riêng.
Đây chưa phải toàn bộ particle/custom effect của mọi item trong engine gốc.

Kiểm thử `mu_client_live_equipment_contract`: mặc áo Elf +15 rồi tháo; kiểm tra
revision, GPU preparation, tối thiểu 3 overlay có skinner, scene/actor/camera
không đổi. Render screenshot được lưu trong thư mục build. Resource contract
kiểm tra cache tách mức cộng và metadata +15 được giữ tới renderer.

## 0.28.1 — click nhấc/đặt từ ô trang bị

`item_placement` giữ chung tọa độ 12 ô cho renderer Inventory và bộ nhận click.
Bộ chọn item nhận slot 0..11; dấu vùng đặt dùng kích thước ô trang bị thay vì
công thức lưới túi. Nhấc item chỉ ẩn ô nguồn tại client; click đặt mới gửi C3:24.
Gói tin hỗ trợ equipment/main bag 0..75 và warehouse 0..119. Server vẫn kiểm
tra class, chỉ số, level và quy tắc map. Client kiểm tra slot theo wearSlot của
Item BMD, cặp tay/nhẫn và xung đột vũ khí hai tay theo CheckItemMoveToInventory.
Ô đích đang có đồ cần tháo trống trước; chưa hỗ trợ đổi chỗ hai món trong một click.

Esc/chuột phải trả item về chỗ cũ. Chỉ ACK thành công mới đổi snapshot và revision.
Publish đồng bộ lại bootsLevel cả khi inventory đổi qua ACK chuyển đồ, để tháo/
mặc giày cập nhật RunPolicy. Không thêm công việc vào vòng render 60 FPS.

Contract: packet mặc/tháo, giữ options, không nhận slot ngoài phạm vi; click nhấc
boots slot 6, từ chối đặt vào helm, đặt xuống túi slot 57; nhấc staff slot 14 và
đặt vào tay slot 0. Kiểm tra occupancy, ring/hand slot và vũ khí hai tay.
`MuClient-equipment-test` dành riêng account test, tự tìm fixture, di chuyển hai
chiều và relog kiểm tra khôi phục. Không thay chỉ số nhân vật để ép mặc item.

## 0.28.2 — model đang cầm/đeo và đồng bộ sau ACK

`CharacterResources.attachments` lấy slot 0/1/7 từ inventory authoritative,
cache key chứa index/mức cộng/Excellent/set. `item_models.cpp` là mapping model
chung cho icon và trang bị trên người, đối chiếu OpenData. Không lấy model từ
ảnh icon hoặc tự dựng hình thay thế.

World dựng rig riêng bằng BMD của vũ khí/khiên/cánh. Vũ khí gắn bone 33/42,
vào safe zone gắn bone 47; vị trí/góc theo RenderLinkObject và nhánh
CSK_REF_BACK_RENDERITEM trong engine tham chiếu. Cánh thông thường lấy vị trí
bone 47 với offset (0,0,15), animate action 0 ở .25 key/tick 25Hz. DarkLordRobe02
gắn bone 19 theo nhánh riêng. Không dùng inverse bind của Player cho xương cánh.
Weapon tĩnh không cập nhật xương lặp lại mỗi frame. Effect Chrome/Shiny dùng
chung `item_upgrade` và skinner của model trang bị.

ACK chuyển đồ gọi `changedEquipment` ngay trên network queue, sau đó GPU prepare
và thay nodes ở đầu frame. Pump cũng kiểm tra equipment key riêng, không phụ
thuộc revision của icon. Tháo xóa cả rig và effect cũ; không đổi camera/map.

Test resource và render kiểm tra staff +15, shield +9, wing, giáp; tháo không
còn node, gắn đúng hand/back theo ATT; scene/actor/camera giữ nguyên khi thay đồ.
Ảnh kiểm thử `native/build/client/live-equipment.png.equipped.png` hiển thị
fixture render, không phải bằng chứng server đã cho một nhân vật thật mặc đồ.

Giới hạn: CharSet-only lobby chưa dựng weapon/wing; action thân theo vũ khí,
cloth/cape physics, mount/pet, glow/particle riêng của mỗi item chưa đầy đủ.
Source engine tham chiếu không được xác nhận trùng tuyệt đối engine Windows
của bộ DLL EX603 đi kèm. Model/texture thiếu được ghi stderr; không thay Data.

### Ranh giới safe zone / cầm tay

Rà lại RenderCharacter (vòng Weapon), RenderLinkObject và BMD::RotationPosition:
hand dùng Link=false, RotationPosition chép trực tiếp matrix bàn tay vào
ParentMatrix; không áp matrix offset/góc dành cho Link=true trên lưng. Do đó
cầm tay dùng transform identity dưới bone 33/42; safe zone mới dùng grip back.
Cánh Link=false cũng giữ orientation của bone gốc, không chỉ lấy tọa độ.

`equipped_actions` chọn action đứng/đi/chạy từ TwoHand của Item BMD và tay đang
cầm: sword/dual sword/two hand/spear/scythe/staff/wand/bow/crossbow. Nhánh kiếm
hai tay đặc biệt dùng 142/143/144, chỉ số được xác minh bằng cách biên dịch enum
tham chiếu. Ngoài safe zone dùng clip trang bị; trong safe zone dùng clip thân
thường. Chuyển bộ clip blend từ pose hiện tại, không reset map, không đọc BMD
trong render. Thay trang bị chuẩn bị sample quaternion trước khi GPU swap.

Contract kiểm tra outside-safe-zone hand transform identity, parent hand đúng,
action đúng theo trang bị và safe zone; có snapshot `.combat.png`. Chưa bao phủ
mọi action mount, flight, RF/weapon riêng hoặc engine Windows tương ứng.

## 0.28.3 — quy tắc tay và chuyển tay trực tiếp

CNewUIMyInventory::IsEquipable có kiểm tra slot chặt hơn ItemManager của server:
cung dài (wearSlot 1) không đặt sang tay phải; nỏ (wearSlot 0) không đặt sang tay
trái của Elf. Native UI truyền base class vào kiểm tra vị trí: exception vũ khí
wearSlot 0 sang tay trái chỉ cho DK/MG/RF một tay, hoặc Summoner ngoài staff;
nhẫn dùng hai ô. Class/stat/level vẫn được server xác nhận.

Client nhấc từ slot 0/1 và click ô tay trống tương ứng gửi trực tiếp C3:24,
không gửi bước trung gian xuống túi. Contract UI kiểm tra cả hai chiều với kiếm
một tay và chặn cung Elf sai tay. Quy tắc hai tay vẫn xét món thực sự khác đang
chiếm tay đối diện.

CheckItemMoveToInventory (nhánh MU_NATIVE_MACOS) bỏ qua chính địa chỉ CItem
nguồn khi kiểm tra tay đối diện; tránh coi món đang chuyển tay là một vật cản
hai tay thứ hai. Không bỏ kiểm tra class/chỉ số, item khác hoặc đích đang có đồ.
Bản Windows giữ nguyên nhánh cũ. Muốn áp dụng sửa này phải build/restart GameServer.

## 0.28.4 — vị trí đeo cung trong safe zone

Ảnh người dùng tại Noria (172,114) có ATT=1: lỗi ở pose đeo sau lưng, không
phải ô Inventory. Không áp correction trái (145,0,275; 0,10,-30) cho nhóm Bow/
Arrows. Nhánh RenderLinkObject cũ không có correction này; giữ grip cung
(70,0,90; -10,5,10). Nhánh correction mới làm cung lệch ngang hông với Data này.
Không đổi slot nhận từ server, không mirror model hay đổi toàn bộ xương tay.

`mu_client_elf_bow_contract` dựng cung trái + bao tên phải ở đúng (172,114),
kiểm tra offset đeo cung, sau đó chuyển ra ATT không-safe để kiểm tra bone tay,
transform identity và action 8. Mặc/tháo phải gỡ model cũ. Snapshot cận cảnh chỉ
thay FOV của camera kiểm thử, không đổi camera game thực tế.

## 0.28.7 — hiệu ứng giáp ở màn chọn nhân vật

DGCharacterListRecv trong source EX603 đóng gói 7 mức cộng vào CharSet[6..8],
3 bit mỗi ô trang bị 0..6. Giải mã LevelConvert 0/3/5/7/9/11/13/15; giáp
slot 2..6 lấy từ bit 6/9/12/15/18, Excellent và Ancient lấy từ byte 10/11.
Chỉ dựng metadata này nếu chưa có inventory thật; packet inventory vẫn ưu tiên.
CharSet rút gọn không phân biệt +8/+9, +10/+11, v.v. Đây là giới hạn wire gốc,
không giả lập yêu cầu nhận toàn bộ inventory ở màn chọn nhân vật.

load_characters giữ CharacterMesh.equipment và Data effectDirectory khi chuyển
sang ActorVisual; MuCharacterRig gọi cùng apply_item_upgrade với world renderer.
Không tự nhân bản shader hay chỉnh palette riêng cho lobby. Chọn nhân vật dim
bằng opacity của root, không ghi đè material.multiply của Chrome/Metal passes.

Contract dựng giáp Legendary các grade 0,3,5,7,9,11,13,15 từ CharSet, kiểm tra
metadata qua body/lobby và kiểm tra additive passes trong GPU geometry. Không
thay đổi database hoặc trang bị của người dùng. Weapon/wing preview từ CharSet
vẫn là phần chưa hoàn thiện; sửa này bao phủ giáp 5 món.

### 0.29.2 — vũ khí màn chọn nhân vật

Hai tay giải mã từ CharSet[1,2] và nibble cao [12,13], sentinel0xFFF; level nén từ [6..8], excellent/ancient từ bit4/2 của [10,11]. Inventory authoritative vẫn ưu tiên khi vào game. Lobby gắn model từ CharacterBody vào cùng bone tay33/42 và dùng chung equipment_grip.hpp với world; stance dùng equipped_actions. Level nén chỉ thể hiện các grade0/3/5/7/9/11/13/15 theo giao thức gốc. Wing CharSet-only chưa port đầy đủ.
