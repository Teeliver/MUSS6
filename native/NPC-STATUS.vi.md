# NPC interaction — bản kiểm thử

Click model NPC gần trong 5 ô gửi C3:30 mã hóa (HackPacketCheck index48 yêu cầu Encrypt=1) với index từ viewport thật. Phản hồi C3:30 (sau giải mã), C1:F9:01 (Gens), C2:31 (item list) theo NpcTalk.h, QuestWorld.h và Shop.h của GameServer. Đóng gửi C1:31, heartbeat tiếp tục trong lúc hội thoại mở. Không gửi mua/bán/chuyển vật phẩm.

GUI hiện là hộp hội thoại kiểm thử phản hồi và số vật phẩm, chưa phải giao diện kho/shop/Gens hoàn chỉnh của MU. NPC im lặng hết 5 giây sẽ hiện lỗi và gửi đóng; không tự tạo dữ liệu.

Cursor lấy Data/Interface/Cursor.ozt và CursorTalk.ozt. Source engine tham chiếu ZzzOpenData.cpp ánh xạ BITMAP_CURSOR+4 vào CursorTalk.tga; ZzzInterface.cpp RenderCursor dùng 4 ô, nhịp 100ms theo 0,1,2,3,2,1, kích thước 24x24, hotspot 2x2. Main_EX603 là DLL mở rộng nên đối chiếu engine tham chiếu không phải bằng chứng engine EX603 giống 100%.

FPS: tránh ghi lại transform không thay đổi, giữ cache pose đã áp dụng để các thay đổi nhỏ tích lũy đúng. Mục tiêu 60FPS; cần đo GUI online trên Mac mở khóa, không coi build/test là bằng chứng FPS đạt 60.

Kiểm thử live 2026-10-14: Warehouse class240 tại146,113 trả result2 sau C3:30, nhận item list, C1:31 đóng, phiên heartbeat giữ kết nối. GUI chưa xác nhận do Mac khóa.

## Warehouse 0.16

GUI là NSView cố định trong vùng game, không phải cửa sổ riêng. Skin gốc từ Data/Interface, bố cục đối chiếu NewUIStorageInventory.cpp: 190×429, 8×15 ô 20×20 tại15,36; khung và Zen theo tọa độ source. Các nút tiền/khóa chưa bật vì giao dịch chưa triển khai. Icon item vẫn là mã tạm, không khẳng định giống hoàn chỉnh100%.

Text dùng bảng GlobalText ký hiệu GT, count32bit, key/length32bit, XOR FC-CF-AB cho từng chuỗi. Đọc Data/Local/{Eng,Spn,Por}/Text_{eng,spn,por}.bmd theo Language trong config/client.ini. UI thử UTF-8 rồi Windows1252; chưa hỗ trợ tùy ý mọi codepage. Mục234 Storage,240open,241close,266Storage fee. Cả3 file hiện tại cùng các chuỗi tiếng Anh này.

Zen phân cách hàng nghìn bằng dấu phẩy, căn phải tạix168/y350; không thêm hậu tốZen. getGoldColor tham chiếu: dưới100000 RGB255,220,150; từ100000 RGB0,201,24; từ1000000 RGB255,150,0; từ10000000 RGB255,0,0. Chưa hiển thị số phí kho vì chưa nối đủ trạng thái Level/MasterLevel và quy tắc EX603.

Terrain được chia batch16×16, dùng chung material/texture để SceneKit cull vùng ngoài khung nhìn. GUI đã quan sát59–60FPS tạiWarehouse với10.9k tam giác (trước179k). Log vẫn có frame spike, không coi một ảnh60FPS là đảm bảo mọi frame đạt16.67ms.

Source engine tham chiếu: https://github.com/oXaa/muonline-client-sources/blob/master/NewUIStorageInventory.cpp , GlobalText.h , ZzzInventory.cpp. Chưa chứng minh engine này trùng chính xácEX603.
