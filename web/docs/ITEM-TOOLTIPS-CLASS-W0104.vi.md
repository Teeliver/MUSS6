# W0103–W0104 — Tooltip, class, số lượng, phép chiếu Orb/Life

Ngày 30/10/2026. Tiếp theo W0102. Người dùng xác nhận Orb vẫn quá to, Life lệch lên trên, muốn số lượng đậm nhẹ và class không dùng được có nền đỏ.

## Dùng dữ liệu gốc

- `Data/Local/Eng/ItemToolTip_eng.bmd`: 973 record có tên; struct 124 byte.
- `ItemTooltipText_eng.bmd`: 415 dòng text có nội dung; struct 260 byte.
- `ItemLevelTooltip_eng.bmd`: 105 record level; struct 102 byte.
- Tool `generate_item_tooltips.mjs` kiểm tra checksum 0xE2F1, XOR theo từng record và giữ SHA-256 nguồn. Artifact `item-tooltip-data.js` được tạo từ các file này; không viết công dụng item theo trí nhớ.
- `itemDescriptionLines` chọn record thường/record theo level, resolve nội dung và các giá trị đã hỗ trợ từ NewUIItemTooltip.cpp. Bao gồm công dụng gem, bundle “Combines N … / Available after dismantling”, số lượng potion, điều kiện level/stat và số còn thiếu. Dấu `#` được tách dòng theo SplitText.
- [Kiểm kê model/texture collectibles](ITEM-COLLECTIBLES-W0104.json): 32 mã Orb/ngọc/ngọc nén, model và mọi texture đều đọc trực tiếp từ `/Users/angelale/Downloads/MuClientEX603/Data`, ghi SHA-256. Ví dụ Orb 12/7–19 (trừ Chaos 12/15) → Gem01–Gem13 theo vòng lặp ZzzOpenData.cpp; Life/Life bundle → Jewel03.bmd. Không thay asset bằng ảnh vẽ hoặc sprite khác.

## Điều kiện class và mặc đồ

- Đọc mảng bảy class từ Item_eng.bmd; required=0 là không được dùng; 1/2/3 là cấp tiến hóa tối thiểu.
- Tooltip dùng đúng tên class/evolution từ GlobalText và `RequireClass`. Các mã bị `IsRequireClassRenderItem` loại trừ không tự thêm class; all-seven=1 không in một danh sách thừa.
- Class không tương thích hoặc chưa đủ tiến hóa: **chữ trắng, nền #A00000**. Màu lấy `SetBgColor(160,0,0,255)` trong ZzzInventory.cpp/HookColorFont, không chỉ đổi chữ sang đỏ.
- Cùng điều kiện class dùng trong canPlaceInventory; dữ liệu class không hợp lệ không được tự coi như phù hợp. GameServer vẫn quyết định cuối cùng qua ACK.
- Sửa nhầm thứ tự: Item table chứa STR, DEX, ENERGY, VITALITY, COMMAND, LEVEL; HUD chứa STR, DEX, VITALITY, ENERGY, COMMAND. So sánh và nhãn tooltip giờ quy đổi Energy/Vitality đúng vị trí; không đổi ABI/table gốc hoặc hàm thống kê C++ vốn dùng thứ tự table.
- Kiểm tra điều kiện tối thiểu cho trang bị nhóm 12/13; level yêu cầu của cánh cấp 1/2 tăng theo nâng cấp như CalculateItemRequireStat (4 hoặc 5 mỗi cấp), các nhánh fixed giữ level bảng.

## Số lượng và Life

- Giữ công thức W0102: durability cho đúng nhóm stack; bundle = (level đã decode+1)×10; không hiển thị độ bền armor/pet thành số lượng.
- Theo yêu cầu điều chỉnh đọc rõ: chữ Arial **600 9px**, màu #DEDEDE, bóng đen lệch 1px, không additive/chrome/glow. Đây là cách vẽ chữ web theo yêu cầu người dùng, không nói là atlas number nguyên vẹn.
- Life 14/16 và Life Bundle 12/136 dùng tâm bounding box sau pose/góc chiếu, anchor giữa ô. Khi xoay giữ tâm, tránh origin model làm nó lệch lên trên. Đây là căn giữa theo yêu cầu trực tiếp; chưa xác nhận override riêng trong ItemTRSData bị thiếu.

## Tìm ra lý do Orb vẫn lớn ở cửa sổ rộng

- Resource đúng chưa đủ: W0102 vẫn để panel CSS nhân kích thước model thêm lần nữa.
- `CGMFrame.h`: `ScaleMA(MA) = MA * GetWindowsY / 480.0`.
- `ZzzInventory.cpp::RenderObjectScreen`: depth ray dùng ScaleMA(.10), hoặc .07 khi nhấc đồ; `CreateScreenVector` dùng RENDER_ITEMVIEW_FAR=2000; UI projection FOV=1° trong NewUICommon.cpp.
- Vì depth tăng cùng chiều cao cửa sổ, model không được phóng thêm theo scale panel. Pipeline inventory mới dùng hệ số `1/panelScale` riêng cho skill Orb (12/7–19 trừ Chaos 12/15). Khi nhấc Orb, tỷ số depth .10/.07 được giữ. Sau khi người dùng báo ngọc bị nhỏ theo, đã giới hạn hiệu chỉnh cửa sổ cho Orb; ngọc/ngọc nén/trang sức giữ kích thước W0103, Life chỉ căn giữa. Không đổi tùy tiện scale riêng của Gem01/Gem02 để chiều theo cảm giác.
- Tách anchor khỏi scale/rotation trong shader: vị trí ô vẫn theo UI; kích thước model và điểm xoay không bị anchor kéo lệch. Resize đánh dấu icon vẽ lại. Phép chiếu world và các icon đang dùng autofit không bị đổi.
- Theo geometry Data: Bless khoảng 10.63×13.39 px, Soul 11.82×11.75, Life 9.77×14.32 ở tỷ lệ vật lý fallback. Những số này là kích thước CSS theo phép chiếu đang tái hiện, chưa phải chứng nhận pixel parity với một executable client cụ thể.

## Kiểm tra và giới hạn

- Test biên dịch `IsRequireClassRenderItem` từ source và so sánh cả 8192 mã; class/evolution, cánh yêu cầu level, class không xác định đều có test.
- Regenerate tooltip artifact từ Data cho kết quả byte-for-byte giống nhau; test mô tả Jewel of Bless, bundle 30, potion 43, Healing Orb yêu cầu Energy chứ không phải Vitality.
- Test ScaleMA: kích thước model sau panel .5/1/1.5/2/3 giữ nguyên; tỷ lệ picked đúng 10/7. Test Life và bundle có tâm/anchor giữa ô.
- Chưa port toàn bộ status động của tooltip: Harmony/socket, timer/map/party, giá trị item-add-option và một số hệ thống phụ thuộc dữ liệu khác. Status không hỗ trợ bị bỏ qua, không chèn số giả. Chưa hoàn thiện mọi dòng tooltip của cả 973 record.
- ItemTRSData.bmd vẫn chưa có: không chứng nhận tất cả tỷ lệ/góc item 100%. Lượt này sửa được lỗi phóng thêm theo UI dựa trên source, không thay thế bảng custom đang thiếu.
- Cập nhật các bảng Data tooltip cần chạy lại generator và build; artifact là dữ liệu build có hash, không phải một cache âm thầm giữ dữ liệu cũ.

## Chốt theo phản hồi hình mẫu

- Điều kiện class nằm sau yêu cầu chỉ số và trước các dòng skill/Luck/Option; Wizardry option cũng nằm sau class.
- Mỗi dòng class cách nhau .2em (~2.7 px ở viewport kiểm tra), line-height 1.2 để nền đỏ không nối thành một khối.
- Full suite 290/290 qua trước giới hạn sửa scale theo phản hồi; sau giới hạn chỉ Orb, 80 test liên quan qua. Build debug 140 file và release 38 file thành công; thay đổi spacing/thứ tự kiểm tra trên browser, không thêm test chỉ sao chép CSS.

## Bằng chứng browser cuối

- DOM trên fixture Bronze Gloves: Defense / Attack speed / Durability / Strength / Agility → ba dòng class → hai dòng Luck. Hai class không phù hợp có nền rgb(160,0,0), margin-top ~2.70px; class Dark Knight phù hợp không có nền đỏ.
- Các canvas Orb báo modelResource Gem01–Gem07/Gem13 đúng Data; không có iconMissing trong mẫu. Ngọc được trả về tỷ lệ W0103 sau phản hồi người dùng; chỉ Orb nhận hiệu chỉnh độ phóng cửa sổ. Life và Life Bundle được căn giữa.
- Ảnh bàn giao `/tmp/mu-w104-class-order-final.png`; không mua/bán hoặc thay item nhân vật thật để kiểm tra.
