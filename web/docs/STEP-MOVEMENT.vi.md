# W0060 — bước di chuyển web đầu tiên (2026-10-26)

## Phạm vi đã triển khai

Debug nối phím mũi tên → C1:D4 → ACK đúng index → walk/idle → camera follow. Đây là bước nhỏ của M07, chưa phải click/pathfinding, full movement policy hoặc nghiệm thu online.

- `protocol/movement-codec.js`: profile EX603 ENG (GameServer/Protocol.h LANGUAGE1),13byte `PMSG_MOVE_RECV`,1–14hướng trong path[8], nibble và tọa độ biên0..255. D4 ACK8byte, correction157byte; index big-endian. Đối chiếu native/client/client.cpp path_packet và GameServer/Protocol.cpp CGMoveRecv.
- `session/world-session.js`: lấy index bản thân từ greeting của lobby, giữ tọa độ authoritative entry/ACK, chỉ1request pending, tối thiểu40ms giữa request, deadline4s. ACK index khác bỏ qua; own ACK sai endpoint hoặc packet lỗi đóng phiên; correction hủy pending và cập nhật vị trí. Close/send-failure/timeout xóa timer và reject. Chưa tự retry.
- `world/step-motion.js`: đi bộ1ô theo ACK, không dự đoán trước ACK. Nội suy tuyến tính theo quãng đường3tile/s (native12×25/100); dt tối đa50ms để không chạy bù khi tab quay lại. Đây chưa phải Catmull-Rom/turn policy gốc. Correction tăng revision, hủy interpolation và không cho promise ACK cũ ghi đè correction.
- Keyboard: ArrowUp/Right/Down/Left tương ứng hướng world1/3/5/7. Không gửi khi tabẩn, đang focus input/textarea/select/button/contenteditable hoặc giữ modifier. Chờ ACK và đi hết bước trước bước kế, không tích hàng đợi phím. Khi đóng/đổi phiên gỡ listener và dừng scene.
- Chặn terrain flags4/8, biênmap/cắt góc và giới hạn12ô mỗi trục quanh entry để không đi khỏi patch radius14 hiện có. Chưa tải tiếp terrain. API codec hỗ trợ đường14bước nhưng UI chỉ gửi1bước/cardinal.
- `world/live-view.js`: cấu hình WASM BodyAnimation motion với walk theo class/equipment adapter hiện có, sampleMotionResult idle/walk theo trạng thái. Không bật run/mount/wing policy hoặc tự sửa gameplay. Vẫn còn khác biệt pose/trang bị và safe/combat transition sau khi qua safezone.
- `world/camera.js` và model-preview: camera cùng góc/FOV, dịch theo vị trí render và terrain height; terrain không dựng lại từng frame. HUD cập nhật tọa độ đã ACK/correction; HP/Zen vẫn là snapshot entry, nhãn đã làm rõ.

## Nguồn tham chiếu

GameServer/Protocol.h PMSG_MOVE_RECV/PMSG_MOVE_SEND/PMSG_POSITION_SEND, Protocol.cpp CGMoveRecv và ObjectManager.cpp xử lý path là chuẩn server đang chạy. Native path_packet/motion và `file thieu/ZzzAI.cpp` MovePath, `ZzzCharacter.cpp` SetPlayerWalk/CharacterMoveSpeed là nguồn đối chiếu. Implementation bước đầu dùng đường thẳng/speedwalk cơ bản, chưa chuyển đủ các nhánh map/buff/mount/wing/boots/run của source bổ sung. Không sửa GameServer, Data hoặc config để cho phép di chuyển.

## Kiểm chứng

- 116/116 Node tests pass, log `/tmp/mu-w60-tests-final.log`. Test mới bao phủ8hướng/nibble/14bước/boundary, malformedreply, indexkhác, ACK endpoint, cooldown, overlappingrequests, correction, timeout, sendfailure/close, wall/corner, motion60/120Hz và không chạy bù10s, radius, camera dịch không đổi hướng, typing/tabẩn/modifier, staleACK và dispose.
- Build Debug/Release qua54/31file; staging179asset và profile Debug local. Release không thêm movement UI/module.
- Thêm `tools/live_movement_check.mjs`: chỉ nhận file credential owner-only của account testmuverify, chỉ chọn MacGuiDW có sẵn, chỉ Lorencia/safezone; dự định đi1ô rồi quay lại ô đầu bằng **session JavaScript thật** qua gateway/GameServer. Không log credential/identity/packet. Không tạo nhân vật hoặc sửaDB.
- Lượt live chưa thành công: GameServer trả logincode3 (đang có phiên đăng nhập). Không ép disconnect hoặc sửa trạng tháiDB để vượt qua. Một lượt trước dừng sau roster, chưa nhận movementACK; không ghi là pass và không có bằng chứng đã đi/đã lưu vị trí.
- Chưa có browser visual/live keyboard pass trong đợt này. Unit tests không chứng minh GPU pose/camera hay vị trí lưu sau logout. Không đóng M07.

## Thử trực tiếp

Mở `http://127.0.0.1:8080/?scene=servers`, tải lại để lấy module mới (phiên cũ sẽ đóng), đăng nhập, chọn nhân vật và vào map. Sau khi model nạp xong, dùng phím mũi tên, quan sát dòng server xác nhận X/Y và camera. Tối đa12ô quanh entry; UI không đi tiếp nếu gặp wall/biên giới hạn. Chưa nhận click để tìm đường.

Livecheck command, chỉ chạy khi accountmuverify rảnh:

```sh
node web/tools/live_movement_check.mjs native/runtime/client/credentials.txt /Users/angelale/Downloads/MuClientEX603/Data
```

## Phần kế tiếp

- Xác nhận thực trên browser: ACK→walk/camera/terrain, wall, disconnect/tabẩn và correction; sau đó logout/relogin kiểm vị trí server lưu. Không dùng account đang chơi song song để đo.
- Terrain picking/A*, route14bước và segment tiếp, retarget/correction tests; tải patch/camera khi đi xa. UI arrow hiện tại chưa thay thế phần này.
- Chuyển đủ gait/safezone/boots/buff/mount policy, Catmull-Rom và hướng quay theo nguồn đã audit. Đo frame CPU/GPU/ACK riêng; chưa cam kếtFPS.
- Effect bổ sung vẫn ở bước inventory/audit W0059; đợt này không thêm effect skill.


## W0061 — 2026-10-26: kiểm thử browser trên phiên người dùng mở sẵn

- Người dùng đã vào sẵn nhân vật DKTEST, map0. Kiểm thử trực tiếp tab `http://127.0.0.1:8080/?scene=servers`, không reload/logout hoặc đăng nhập thêm account.
- Bốn phím Right/Down/Left/Up lần lượt được UI báo **Server xác nhận**: (134,119) → (135,119) → (135,120) → (134,120) → (134,119). HUD tọa độ khớp mỗi ACK; kết thúc đúng ô ban đầu, phiên GameServer vẫn mở.
- Quan sát hình: nhân vật đổi hướng/pose đi bộ, mặt đất dịch khi camera theo; không thấy lỗi render/đóng phiên trong vòng thử ngắn. Overlay59.9–60.0FPS ở buffer1416×1798,DPR2. Đây là quan sát ngắn, không đo CPU/GPU percentile hoặc cam kết60FPS ổn định.
- Ctrl+Right không đổi tọa độ (134,119), đúng điều kiện bỏ modifier. Không thử wall/giới hạn12ô/tabẩn/disconnect/correction trên phiên này; các trường hợp đó vẫn chỉ có unit coverage.
- Ảnh bằng chứng cuối: `/tmp/mu-w61-movement.png`. Không sửa code gameplay hoặc rebuild trong lượt test; giữ nguyên phiên để người dùng tiếp tục.
- Phát hiện UI: HUD entry ở giữa phía trên chồng lên Debug overlay bên trái tại viewport khoảng708×899CSSpx. Chưa sửa trong lượt này vì rebuild/reload sẽ ảnh hưởng phiên đang dùng. Cần tách vị trí/responsive overlay.
- Chưa kiểm save sau logout/relogin, tốc độ/parity gốc, đi liên tục/chéo, clickA*, streaming, đủ7class hoặc chạy nhanh. M07 vẫnPARTIAL. Kết quả browser này bổ sung cho W0060, không biến lượt Node livecheck bị logincode3 thành pass.
