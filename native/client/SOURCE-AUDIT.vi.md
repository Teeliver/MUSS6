# Rà soát bộ SOURCE cho client EX603 macOS

Đã rà soát toàn bộ `/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/`, kể cả file bị ignore/ẩn khi thống kê filesystem. Không tìm thấy executable Windows hoặc archive source trong bộ này. Program không có file. Source/Media có tài nguyên biểu tượng, không có C/C++ engine.

## Thành phần dùng được

| Thành phần | Vai trò trong client native |
|---|---|
| Main_EX603/Main/Protocol.h và Protocol.cpp | Packet extension, trạng thái player, HP/MP/SD/BP, reconnect; đối chiếu với GameServer/Protocol. Các lời gọi về địa chỉ engine phải thay bằng implementation native. |
| Main_EX603/Main/PacketManager, HackCheck | Cipher, XOR, serial và transport. Client 0.2 đã dùng key thật và cipher C++ port từ GameServer, đối chiếu luồng DataSend của Main_EX603. |
| Main_EX603/Main/CustomItem, CustomWing, CustomMonster, CustomMap, CustomTooltip | Bảng cấu hình, ID/model name, quy tắc tính thuộc tính và nội dung UI. Tái sử dụng dữ liệu và logic độc lập; không tái sử dụng hook địa chỉ Windows. |
| Main_EX603/Main/Camera, Resolution, HealthBar, PrintPlayer | Tham khảo hành vi/giao diện. Camera/renderer hiện đọc/ghi địa chỉ trong main.exe, cần implementation native. |
| GetMainInfo_EX603/GetMainInfo | Schema MAIN_FILE_INFO, version/serial, event entry levels, các bảng custom; trình đọc text có thể port. Đây là tool sinh cấu hình, không phải game engine. |
| Emulator/GameServer | Protocol và hành vi authoritative, định nghĩa inventory/skill/map/movement/viewport. Nguồn kiểm chứng gameplay tương thích EX603. |
| Util/lua, pugixml, sha1, cryptopp | Source thư viện; chỉ port/link module thực sự cần. Lua đã có bản source native trong native/vendor. |
| MHPClient/MHPServer | Protocol anti-hack bổ sung; phần Windows process/registry/DLL scanning không dùng trực tiếp trên Mac. |
| Main_EX803 và Main_EX101/201/301/401 | Tham khảo khác phiên bản, không trộn địa chỉ/packet vào EX603. Các project Main đều khai báo DynamicLibrary. |

## Engine chưa tìm thấy

Main_EX603 có 34 file .cpp và 36 header .h. Main.vcxproj khai báo DynamicLibrary. Offset.h định nghĩa pLoadImage/pLoadItemModel/pLoadItemTexture/pTransformPosition/pCreateCharacter là function pointer trỏ tới địa chỉ executable. Map.cpp patch/check/hook map và gọi hàm engine ở địa chỉ cố định; không có implementation terrain loader/renderer. Camera.cpp trỏ vào vùng nhớ camera của executable. Vì vậy thư mục có code client mở rộng, nhưng chưa tìm thấy source implementation engine được các hàm này gọi.

Trong MuClientEX603, tài nguyên model/texture/map/audio dùng được; chưa tìm thấy C/C++ project hoặc main.exe. File Data/World3/World3.zip là archive tài nguyên map, không phải bằng chứng có source engine.

## Hướng phát triển từ bộ hiện có

Giữ protocol/dữ liệu/logic EX603 làm nguồn chuẩn; viết platform và engine macOS cho phần thiếu. Mốc tiếp theo: đọc texture/terrain Lorencia từ Data, dựng map, nạp model nhân vật và animation; sau đó xử lý viewport và movement. Đây là client mới tương thích EX603, không có cơ sở gọi là port engine gốc 100% khi engine implementation chưa có trong bộ đã kiểm tra.
