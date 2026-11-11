# Đọc lại kiến trúc server → client web — 23/10/2026

## Phạm vi và mức xác minh

Đã đọc kiến trúc, entrypoint, những đoạn xử lý chính của login/chọn nhân vật, inventory/equipment, di chuyển/animation, nhập BMD và render item. Đây là bản đồ nguồn và điểm cần kiểm chứng; không phải tuyên bố đã đọc từng dòng hoặc kiểm thử toàn bộ source.

Kiểm kê tại thời điểm đọc: GameServer EX603 có 484 file C++/header; Main_EX603/Main có 70; client web/src có 198 file TS/TSX; OpenMU/src có 3.969 file C# sau khi bỏ bin/obj. Số lượng file không chứng minh hành vi đã đúng. Không sửa gameplay hoặc renderer trong lượt nghiên cứu này, không restart dịch vụ, không thay dữ liệu nhân vật.

## Các cây nguồn phải phân biệt

| Cây nguồn | Vai trò |
|---|---|
| `/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator` | Server EX603 C++: Connect/Join/Data/Game; native là bản port macOS và adapter |
| `../Main_EX603/Main` | DLL hook Win32, protocol bổ sung, nạp model/texture custom, địa chỉ hàm trong executable PC |
| `/Users/angelale/Projects/OpenMU` | Server C# dùng được với client web; message handler → player action → view/serializer → packet |
| `/Users/angelale/Projects/muonlinejs` | Client TS/React/Babylon, Bun proxy WebSocket↔TCP, công cụ chuyển/nhập asset |
| `/Users/angelale/Downloads/MuClientEX603/Data` | BMD, texture, terrain và resource PC đầu vào |

`Main_EX603` không chứa implementation của mọi địa chỉ mà nó gọi. Ví dụ ItemModelLoad gọi 0x00617D30; GetItemColor fallback 0x005F8C50; Offset.h định nghĩa pRenderPartObjectEffect ở 0x00609E70. Không suy ra toàn bộ camera/lighting nguyên bản chỉ từ BMD hoặc các hook này.

## Đường chạy và lưu dữ liệu

`Emulator/start-openmu.command` chuyển tới `/Users/angelale/Projects/start-openmu.command`. Launcher đó chạy OpenMU với `-demo -autostart -deamon -resolveIP:loopback`, ASP.NET localhost:8080 và thông báo progress không được lưu. Đây là cấu hình launcher, không phải xác nhận mọi tiến trình đang chạy dùng đúng launcher này.

Client web mặc định Bun proxy localhost:3000; proxy chuyển byte tới TCP đích được chỉ định và mặc định chỉ cho loopback. Vite phục vụ client; địa chỉ ConnectServer còn phụ thuộc cấu hình/local storage của client. Không gộp luồng OpenMU này với server C++ native chỉ vì cả hai đều nằm trên máy.

## Chọn nhân vật và class

- `muonlinejs/src/ui/pages/charactersPage/index.tsx`: chọn tên, focus, gửi enter; preview đọc appearance từ danh sách, khác đường tạo mới dùng newCharClass.
- `src/store.ts::selectCharacterRequest`: xác nhận tên có trong danh sách, đặt focusedChar, bỏ actor local cũ, gửi SelectCharacter.
- `src/logic.ts`: CharacterInformation cập nhật vị trí, chỉ số, tài nguyên và phát requestWarp; CharacterInventory deserialize toàn bộ danh sách item rồi syncPlayerAppearance.
- `src/common/deserializeAppearance.ts`: class và appearance là bitfield, không phải lấy trực tiếp index của nút tạo class.
- OpenMU: `RemoteView/Character/ShowCharacterListPlugIn.cs` ghi appearance; `PlayerActions/Character/SelectCharacterAction.cs` chọn theo tên thuộc account.
- EX603: `GameServer/Protocol.cpp` dispatch F3 và CGCharacterInfoRecv chuyển yêu cầu tới DataServer.

Khi sửa lệch class phải đối chiếu tên được chọn → appearance → class mapping → actor hiện tại; không dùng class tạo mới làm fallback thiếu căn cứ.

## Inventory và item bytes

OpenMU: `MessageHandler/Items/ItemMoveHandlerPlugIn.cs` đọc item-size theo serializer của client, gọi `GameLogic/PlayerActions/Items/MoveItemAction.cs`; action kiểm tra storage, item, chỗ trống, yêu cầu và rollback. `RemoteView/Inventory/ItemMovedPlugIn.cs` gửi slot và item bytes sau kết quả.

EX603: `GameServer/Protocol.cpp` dispatch 0x22 nhặt, 0x23 thả, 0x24 chuyển, 0x26 dùng. `ItemManager.cpp::CGItemMoveRecv` kiểm tra trạng thái/interface/storage và trả result 0xFF khi thất bại. `GCItemListSend` gửi F3:10 theo slot. Đây là đường tham chiếu riêng, không thay trực tiếp handler C#.

Web:
- `src/common/itemSerializer.ts`: NeededSpace=12; ưu tiên giữ rawData đủ 12 byte khi gửi lại. Đường serialize item tự tạo còn nhiều nhánh option chưa triển khai; không đánh đồng với item thật giữ rawData.
- `inventoryActions.ts`: nhấc item là selected slot local; gửi move rồi chỉ cập nhật sau ItemMoved; có pending, từ chối và timeout. Phạm vi đang cho storage 0 và item đích trống, chưa phải mọi thao tác swap/stack của PC.
- `inventoryLayout.ts`: 12 slot mặc đồ, túi cơ bản 8×8; footprint lấy X/Y từ ItemsDatabase; kiểm tra va chạm/bờ túi.
- `store.ts::syncPlayerAppearance` và `ecs/systems/appearanceSystem.ts`: cập nhật từng phần trang bị của actor, tách khỏi hình item trong Inventory.
- `pcItemRequirements.ts` dùng bảng pcItemAttributes xuất từ initializer Season Six; ghi chú trong code cảnh báo các cột stat của items.json cũ bị lệch. Cần đối chiếu lại với cấu hình server thực tế trước kết luận yêu cầu mặc đồ chuẩn.

## DATA → model → hình Inventory

`tools/exportNativeBmdInventory.ts` → `nativeInventorySources.json` → `loadNativeInventoryBmd.ts` → `ItemHoverPreview.tsx` → `NativeItemIcons`/`nativeItemIcons.ts` → `ItemIcon`.

- Mapping hiện có 652 item, 584 đường model riêng; không có nghĩa mọi item ở server đã được hỗ trợ.
- Exporter dùng items.json và override Helper01..04, tìm file không phân biệt hoa thường; có fallback basename. Fallback cần kiểm tra trường hợp texture trùng tên ở nhiều folder.
- BMD được copy trực tiếp; OZJ/OZT được giải mã ra PNG. Không suy ra ảnh render giống PC chỉ từ việc copy đúng byte.
- Loader đọc action 0/frame 0; đồ nhóm 7..11 dùng skeleton Player và bodyHeight; bỏ mesh hide/skin/hair. Resource wearable và inventory preview có thể dùng cách đặt pose khác nhau.
- Renderer hiện dùng camera orthographic tự fit, tâm bounding box, canvas hover mở rộng, ảnh idle chụp/cache. Đây là implementation web mới.
- Tại thời điểm đọc có depth mirror cho nhóm 7..11, yaw riêng cho một số pet, turntable quanh trục màn hình, trim ảnh snapshot và visibleIconRect. Các điều chỉnh này cần bằng chứng đối chiếu, không gọi là quy tắc PC đã xác nhận chỉ vì comment ghi như vậy.
- Shader `gemPreviewMaterial.ts` dùng Chrome01/02/Shiny01, các nhánh cấp + và Excellent, brightness 1.35. Đây vẫn là shader tái dựng; độ sáng, alpha, blend, gamma và normal phải kiểm chứng riêng.
- `pcItemDisplay.json` và `pcLocomotion.ts` có nguồn engine tham chiếu bên ngoài; chưa chứng minh trùng executable EX603 của bộ DATA này.

## Di chuyển và animation

- Client `NetworkSystem` đóng gói đường đi; `store.sendWalkPath` ghi direction/step; không suy ra tốc độ animation từ tốc độ network.
- `MoveAlongPathSystem` cập nhật transform theo deltaTime/tốc độ; `AnimationSystem` tính safe zone mỗi frame, chọn locomotion và stow vũ khí.
- `pcLocomotion.ts` lựa chọn đứng/đi/chạy theo class, safe zone và trang bị; nhịp key BMD được đổi theo 25 Hz tham chiếu.
- `AppearanceSystem`, `playerObject`, `modelObject`, `characterShadowSystem`, `cameraFollowSystem` là các điểm độc lập khi kiểm tra tư thế vũ khí, bóng, camera và độ mượt.
- Mục tiêu 60 FPS cần đo frame time tại runtime. Đọc code không chứng nhận 60 FPS.

## Phần đã tìm được đầu mối nhưng chưa kiểm tra hết

Combat/skill/monster/viewport: GameServer Attack, SkillManager, Monster, Viewport; giao dịch Shop/Trade/Warehouse/ChaosBox; quest/event; persistence/procedure native; plugin và các version serializer OpenMU. Đã xác định nơi dispatch/entrypoint, chưa đọc và test toàn bộ nhánh trong lượt này. Bản audit native/audit/README.vi.md và source-manifest.json là lịch sử kiểm kê, không tự nâng trạng thái thành đã xác minh.

## Cách bắt đầu sửa tiếp

1. Chốt một hành vi bằng ảnh hoặc thao tác tái hiện, phân biệt idle pose với hướng quay hover.
2. Chỉ ra file DATA, mapping, pose/bone, camera/pivot, normal/culling và shader đang tham gia.
3. Với nghiệp vụ, đối chiếu packet/version và server trả lời; với hình ảnh, giữ server/data nhân vật ổn định.
4. Sửa đúng tầng, kiểm tra trong game lẫn trang QA; không lấy góc debug được giữ lại trong QA làm góc mặc định.
5. Báo phần thực sự được kiểm chứng; không dùng build pass, file gốc hoặc source tham chiếu làm bằng chứng 100%.
