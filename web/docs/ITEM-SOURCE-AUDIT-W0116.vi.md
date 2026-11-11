# W0116 — Rà item/tooltip/giá theo source PC

Ngày 2026-10-31. Phạm vi: các policy thuần logic của client web so với `Main5.2/source` (source PC được ghi trong comment của web) và phần giá lấy theo `GameServer/Item.cpp` mà web đang mô phỏng. Không thay đổi model/texture/animation.

## Đã đối chiếu và giữ nguyên

Các mục sau đã kiểm tra là khớp source, không sửa:

- `goldColor` ↔ `ZzzInventory.cpp:1341` — bốn ngưỡng 10.000.000 / 1.000.000 / 100.000 và RGB tương ứng.
- `requirementAttributeIndex`, `itemRequirements` ↔ `ZzzInfomation.cpp` — thứ tự STR/DEX/Energy/Vit/Cha/Level.
- Điểm cộng/trừ yêu cầu và cách cộng `option*4` vào Strength.
- Bảng màu tên item: xanh lá `#19ff80`, vàng `#ffcc19`, xanh dương `#80b3ff`, trắng — đúng `ZzzInventory.cpp:216–229`.
- `noticePolicy` (Welcome vào system chat, branding thay tên) không có nhánh source mâu thuẫn.

## Đã sửa

### 1. Độ bền tối đa — `item-visual-policy.js::maximumDurability`

Đối chiếu `ZzzInventory.cpp:1796 calcMaxDurability`:

- Vòng cộng cấp dùng đúng ngữ nghĩa `WORD`: từ cấp 13 (+7) và cấp 14+ (+8) **cap 255 trong lúc cộng**, sau đó mới cộng thưởng ancient/excellent và **không cap lại**.
- Thêm ngoại lệ `ITEM_STAFF + 36` (`5*512+36`) khỏi thưởng +15 excellent — source có trong danh sách loại trừ.
- Thêm `ITEM_HELPER + 4` / `+5` → 255 trước khi cộng thưởng (đúng thứ tự source, nên kết quả có thể vượt 255).
- Thêm lucky item: group 7..12, index 62..72 → 255, áp **sau cùng** (`Check_LuckyItem`, `ZzzInventory.cpp:7662`).
- Bỏ `Math.min(value,255)` cuối hàm vì source trả `WORD` không kẹp sau thưởng.

### 2. Màu ô item — `itemBackground`

Đối chiếu `NewUIInventoryCtrl.cpp:250 CanChangeItemColorState` và danh sách item có độ bền:

- Thêm helper `106` và `109..115` vào nhóm hiển thị màu độ bền.
- Giữ helper `30` (vì `IsWingItem` coi `ITEM_HELPER+30` là cánh).
- Giữ cách xử lý riêng cho `HELPER+20` cấp 1/2 (source có bug ưu tiên `||`; web giữ hành vi đã thu hẹp có chủ đích).

### 3. Chỉ số trang bị — `item-visual-policy.js::equipmentItemStats`

Đối chiếu `ZzzInfomation.cpp:525 ItemConvert`:

- Ancient được nhận diện bằng `setOption & 3 ∈ {1,2}`.
- Ancient **buộc** `is_excellent = 1` như source, nên yêu cầu/chỉ số dùng nhánh excellent (+25 item level). `+30` chỉ dùng cho các bonus ancient (`setItemDropLevel`), tách thành biến `dropLevel`.
- Yêu cầu cấp ≥220 dùng công thức phẳng của source: Energy `n*2 + n*0.035*level` (excellent) / `n + n*0.035*level`; STR/DEX/VIT `n + n*0.75 + n*0.02*level` (excellent) / `n + n*0.02*level`. **Charisma không đi nhánh cấp cao** — đã sửa.
- `HELPER+5`: Charisma cố định `185 + req*15`.
- Damage: bonus excellent `25*DamageMin/baseLevel + 5` (hoặc `excelAddValue` của Mace+6/Bow+6/Staff+7) và bonus ancient `dropLevel/40 + 5` được cộng **độc lập**, không loại trừ nhau.
- Defense: ancient dùng `3*Defense/dropLevel + dropLevel/30 + 2`; khiên `20*Defense/dropLevel + 2`.
- SuccessfulBlocking: **bỏ** bonus ancient (source không có).
- MagicPower: ancient dùng `dropLevel/60 + 5` (trước là `effective/60 + 2`).

### 4. Giá NPC — `npc-price.js`

Đối chiếu `GameServer/Item.cpp:916 CItem::Value`, `ItemValue.cpp:74` và `ItemManager.cpp:417`:

- Giá theo `ItemValue.txt`: chỉ nhân durability khi item thực sự stackable (`MaxStack != 0`), không chỉ vì có dòng trong `ItemStack.txt`.
- Phạt độ bền khi bán chỉ áp cho item tính bằng công thức chính và có `wearSlot` 0..11; trước đây áp cả cho potion/jewel (`wearSlot = -1`).
- Bổ sung nhánh công thức cho group 12/13/15 đúng source:
  - Wing cấp 1 và các dải 36..43 / 50 / 262..265: `(40+ItemLevel)*ItemLevel²*11 + 40.000.000`.
  - Các wing còn lại và helper/pet: `ItemLevel³ + 100`.
  - Group ≤11 giữ công thức thường; hệ số 2 tay, skill, luck, option như cũ.
- Excellent: +100% mỗi bit cho group <12, +25% mỗi bit cho wing/helper (source phân biệt `m_Index < GET_ITEM(12,0)`).
- `+25 ItemLevel` cho excellent chỉ áp group <12 (source kiểm tra `m_Index < GET_ITEM(12,0)`).

### 5. Các điểm khác phát hiện khi rà lại

- `maximumDurability`: danh sách loại trừ +15 excellent của source là wing 36..40, 42..43 và 49..50; **wing 41 vẫn được +15**. Bản trước loại cả 36..43 nên wing 41 bị thiếu.
- `equipmentItemStats`: option cộng vào Strength là `5 * Option3` (`ZzzInfomation.cpp:1322,1332,1338,1344`), không phải `4 * Option3`; bỏ điều kiện chỉ cộng khi Strength gốc > 0.
- `tooltipNameColor`: nhánh `IsEnableOptionWing` của source được xét **trước** ancient/excellent — wing cấp <7 giữ màu thường, cấp ≥7 luôn vàng. Ancient không phải wing dùng chữ trắng trên nền xanh `(60,60,200)`; đã thêm nền này ở tooltip.
- `item-class-policy`: tên class bậc 2 của Magic Gladiator/Dark Lord dùng text 2305 như source (trước đó bị bỏ dòng).
- `equipmentSmoke`: đủ 5 mảnh gồm cả boots (slot 6); test bổ sung trường hợp thiếu boots.

### 6. Tooltip/chỉ số bổ sung

- `equipmentItemStats`: tách yêu cầu Energy đúng `ItemConvert` — BOOK (`group 5, type 21..23`) dùng `3*(Level+item_level)`, skill scroll/parchment (`group 15`, RequireLevel>0) dùng `4*RequireLevel`, còn lại `4*(3*Level+item_level)`.
- Cepter: loại Frost Mace `2:16`, thêm `2:17..18` theo tên/kind thực tế trong Item table.
- Áp danh sách `is_excellent` bị `ItemConvert` xoá cho Archangel/wing đặc biệt (`SWORD+19`, `BOW+18`, `STAFF+10`, `MACE+13`, `HELPER+30`, `STAFF+36`, wing 3..6, 36..40, 42..43, 49..50, 130..135); ancient vẫn bật lại như source.
- Set option: đã đối chiếu `calcSetOptionList` — với dữ liệu set hiện tại (≤11 món) nhánh `byExtOption` của source là dead code, nên policy hiện tại (standard + full khi `count>=byOptionCount-2`) là khớp hành vi thực tế.

### 7. Tên/màu đồ rơi

- Port `CreateDropItemName` phần màu và bold: trắng/xám/cam/xanh dương/vàng/xanh lá/tím/violet theo level, skill/luck/option, excellent/ancient, jewel, wing/socket range.
- `ground-layer.js` dùng `groundItemVisual` thay vì màu tooltip; sửa luôn lỗi đồ rơi đang dùng `#19ff80` của tooltip thay vì `#19c81c` của `HookColorFont`.
- Mở rộng hậu tố `+level/+Skill/+Option/+Luck` cho ring/pendant group 13 (8–9, 12–13, 21–28) đúng nhánh default của source; giữ helper10/11 theo nhánh đặc biệt.
- Tên đặc biệt (pet/event/quest/jewel tiers) vẫn dùng tên catalog + hậu tố; chưa port toàn bộ bảng `GlobalText` trong `CreateDropItemName`.

### 9. ItemTRSData.bmd

- Đã giải mã `Data/Local/ItemTRSData.bmd` (4.362 record, `ITEM_POSITION` 32 byte, BuxConvert `[FC,CF,AB]`, checksum cuối file).
- Sinh `client/src/ui/item-trs-data.js` (3.694 index duy nhất, giữ bản ghi đầu như `std::map::insert`) bằng `tools/generate_item_trs.mjs`.
- `inventoryTRSScale(index,height)` áp kích thước icon theo đúng công thức source: `TRS.scale * 2 * 137.51 / (height * 20)`.
  - `137.51` = `1/(tan(0.5°)*2/WindowHeight * 0.1*WindowHeight/480*2000)` — số pixel/world unit của `RenderItem3D` (FOV 1°, far 2000, object ở `ScaleMA(0.10)`), độc lập độ phân giải.
  - Canvas icon cao `height*64` nên hệ số rút gọn còn `TRS.scale * 13.751 / height`.
- Thứ tự ưu tiên scale trong `item-icons`: `inventoryFixedScale` (orb/jewel/bundle — đã đối chiếu client gốc) → `inventoryTRSScale` → auto-fit. Orb/jewel dùng TRS .002 sẽ nhỏ hơn bản gốc nên giữ scale cố định; item thường dùng công thức TRS.
- **Ring/pendant group 13** không có override `ItemScale` trong source nên dùng TRS `.0025` (vd. Ring of Ice → NDC ~0.76 ô), không dùng scale jewellery cũ.
- Góc/anchor TRS **chưa** áp vào renderer web: model origin/camera của web khác `RenderObjectScreen`, áp trực tiếp từng làm item lệch khỏi ô. Giữ góc/anchor đã calibrate; scale thì đã theo source.

### 10. Override `RenderItem3D`

- Đã port **scale + angle + pos** cho 21 nhánh đặc biệt của `ZzzInventory.cpp:7975–8263`:
  - Helper+19 level 0/1/2, Helper+20 level 0–3.
  - Potion+9/12/21/23/24 level đặc biệt.
  - Potion+11 (Rena) level 1/2/3/5/6/8–15.
  - Helper+14 level 1.
- Đã port **model path** cho các nhánh thay model: Helper+19 (Sword20/Bow19/Staff11), Helper+20 (RingOfLordEvent/MagicRing), Helper+14 (DarkLordSleeve), Potion+11 (MagicBox02/03/05/06/07/08/Beer02), Potion+12, Potion+21 (EventBloodCastle03), Potion+23/24 (QuestItem3rd), Potion+32–34 (p03box/obox02/blue01).
- `pos[]` được đổi sang anchor NDC và chỉ áp cho item có override; item thường vẫn center theo bounding box.
- **Chưa port**: `BodyHeight` (dịch dọc helm/armor/pants/gloves/boots, lucky item).

### 11. Test hồi quy

- `tests/item-visuals.test.mjs`: thêm test cho staff+36, helper 4/5, lucky 255, thưởng không cap, helper 106/109..115.
- `tests/npc-shop.test.mjs`: thêm test phạt độ bền chỉ cho trang bị, công thức wing/helper và giá potion.

## Kiểm chứng

- `node --test tests/item-visuals.test.mjs tests/appearance.test.mjs tests/game-ui-state.test.mjs` → 31 pass.
- `node --test tests/npc-shop.test.mjs` → 14 pass.
- Toàn bộ `npm test`: **321/321 pass**. Test build allowlist được chỉnh để bỏ qua các thư mục resource được `tools/build.mjs` cố ý giữ lại (`runtime`, `preview-assets`, `audio-assets`, `ui-assets`, `map-title-assets`) — trước đây test coi chúng là file lạ nên fail giả.
- Test mới: giải mã TRS thật, anchor/góc cho index 0/6144, fallback cho index không có TRS, và tái sinh artifact khớp byte-for-byte.

## Giới hạn

- `BodyHeight` theo nhóm trang bị chưa áp; vị trí dọc của helm/armor/pants/gloves/boots và lucky item có thể lệch nhẹ.
- Chưa mô phỏng các nhánh custom: pentagram, socket item, 380 item, custom jewel/wing, Muun/pet value, period/expired item trong `CItem::Value` và `IsHighValueItem`.
- `npcPrice` chưa cộng `Is380Item +16%`, socket bonus, pentagram value; các nhánh này cần metadata client chưa có.
- `equipmentItemStats` đã tách BOOK/SKILL_SCROLL; phần còn lại chỉ là danh sách loại trừ `is_excellent` đã áp, nhưng chưa có `Kind2` thật nên Cepter vẫn dựa trên dải type.
- Set option đã đối chiếu `calcSetOptionList`; policy hiện tại khớp hành vi thực tế với bộ dữ liệu set hiện có, nhưng chưa mô phỏng cơ chế `m_bySameSetItem`/2 bảng option A-B của native.
- `groundItemName` đã port màu/bold của `CreateDropItemName` (white/gray/orange/blue/yellow/green/violet/purple); phần **tên đặc biệt** cho jewel/pet/event/quest vẫn chưa port đầy đủ.
- Các hiệu ứng ánh sáng/smoke/flare mới ở mức xấp xỉ màu và nhịp, chưa đủ mọi nhánh particle/joint theo cấp và loại set.
- Chưa port đầy đủ `NewUIItemEnduranceInfo` (màu/độ bền từng ô), repair gold, personal shop và các tooltip trạng thái phụ thuộc hệ thống chưa có trong web.
- Chưa chứng nhận pixel/visual parity; tài liệu này chỉ chốt phần logic đã đối chiếu.
