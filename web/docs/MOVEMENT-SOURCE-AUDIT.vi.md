# W0059 — kiểm tra di chuyển web và source bổ sung (2026-10-26)

## Kết luận và phạm vi

Di chuyển web **chưa triển khai**, không phải đã có rồi chỉ lỗi animation. Đã đối chiếu toàn chuỗi input → tìm đường → packet → server → render trong các file bên dưới. Chưa review từng dòng toàn bộ engine hoặc xác nhận mọi skill. Không thay source/Data/config gốc; đợt này thêm công cụ inventory và tài liệu, không thêm gameplay.

Docs đã đọc: STATUS, KNOWN-GAPS, WEBGL-TODO, SOURCE-MAP, TRANSPORT, CONVENTIONS, EVIL-SPIRIT và native/client/README. Một số docs lịch sử vẫn mô tả các phiên bản cũ; dùng trạng thái W0057–W0059 và mã hiện tại để đánh giá.

## Luồng di chuyển đối chiếu

| Phần | Source đã kiểm tra | Hiện trạng web / việc cần làm |
|---|---|---|
| Input | web/client/src/world/live-view.js; app/live-servers.js | Chưa listener click terrain/keyboard, picking screen→terrain hoặc xử lý focus/tab ẩn. Button hiện chỉ chọn nhân vật/vào map. |
| Pathfinding | native/client/pathfinding.hpp; file thieu/zzzpath.cpp; ZzzAI.cpp:799 PathFinding2 | Native A* chặn attr4/8 và corner cutting, chưa port web. Source mới có safe-zone/wall policy riêng; không coi native A* là bản sao nguyên thuật toán mới. |
| Packet outbound | native/client/client.cpp:363 path_packet; GameServer/Protocol.h PMSG_MOVE_RECV | Profile hiện tại C1:D4, 13 byte, x/y + path[8]; tối đa14 bước theo giới hạn native. Path[0] high nibble hướng đầu, low nibble số bước, hướng tiếp đóng high/low nibble. Web world-codec chưa có encoder này. |
| Server authority | GameServer/Protocol.cpp:1963 CGMoveRecv; ObjectManager.cpp quanh608 | Kiểm tra connected/regen/teleport/stun/immobilize/move hack; decode đường, giới hạn đích quanh vị trí hiện tại, chặn attr4/8, gửi ACK đích. ObjectManager tiếp tục kiểm tra đường. Không sửa/tắt các điều kiện này để test. |
| ACK/correction | native/client/client.cpp observe/movePath; Protocol.cpp CGPositionRecv; Protocol.h PROTOCOL_CODE3 | ACK D4 có index, x/y đích, dir; correction15 phải hủy đường. Web world-session chỉ nhận F3:03/E1, chưa xác định index bản thân để lọc ACK khỏi người khác, chưa timeout/retarget/correction movement. Không coi ACK đích là lệnh teleport render. |
| Nội suy/hướng/tốc độ | native/client/motion.hpp; file thieu/ZzzAI.cpp:682 MovePath; ZzzCharacter.cpp:6316 CharacterMoveSpeed | Source bổ sung dùng các điểm Catmull-Rom t=.25/.5/.75/1, điều kiện khoảng cách20, quay snap>=45° hoặc giảm nửa. Native dùng nội suy theo arc length; web live-view chỉ giữ góc/vị trí entry. |
| Walk/run | file thieu/ZzzCharacter.cpp:181 SetPlayerStop; :455 SetPlayerWalk | Policy có safezone, class, boot/glove, mount/wing, map và buffs; CharacterMoveSpeed có nhiều nhánh ngoài12/15. Không hardcode mọi class chạy3.75tile/s. Preview motion cycle không phải di chuyển online. |
| Terrain/camera/HUD | web/client/src/world/terrain.js, camera.js, live-view.js, entry-hud.js | Patch radius14 quanh spawn, camera cố định; chưa streaming/camera follow. HUD là snapshot entry; chưa authoritative movement HUD hoặc thay animation theo đường. |

## Kiểm thử và quan sát

- `npm --prefix web test`: **109/109 pass**, kết quả `/tmp/mu-w59-web-tests.log`. Các test hiện tại không có web movement encoder/session/input end-to-end; pass không chứng minh đi được.
- `ctest --test-dir native/build -R '^mu_client_(contract|render_contract|cache_contract)$' --output-on-failure`: contract và render_contract pass. Bao gồm test native tìm đường vòng tường, tránh cắt góc, retarget không snap và motion; không phải browser/server live test.
- cache_contract lần đầu fail `Upgrade icon not rendered` trong sandbox; chạy lại riêng ngoài sandbox pass3.62s. Không kết luận đã sửa một lỗi code vì không sửa renderer trong đợt này.
- Browser ban đầu hiển thị map0 tại148,129, 60FPS, trạng thái tự ghi chưa di chuyển; đây là quan sát đứng yên, **không phải test click/phím thành công**. Tab đó không còn khả dụng khi thử thao tác, nên không có bằng chứng click/di chuyển live trong đợt này. Tab kiểm tra riêng nhận1server từ ConnectServer rồi được đóng, không đăng nhập hoặc chiếm tài khoản người dùng.
- Chưa chạy MuClient-world-test live vì đó là native/test account khác và không chứng minh web. Không thay đổi vị trí/DB nhân vật để giả kết quả.

## Source bổ sung và Effect

Nguồn người dùng: `/Users/angelale/Downloads/SOURCE-2.0.7/file thieu`.
Inventory đọc toàn bộ39file C++/header, 3.648.890byte, hashSHA256 từng file + line anchors + include references ở `supplemental-source-audit.json`. Không copy cả source vào bundle web. 121tên include chưa có file tương đối trong thư mục bổ sung; đó là kết quả kiểm đường dẫn, không chứng minh chúng vắng mặt ở toàn bộ máy. Ví dụ stdafx.h, DSPlaySound.h, wsclientinline.h và nhiều manager. Chưa có version/build manifest xác nhận đây là engine đúng EX603; phân loại supplied supplemental reference.

| Phần | Điểm vào đối chiếu tiếp | Giới hạn |
|---|---|---|
| Fire Ball | ZzzCharacter.cpp:5082 AT_SKILL_FIREBALL → CreateEffect(MODEL_FIRE,...,1,to); ZzzEffect.cpp MODEL_FIRE create/update; ZzzOpenData.cpp:4166 model Fire | Có source cụ thể thay cho kết luận chung thiếu mọi nhánh; phải đối chiếu subtype1, target, lifetime, impact, sound SOUND_METEORITE01 với prototype Fire01 hiện có. |
| Energy Ball | ZzzCharacter.cpp:2173 và5136 AT_SKILL_ENERGYBALL; các nhánh monster trong cùng switch | Không suy mọi monster/player dùng chung sprite/timing. Cần truy vết hết nhánh và texture definitions trước khi thay prototype. |
| Evil Spirit | ZzzCharacter.cpp các CreateJoint BITMAP_JOINT_SPIRIT; ZzzEffectJoint.cpp:641 creation và3834 update; ZzzEffect.cpp MODEL_LASER; ZzzOpenData.cpp:4175 Laser | Có nhiều subtype/scale/target, đặc biệt scale80 sinh MODEL_LASER và có logic AttackCharacterRange. Không bê damage/hit phía client thay quyền server; numeric native4head không tự chứng minh parity nguồn mới. |
| Hệ effect | ZzzEffectMagicSkill/Particle/Joint/BlurSpark/FireLeave/Point/Pointer/NoUse.cpp, zzzeffectsprite.cpp; ZzzTexture/ZzzBMD/ZzzOpenglUtil | Inventory có toàn bộ hash, chưa port/review từng nhánh, chưa xác minh texture/material/UV/timing/dependency và skill coverage7class. |

Tái lập inventory (chạy ở Emulator):

```sh
python3 web/tools/audit_supplemental_source.py --source '/Users/angelale/Downloads/SOURCE-2.0.7/file thieu' --output web/docs/supplemental-source-audit.json
```

## Các phần tiếp theo, chỉ đóng khi có bằng chứng

1. Movement codec fixture đối chiếu native/server, index bản thân từ greeting và ACK/correction đúng index; session một đường pending, timeout, đóng phiên và tabẩn không gửi dồn.
2. Terrain picking + pathfinding, boundaries/attr4/8/corner tests; chia đường14bước và hạn chế tần suất theo source, không chỉ thay tọa độ DOM.
3. Actor walk/stop và nội suy quãng đường, camera follow/patch quanh vị trí mới; retarget không giật/teleport. Walk/run chỉ sau khi có metadata inventory/map/buff cần thiết.
4. Live browser: ô cardinal/diagonal, wall, click retarget, correction, timeout/disconnect và tabẩn; đo vị trí server, vị trí render, ACK latency/FPS riêng. Chỉ sau đó xác nhận logout/relogin lưu vị trí.
5. Effect audit riêng từng skill: source→asset→material→animation/timing→sound→server packet, trước mắt Fire Ball/Energy Ball/Evil Spirit. Mỗi skill có task/test và giới hạn riêng.
