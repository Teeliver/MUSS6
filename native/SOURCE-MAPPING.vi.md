# Quy ước port EX603

Giữ source Windows ở đúng folder hiện có; code và build macOS nằm riêng trong native/. Không đổi tên resource, đổi item/classindex hoặc dùng dữ liệu giả để bù file thiếu. Root runtime được chọn bằng config, còn đường dẫn trong Data giữ cấu trúc gốc.

| Chức năng gốc | Module native | Dữ liệu gốc |
|---|---|---|
| GameServer/ItemManager.h, ItemManager.cpp | client/inventory.hpp, inventory.cpp | ItemByteConvert 12byte; slot gốc236/warehouse240 |
| GlobalText.h | client/localization.hpp, localization.cpp | Data/Local/Eng/Text_eng.bmd (Language Eng/Spn/Por) |
| NewUIStorageInventory.cpp tham chiếu | client/npc_panel.hpp, npc_panel.mm | Data/Interface/newui_item_back*, money3, Bt_money*, Bt_lock* |
| NewUIMyInventory.cpp tham chiếu | client/inventory_panel.hpp, inventory_panel.mm | Data/Interface/newui_item_*, exit/repair/shop |
| Decode wrappers | client/asset_images.hpp, asset_images.mm | Giữ tên OZJ/OZT, đối chiếu JPG/TGA theo tên source |
| GameServer/Viewport | client/viewport.hpp, viewport.cpp | index/class/tọa độ server |
| MonsterSetBase.cpp | server gốc đã port | Data/Monster/Spawn/NNN - map.txt |

Nguồn engine tham chiếu: https://github.com/oXaa/muonline-client-sources . Main_EX603 trong bộ này là DLL mở rộng, chưa chứng minh engine tham chiếu trùng toàn bộ EX603. Các hạn chế được ghi rõ, không coi port đã chuẩn100%.

## Inventory 0.17

Shared item decoder giữ nguyên12bytewire, slot, index,level,durability,skill,luck,option,excellent,setOption,extended và các byte còn lại. Item DB16byte là định dạng khác, không đọc như packet. Danh sách F3:10 và modifyF3:14/delete28/dur2A/drop23 đã nối; snapshot chỉ copy khi revision đổi.

I hoặcV mở inventory cố định trong game. Bố cục nguồn tham chiếu190×429, 12ô trang bị, túi chính8×8 bắt đầu15,200; tên223 từ TextBMD, Zen từ world-entry/server. Còn thiếu icon3D, tên/tooltipItem_eng.bmd, ô mở rộng, một sốdelta pickup/buy/move, equiprenderer và chuyển đồ. Những byte/slot không hiển thị trong túi chính vẫn giữ nguyên trong state. Không gửi giao dịch trước khi nối kích thước item và xác nhận server.

Giao dịch kế tiếp dùng ItemManager24; container0=inventory,2=warehouse; server kiểm Interface/LoadWarehouse/WarehouseLock. Một request đang chờ, không sửa state trướcreply. Cần đối chiếu đầy đủ fail/timeout và lưuDB khi triển khai, hiện chưa bật.

## Inventory/Storage 0.18

ItemCatalog đọc đúng Item_{language}.bmd dưới Data/Local: 8192 record84byte, checksumGenerateCheckSum2 với keyE2F1, XOR từng recordFC-CF-AB. Nguồn ZzzInfomation.h/.cpp và _struct.h tham chiếu. Tên, footprint và wear slot từ catalog; dữ liệu runtime12byte vẫn từserver. Tooltip cơ bản tên+level,durability95,skill176,luck87/94 từTextBMD. Chưa port toàn bộ ItemTooltip scripts, stat tính theo level và set/socket.

NewUISystem.cpp source tham chiếu Show INTERFACE_STORAGE mở INTERFACE_INVENTORY; Hide STORAGE gọi ProcessClosing và ẩnInventory. Native0.18 đã mở cặp190pixel và đóng cặp, Warehouse bên tráiInventory. SceneView giữ kín khung game; dịch tâm projection theo số bảng để nhân vật nằm trong vùng còn lại, tránh vùng tối dưới bảng. HostGUI cố định trong game.

Icon item dựng từ BMD và texture gốc theo registry model trong item_icons.mm, cache theo index/level; không dựng lại mỗi frame. Model chưa có mapping giữ tên thật để nhận diện, không thay bằng model giả. Tooltip là view trong game, giới hạn theo host; hiện hỗ trợ tên/level, độ bền, skill và luck. Chưa có hiệu ứng excellent/set/socket và đầy đủ công thức tooltip. Kiểm tra GUI thực tế: Wizards Ring +1, Durability [30]; Inventory/Storage cùng mở, map phủ phần dưới, quan sát 60 FPS khi cửa sổ hoạt động. 5/5 kiểm thử client qua.

## Tooltip: chỉ số trang bị

`client/item_stats.hpp/.cpp` tách phép tính khỏi GUI. ItemCatalog đọc thêm level cơ bản, two-hand, sát thương, phòng thủ, magic defense, blocking, tốc độ, magic power và sáu yêu cầu từ record 84 byte. Nhóm 0..11 dùng nâng cấp +0..+15, nhánh shield, excellent và ancient trong GameServer/Item.cpp::Convert; magic-power phần trăm đối chiếu ZzzInfomation.cpp/ZzzInventory.cpp. Text được lấy theo ID gốc, thay placeholder bằng parser giới hạn, không dùng chuỗi Data làm printf format.

`item_ui_text.mm` nối dòng chỉ số, yêu cầu, option thường và sáu bit excellent theo ZzzInfomation/GetSpecialOptionText. Các nhánh cánh/pet/event, tên set + bonus ancient, SocketItem, Harmony và so sánh thiếu stat với nhân vật chưa hoàn thiện. Không đánh dấu tooltip hoàn chỉnh 100%. Magic-ring level 0/3 dùng ba option 10 theo nhánh source, level 1/2 không tự thêm bonus.

Kiểm thử: +9/+10/+15, shield, excellent, ancient; tooltip vũ khí excellent và staff dùng đúng Text ID. 5/5 client tests qua. GUI online tại X144/Y115 quan sát 59 FPS; chưa có đo tải dài hạn cho mọi map hoặc item.

## Font và fixture option

Winmain.cpp chọn GlobalText[0] (font thường/bold) và [18] (fixed); Main_EX603/Font.cpp chỉ áp FontInfo/FontName khi ChangeFontSwitch bật. Client hiện tại tắt switch, Text_eng.bmd[0]=Gulim. source_font.mm chọn theo các quy tắc này và đăng ký Gulim từ Office ở phạm vi process nếu macOS chưa thấy font. Không fallback sang system font. Chưa xác nhận raster/độ dày từng pixel giống GDI Windows; Cocoa vẫn là bộ rasterizer khác. Tooltip/panel dùng cùng font source, không dùng font hệ thống cho nội dung game.

native/tools/seed_option_test.sql tạo riêng MuOptions trên account muverify, giữ nguyên nhân vật cũ; lock bộ đếm serial và slot account trong transaction. Inventory 236×16 bytes theo DBItemByteConvert: slot14 Skull Staff +15 skill/luck option7 excellent63, slot18 Bronze Helm +13 luck option4 excellent9, slot44 Bronze Boots +9 luck option3 excellent3. Script không ghi đè khi đã tồn tại. Các item là dữ liệu DB thật, được DataServer/GameServer gửi tới client, không fixture chèn vào GUI.

## Tooltip màu và render item (2026-10-15)

item_ui_text_styled: giữ màu từng dòng thay vì chuỗi trắng; palette từ ZzzInventory::RenderTipTextList (green .1/1/.5, blue .5/.7/1, yellow 1/.8/.1); Excellent prefix Text620, đảo thứ tự excellent bits 5..0 theo source, dòng trống phân nhóm. Tooltip giới hạn 230 đơn vị GUI, wrap và canh giữa, Gulim. Chưa nối so sánh stat/class để tô đỏ yêu cầu, bonus set/socket/Harmony và toàn bộ quy tắc đặc biệt.

BMD parser giữ normals/bone và normal indices của từng face. Chrome shader dùng Chrome01/Chrome02/Shiny01 từ Effect; grade<3 không đổi, +3..4 đỏ, +5..6 xanh, +7..8 Chrome, +9..10 thêm Metal, +11..12 thêm Chrome2, +13..15 thêm Chrome4. Đây là port SceneKit cho nhóm equipment 0..11 với một phần palette theo loại; chưa port đầy đủ exception mesh/palette và hiệu ứng bone/particle +15 trên trang bị nhân vật/cánh. Không coi đã giống engine Windows 100%.

MuItemEffectsLayer ghép item vào một scene mỗi panel để shimmer khi không hover. MuHoveredItemView xoay model khi chọn; đóng/thoát hover giải phóng scene. Kiểm thử GPU có kiểm tra ảnh lỗi magenta, số pass, vòng đời renderer, màu title/option và chiều ngang tooltip.

Regression crash: report MuClient-native-2026-10-15-0908542.ips và 090922.ips xác định hideSlot gọi NSNumber::isEqualToNumber:nil dẫn tới Objective-C exception/SIGABRT trên main thread. Đã guard slot khác nil và thêm kiểm thử clear hover nil lặp lại, update inventory revision khi đang mở. 5/5 client tests chạy cùng quyền GPU qua sau bản sửa.

Bản sửa vòng đời Inventory: mở lại sau close phải dựng lại effects scene đã stop; kiểm thử attach/close/attach/close nhiều lần, cập nhật revision, clear hover nil lặp lại. 5/5 tests client qua sau sửa. Kiểm tra GUI NPC hiện còn chờ công cụ truy cập được màn hình Mac; không dùng kết quả unit/GPU test để khẳng định đã xác nhận phiên online.

## Client 0.20 — luồng vào game trong một cửa sổ (2026-10-15)

Chi tiết nguồn, tài nguyên và giới hạn tại `client/LOGIN-FLOW.vi.md`. Tách discovery → greeting → login → character list → world loading theo kết quả protocol thật. `lobby_view` quản lý layout/tài nguyên, `app.mm` điều phối session, `MuWorldViewer` nhận host window và có stop lifecycle. Dùng GlobalText/font gốc cho các nhãn đã đối chiếu, texture login/loading gốc. Cảnh chọn server/nhân vật 3D và toàn bộ sprite/control gốc chưa hoàn tất; các danh sách tạm vẫn là AppKit. Không ghi nhận milestone này là parity hình ảnh 100%.
