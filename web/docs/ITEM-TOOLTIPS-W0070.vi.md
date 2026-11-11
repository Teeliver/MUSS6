# W0070 — tooltip và kiểm tra kích thước vũ khí

## Yêu cầu

Bỏ viền vàng khi chỉ vào item, đối chiếu tooltip với source. Người dùng cũng báo vũ khí đang cầm quá nhỏ; đã có hai mô tả vị trí (ô trang bị rồi trên tay nhân vật), cần giữ cả hai trường hợp khi xác minh.

## Source đã đọc

Main5.2/source/ZzzInventory.cpp: RenderItemInfo, RenderTipTextList, HookColorOldFont; NewUINameWindow.cpp: RenderTooltip_old/RenderTooltip. Nhánh cổ điển dùng nền đen alpha .8, mép đen 1px; đo chiều rộng theo dòng, cộng 12px; căn giữa con trỏ, chặn ra ngoài màn hình; giãn dòng 1.1. Nhánh mới dùng IMAGE_HUD_PLAYER qua jpexs, phụ thuộc shutdown_tooltipold; chưa xác minh config runtime nên triển khai nhánh cổ điển có nguồn rõ ràng.

NewUIItemTooltip.cpp/.h còn đọc ItemToolTip, ItemTooltipText, ItemLevelTooltip. Bộ Data hiện có bản _eng/_por/_spn; chưa tích hợp toàn bộ các bảng custom này. Không coi tooltip mọi loại item đã đạt parity.

## Thay đổi

- Xóa viền vàng hover/selected/focus của item, không xóa nền báo độ bền. Xóa title HTML trên nút item để tránh tooltip trình duyệt chồng tooltip MU.
- Tooltip nền/mép đen, font theo scale HUD, căn giữa; tên/mức cộng và từng dòng có màu. Requirement thiếu hiện đỏ. Đọc chuỗi GlobalText từ Data hiện có cho các dòng đã hỗ trợ.
- Thay base damage/defense bằng phép tính CalculateItem level/excellent/ancient và requirement số nguyên theo native/client/item_stats.cpp; thêm durability hiện tại/tối đa, tốc độ, defense rate và wizardry. Không hiển thị excellent bitmask dưới dạng option cho người chơi.
- Icon staff sửa bằng khôi phục góc xoay của bản trước khi phát sinh lỗi; giữ fit có chiều sâu để không cắt khi hover. Renderer vẫn auto-fit, chưa thay thế ItemTRSData và camera nguyên bản.

## Chốt lỗi theo hai marker của người dùng

Marker chỉ đúng nút Legendary Staff +9 trong ô trang bị và Bone Helm +2 trong túi. Mô tả “trên tay” trước đó đã bị hiểu nhầm thành model ngoài map; không cần thay hệ số vũ khí ngoài map cho lỗi đã đánh dấu.

- Staff06 (Item 5:5) có bounds sau góc cũ 180/270/15 là khoảng 68.94×254.57×18.19. Áp fallback Main5.2 270/-10/0 cho bộ Data thiếu ItemTRSData làm bounds thành 47.99×49.87×249.71: chiều dài nằm theo Z, icon rất nhỏ khi fit. Khôi phục góc staff trước W0069 theo native/client/item_icons.mm và bộ Data đang dùng; vẫn giữ default source Main5.2 trong helper riêng, không tuyên bố bảng TRS đã có. Giữ fit có Z để hover không bị cắt vì thay đổi góc nhìn.
- Bone Helm (Item 7:4), Player/HelmMale05.bmd có head_06.jpg (180 vertex), head_a06.tga (18), hide.jpg (21) và skin_warrior_01.jpg (117). RenderObjectScreen gọi RenderPartObject với HideSkin=true; BMD::RenderMesh bỏ mesh skin/hair và CGMModelManager::OpenTexture ánh xạ theo tiền tố. Bản web nay lọc các mesh đó trước tải texture, tính bounds và tạo lớp upgrade; giữ head_06.jpg/head_a06.tga là texture nón thực. Chỉ áp dụng trong item icon, không xóa da của nhân vật ngoài map.

## Kiểm tra

162/162 test client qua sau sửa góc staff/HideSkin và cập nhật contract build có content revision. Debug 82 / release 31 file, staging 179 asset debug; source Data không chỉnh sửa. Browser bản cuối đã xác minh, chi tiết cuối tài liệu.


## Xác minh bổ sung

160/160 test client qua, debug 82 / release 31 file build thành công. Phát hiện trình duyệt giữ ES module cũ; build nay thêm content revision đồng bộ cho các relative JS imports và entry/CSS trong HTML. Browser đã xác nhận entry có revision và tooltip mới (không còn dòng “Phòng thủ cơ bản”, Luck thô). Kiểm tra tooltip staff +9 ngoại tuyến có damage 50–52, wizardry +54%, durability 100/74 và requirement theo dữ liệu; nền đen không viền vàng. Sửa escape %% của chuỗi C khi đưa sang DOM sau quan sát. Fixture loadout=legendary dùng đúng Item 5:5 (Staff06); 5:3 là Thunder Staff trong Data này. Ảnh /tmp/mu-w70-tooltip-offline.png chụp fixture Thunder trước khi đổi fixture sang Legendary; không gán ảnh này cho nhân vật online.

Đã thêm test với Staff06 và HelmMale05 thật qua WASM: trục dài staff sau chiếu >240, cao >3×rộng; mesh còn lại của Bone Helm là head_06.jpg và head_a06.tga. Fixture ngoại tuyến có Legendary Staff +9 và Bone Helm +2 để kiểm tra trực tiếp.

Staff có kích thước grid 1×4 (20×80), cao hơn ô trang bị 46×66. Bỏ overflow:hidden của nút item để canvas không bị cắt theo khung ô, tương ứng RenderItem3D không dùng scissor từng ô. Hotspot vẫn giữ hình chữ nhật ô trang bị.

Đối chiếu Item_eng xác nhận Bone Helm là 7:4 (HelmMale05), không phải 7:1 (Dragon Helm/HelmMale02). Fixture và test asset thật đã sửa đúng Bone Helm trước kiểm tra cuối.

Bản cuối: backup W0070-20261026-214454; 162 test suite qua trước chỉnh fixture từ Dragon sang Bone đúng tên, sau đó 9/9 item visual test qua với HelmMale05 thật. Chỉnh CSS không cắt ô đã đóng gói lại.

## Browser cuối

Cảnh ui-check OFFLINE loadout=legendary đã nạp đúng Legendary Staff +9 và Bone Helm +2. Quan sát staff đủ chiều dài trong ô trang bị, Bone Helm không còn mesh mặt/cổ, không còn viền hover vàng. Tooltip staff hiển thị damage 56–58, wizardry +61%, độ bền 100/80, yêu cầu strength 149 tô đỏ (fixture thiếu điểm), Luck +25%/+5% không còn ký tự % kép. Ảnh /tmp/mu-w70-staff-bone-fixed.png và /tmp/mu-w70-tooltip-final.png. Không dùng fixture để khẳng định giao dịch online; không thay trang bị thật khi test.
