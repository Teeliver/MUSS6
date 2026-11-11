# Chết và hồi sinh Duel — 05/11/2026

## Hiện trạng đã sửa

- Self trước đây chỉ khóa điều khiển khi chết nhưng vẫn sample idle/run; remote bị loại ngay bởi `!e.dead` trong player layer. Giờ cả hai dùng **PLAYER_DIE1/action231** từ `Data/Player/Player.bmd`, giữ tư thế cuối tới khi server destroy/respawn. World motion thêm slot16/17 cho231/232, không đổi các mode ngồi/dựa có trước.
- Nhịp chết **0.45 ×25 =11.25 keys/s** theo `ZzzOpenData.cpp`; lấy khung cuối, không lặp đứng dậy rồi ngã lại. Giữ rig multipart/gear và âm thanh nhân vật gốc.
- Remote giữ corpse trong viewport, ngừng tiến tới tọa độ di chuyển; spawn cùng index mới bỏ trạng thái chết, snap tọa độ và bỏ attack/cast cũ. Server destroy vẫn thu hồi renderer bình thường.
- Self hồi sinh trong cùng map dùng position packet để reset motion, mở lại điều khiển và đổi pose, **không tải lại terrain/model cả map**.
- Winner được server di chuyển khi bắt đầu vòng Duel kế tiếp: xử lý ngay trên scene hiện có, vẫn gửi `C1:F3:12 FinishLoading` vì server đặt RegenOk=1. Không tạo effect Teleport skill thay cho việc reset vòng.

## Lỗi server cụ thể

`Duel.cpp::RespawnDuelUser` đang chọn chính người chết làm `lpTarget` ở cả hai nhánh Gladiator[0]/[1]. Vì vậy random/teleport/reset người chết lần nữa và không reset đúng đối thủ sống.

Đã đảo lựa chọn sang gladiator **còn lại** và kiểm tra index tương ứng. Giữ nguyên thuật toán vị trí của source: `CMap::GetMapRandomPos(...,18)` chọn quanh tọa độ cũ, loại ô attribute4/8, tối đa50 lần. Không tự đặt spawn cố định hay chuyển về Lorencia giữa trận. Server quyết định tọa độ, client chỉ áp dụng packet. Vì hai vị trí được chọn độc lập, đối thủ có thể ra ngoài tầm viewport; không dựng một player giả khi server không gửi viewport.

Chết trong trận: `UserDieProc` cộng điểm; `ObjectManager` đợi `MaxRegenTime(4000)+1000ms` rồi hồi sinh. Người chết nhận `F3:04`; người sống nhận teleport cùng map; AA0D reset vòng được gửi **cho spectator**, không phải ACK của hai đấu sĩ. Hết trận mới áp dụng luồng kết thúc/đưa về làng riêng.

## Đối chiếu

- `Main5.2/source/ZzzCharacter.cpp::SetPlayerDie`: PLAYER_DIE1 cho player thường.
- `Main5.2/source/_enum.h`: PLAYER_DIE1=231, PLAYER_DIE2=232.
- `Main5.2/source/ZzzOpenData.cpp`: PlaySpeed0.45, Loop=true (giữ cuối theo engine PC).
- `GameServer/Duel.cpp::UserDieProc/RespawnDuelUser/GCDuelRoundStartSend`.
- `GameServer/ObjectManager.cpp::ObjectStateProc/ObjectSetStateProc`, `Map.cpp::GetMapRandomPos`, `User.cpp::gObjTeleport`.

## Kiểm tra

- **501/501 web test qua** trong cây cô lập. Thêm kiểm tra corpse không bị xoá trước destroy; chết→spawn cùng index; mode chết giữ cuối; server chọn đúng đối thủ ở cả hai vị trí; teleport reset vòng không reload nhưng vẫn FinishLoading.
- **8/8 native GameServer contract qua**; GameServer và WASM đã build. WASM framing/protocol/BMD/texture contract chạy qua. Full native34 lần này dừng ở bài Metal `mu_client_elf_bow_contract`, không có kết quả đủ34; không ghi nhận nó là pass. Các kiểm tra GPU browser bên dưới đã thực hiện riêng.
- Browser fixture `?scene=terrain-check&map=0&x=140&y=80&walk=1&deathtest=1`: hai nhân vật ngã và nằm; self mode16/frame29 giữ cuối, hồi sinh trở về mode4. [Chết](reports/duel-death-20261105/dead.png), [hồi sinh](reports/duel-death-20261105/revived.png). Quan sát60FPS ở lần kiểm tra này. Fixture không socket/damage thật.
- **Hai phiên QA thật:** A hạ B, B HP0 và A nhận viewport B `dead:true`; điểm A1–B0. Sau đó A nhận tọa độ(100,60), B hồi sinh(112,68), cả hai còn map64; B HP159/159 và đi thêm một bước thành công. Không cần tải lại map ở round này. [Trace](reports/duel-death-20261105/live-server.json).
- Đã đóng và xoá đúng hai QA account/character + hàng phụ trong12 bảng được kiểm tra schema. Còn0 hàng QA trong danh sách đó; MuOptions/DKAD không đổi. [Biên bản](reports/duel-death-20261105/qa-cleanup.json).

## Triển khai và giới hạn

GameServer mới đã nạp lúc xác nhận0 account online. Frontend debug174file đã publish, có WASM mới; phiên game đang mở cần refresh để nhận animation code mới. Không restart Join/Data/Connect/Gateway.

Không coi lượt này là chứng nhận các hiệu ứng phân rã riêng của morph/monster, mọi kiểu chết bởi skill, hay toàn bộ mount/class. Đã kiểm tra death thường bằng rig Player gốc và trọn vòng Duel nêu trên.

Chỉ giữ `native/runtime/backups/mu-web-code-latest.tar.gz` cho code trước đợt này; rollback phải build lại WASM/GameServer. Không giữ thêm các binary/cây test tạm. Hai backup DB/MuOptions độc lập được giữ nguyên.
