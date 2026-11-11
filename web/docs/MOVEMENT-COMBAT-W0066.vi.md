# W0066 — Di chuyển liên tục, click đất và đánh thường

2026-10-26. Trạng thái **PARTIAL cho toàn gameplay**, đã có phần foot movement/click/normal attack. Docs W0059 là lịch sử audit, không mô tả trạng thái hiện tại.

## Nguồn đối chiếu

- `file thieu/ZzzCharacter.cpp`: SetPlayerWalk455+, CharacterMoveSpeed6316+; safe zone reset Run; foot branch12/15 đơn vị trên tick25Hz, TERRAIN_SCALE100; ngưỡng40tick. CLASS_DARK là MG (nhánh không helmet), không phải DK. MG/DL/RF hoặc boot visual grade>=5 ở map đất mới đủ điều kiện chạy. Không dùng tốc độ này thay mọi mount/map/buff.
- `file thieu/ZzzAI.cpp:799` PathFinding2 gọi path->FindPath với nhiều wall policy/fallback. Web dùng A* giới hạn65.536 node, tám hướng RoadPathTable, chặn attr4/8 và cắt góc theo native pathfinding.hpp; **không coi là port nguyên thuật toán hai lượt của Windows**. Chưa collision occupancy của các nhân vật khác.
- `GameServer/Protocol.cpp:1963` CGMoveRecv, `Protocol.h` PMSG_MOVE_RECV/SEND, `native/client/client.cpp` path_packet: C1:D4 13byte, tối đa14 hướng; ACK đích không teleport hình render. Giữ kiểm tra movement/anti-hack server.
- `file thieu/ZzzInterface.cpp:7430–7443`: click path thành công tạo MODEL_MOVE_TARGETPOSITION_EFFECT scale0.6; `ZzzOpenData.cpp:4573,5531–5532`: Effect/MoveTargetPosEffect.bmd, cursorpin01/02.jpg (Data lưu OZJ). `ZzzEffect.cpp:4848+,15021+,20009+`: light vàng/cam, lifetime, terrain rings. Web dùng model + hai texture gốc, additive, scale/tint/expire; chưa đủ magic/spear joints/spark và chưa khẳng định composite/timing parity100%.
- `GameServer/Attack.h`, `Attack.cpp:2788`, `Protocol.h/cpp`: normal attack ENG C1:11 7byte, action120, target/direction; server chặn safe zone cả người đánh và mục tiêu, tầm3 ô hoặc Elf6 ô. Damage C1:11 với ViewDamageHP32bit, action18, death17, reward9C, HP26, regenF3:04. ABI test biên dịch **struct thật** với UPDATE603/EXTRA1 kiểm sizeof/offsetof.

## Thay đổi runtime

- `navigation.js`: route queue nội suy theo khoảng cách, tiêu thụ thời gian qua nhiều segment, hướng quay theo góc ngắn nhất. Đi bộ3tile/s; foot run3.75tile/s sau1.6s đủ điều kiện, safe zone luôn đi bộ. Không phụ thuộc key-repeat của OS. Run eligibility dùng appearance roster hiện tại, chưa authoritative inventory/buff sync; nhánh water-map glove/mount/wing/freeze chưa triển khai.
- `world-controls.js`: giữ mũi tên để đi liên tục; thả phím/ngắt focus/menu/ẩn tab dừng ý định mới. Click đất inverse camera/raycast terrain, A* rồi chia packet nhỏ2ô (đuổi quái1ô), gửi trước cuối segment để hạn chế ngắt. Click/ESC hủy phần đường **chưa gửi**; đoạn đã ACK hoàn tất, không giả teleport hoặc hủy server path. Độ trễ mạng cao vẫn có thể làm chờ tại cuối đoạn.
- `picking.js`: dùng đúng ma trận worldCamera theo camera follow, aspect và cao độ. Chọn quái theo vị trí hình đang nội suy, tâm cao đo từ model, bán kính30CSSpixel. Đây là proximity picking, chưa triangle/depth hit-test; quái bị vật khác che vẫn có thể được chọn khi ở gần tâm màn hình.
- Core `BodyAnimation.configureWorld/setSafeZone`: giữ sẵn các clip safe idle/walk, armed idle/walk/run/attack; đổi safe flag của attachment mỗi frame, không parse lại BMD trong vòng render. MotionAnimation thêm mode world riêng, giữ hành vi preview spell cũ; blend0.12s và giữ phase walk/run.
- `ActorAnimation.configureMonster/sampleActionResult`: giữ clip0/2/3/4/6, kiểm rig/geometry và khóa rootXY. Monster hình nội suy theo server snapshot; attack18 chọn clip3/4, reset clock; death server hiện loại khỏi renderer, chưa hoạt cảnh corpse/death hoàn chỉnh.
- Click quái hỗ trợ đã thấy: tự đi vào tầm rồi đánh thường, cooldown bảo thủ650ms/attack animation600ms. ESC/click đất/di chuyển phím đổi ý định; quái chết/despawn hoặc player chết hủy target. Không tự tính damage, HP, EXP hay spawn. Chưa adaptive attack-speed và skill.
- `world-session.js` đọc damage/HP32bit, death, reward, nhận F3:04 hồi sinh và dựng lại scene tại vị trí server. HUD cập nhật HP; level/EXP/Zen trong trận, MP/SD/AG và dùng potion chưa là full HUD.
- Bốn monster binding đã có source và dependency/clip kiểm chứng: **2 Budge Dragon,3 Spider,11 Ghost,12 Larva**. Actor catalog có kind=monster; không cho NPC thành mục tiêu combat. Chưa binding mọi quái, đặc biệt quái cần vũ khí/part riêng.
- Hiệu ứng click có hash baseline model/texture và hash helper; decoder texture cho phép đúng đường dẫn Effect một filename, vẫn chặn traversal. Tài nguyên Data/source/server/DB gốc không sửa.
- `?scene=terrain-check&map=0&x=140&y=125&walk=1` là **mô phỏng OFFLINE**, nhãn rõ không packet/damage, để test input/motion/resource độc lập; không tính là bằng chứng server gameplay. Ngoài Lorencia vẫn giới hạn12ô do scene map khác còn nạp từng vùng.

## Kiểm chứng

- Build Emscripten6.0.13; WASM framing/crypto/model/texture/animation contracts pass. World-mode test xác nhận armed idle/walk dùng rate tương ứng, không bị coi là spell, walk↔run giữ phase. 28 tổ hợp class/equipment giữ được đủ world clips.
- Node tests kiểm A* vòng tường/corner/bounds, 60/120Hz, safe run gate, projection inversion ở hai aspect, stale ACK/correction/dispose, click target và safe-zone/death guards, packet/layout, damage/death/regen theo server.
- **Online trên phiên MuOptions do người dùng vào sẵn:** click đất có ACK135,139 rồi134,139; giữ Right4s qua cửa đông từ170,127 tới184,127; scene khoảng60FPS. Click Spider nhận **“Hạ quái #61:78”**, HP65→64. Đây là phản hồi server thật; không đổi DB/teleport để tạo kết quả.
- Marker bản đầu không hiện vì textureReference thiếu Effect; phát hiện khi test browser và sửa. Tab OFFLINE bản sửa tại140,125 click sang139,123 thấy model/rings vàng: `/tmp/mu-w66-marker-offline.png`. Không dùng ảnh OFFLINE để khẳng định damage online. Online kill được quan sát trong AX; ảnh `/tmp/mu-w66-live-kill.png` chụp muộn nên status đã bị packet self-miss0 ghi đè; bản cuối bỏ việc self-miss0 ghi đè thông báo kill.
- Build/test trong thư mục tạm và swap output hoàn chỉnh; không chạy build test xóa dist đang phục vụ. Phiên đang mở cần refresh/đăng nhập lại để nhận build cuối.

### Kết quả build cuối

143/143 client tests pass, không skip; `npm --prefix web run test:wasm` pass (bao gồm world motion mode contract mới). Logs `/tmp/mu-w66-tests.log`, `/tmp/mu-w66-contracts.log`, `/tmp/mu-w66-wasm.log`. Debug71file/Release31file,179preview asset và authorized runtime profile được stage trước khi swap. Backup cuối: `web/.build/deploy-backups/W0066-20261026-152946`.
