# Resource và NPC theo map — W0063/W0064

Ngày 2026-10-26. Đây là trạng thái implementation web đang chạy local, **chưa phải full resource/NPC hoặc parity 100%**.

## Phạm vi hiện tại

- Catalog của 47 thư mục World, các thư mục Object tương ứng, toàn bộ 104 model trong Npc và 344 model trong Monster. Inventory/decoder là kiểm tra resource, không tự tạo instance gameplay.
- 3.825 tham chiếu model được kiểm tra; 3.709 qua decoder/dependency, 116 còn lỗi hoặc ánh xạ chưa đủ. Không gọi các trường hợp chưa qua là Data gốc hỏng.
- 63 binding NPC hiện có đều qua kiểm tra model, texture và skeleton. Đây không phải toàn bộ class NPC: cấu hình 61 map có 123 vị trí NPC; còn 27 class trong các cấu hình này chưa hoàn thiện binding.
- Online nhận NPC từ viewport server. Offline chỉ preview theo vị trí cấu hình, có nhãn rõ và không gửi packet/spawn NPC thật.

Catalog runtime: `web/client/src/world/resource-catalog.json`. Báo cáo mỗi map và danh sách NPC thiếu: `world-resources-W0064.json`. Báo cáo W0063 được giữ làm lịch sử.

## W0063 — nền tảng resource và NPC

### Môi trường

- Đọc `EncTerrainN.obj`, kiểm tra header, record 30 byte và số thực; báo riêng record không hữu hạn, không thay tọa độ bằng 0. Giữ scale âm khi hữu hạn thay vì clamp.
- Resolver Lorencia đối chiếu mọi range trong `native/client/assets.cpp::lorencia_model_name`. Các map khác dùng adapter ObjectN/Object(type+1) đã có ở native; ánh xạ/material của toàn bộ map ngoài native vẫn cần source và visual reference.
- Dựng mesh theo Euler Z*Y*X, scale, vị trí source chia100, giữ hệ tọa độ terrain. Gộp theo texture/material, đọc OZT alpha và một số BlendMesh policy đã có ở native.
- Đọc TerrainLight.OZJ và áp màu đỉnh của terrain. Nhà/cây/object tĩnh được nạp quanh vùng xem, có tính radius của model lớn; giới hạn một triệu vertex. Đây chưa phải streaming toàn map, mọi object animation, roof alpha, sky/water/particle hoặc đầy đủ material gốc.
- Asset có hash baseline và hash trả về từ helper. Queue dùng chung request trùng đường dẫn, tối đa3 request asset đồng thời, giới hạn hàng đợi; không dồn32NPC request vào helper cùng lúc.

### Vòng đời NPC

`GameServer/Viewport.h`, `native/client/viewport.cpp` là nguồn layout/state. Parser hỗ trợ C2:13 spawn và effect bytes, C1:14 destroy, D4/15 vị trí, 17 death; mask index14bit, giới hạn1024 entity và từ chối record bị cắt/đuôi thừa.

Đã sửa trường hợp C2 có byte thấp của length bằngD4 bị nhầm thành ACK di chuyển của chính player. Snapshot của NPC không thay vị trí player authoritative.

`ActorAnimation` trong core/WASM bind nhiều phần vào skeleton chính, kiểm tên/parent/dummy của bone. NPC idle dùng nội suy theo thời gian; cập nhật tối đa25Hz. Bộ đệm transform được dùng lại. Layer NPC vẽ sau world mà không xóa depth buffer của terrain/player, dispose rig/GPU khi despawn, đổi map hoặc đóng; hủy upload trễ và không retry asset lỗi liên tục theo mỗi packet.

Render tối đa32 NPC trong layer hiện tại; nếu vượt ngân sách có cảnh báo. Không render Player khác, talk/shop/warehouse/combat/sound hoặc toàn bộ buff/action NPC trong đợt này.

### Kiểm chứng W0063

- 128 test client và16 test helper/gateway qua. Có test multipart rig, binding mismatch, transform, resolver, viewport, stale upload, dispose và queue.
- Browser offline: Lorencia146,110 có195object và3NPC preview; Noria179,126 có482object và4NPC preview, gồm Cherry Blossom. Giữ cảnh báo Object4/Object44.bmd vàNPC257.
- Browser online DKTEST: bảng NPC server đổi từ2 sang3 khi nhân vật di chuyển, có NPC trong ảnh và không báo lỗi asset trong vùng quan sát. Không tự coi số counter là tất cả NPC đều nằm trong frustum.
- Ảnh: `/tmp/mu-w63-online-npc.png`, `/tmp/mu-w63-noria-resources.png`. Mẫu FPS ngắn khoảng60 không phải benchmark dài hay cam kết60FPS.

## W0064 — phần tiếp sau checkpoint

### NPC dùng thân Player

Đã thêm helper371/414 theo nhánh `case258/371/414` trong `file thieu/ZzzCharacter.cpp`. Generator kiểm nguyên pattern giáp+9 và vũ khí-1 trước khi xuất binding; chỉ class có TypeNPC trong MonsterList mới được đăng ký.

`OpenPlayers` trong ZzzOpenData ánh xạ armor+9 sang Helm/Armor/Pant/Glove/BootMale10. Model Player.bmd chỉ cung cấp skeleton, thêm năm phần giáp. Dùng male idle action1 và scale0,9 theo nhánh game/default class0 của source. Hiệu ứng PC-room riêng chưa được triển khai.

Core `ActorAnimation` nhận action tùy chọn, vẫn giữ constructor cũ action0. Fixture hai action có vị trí bone khác nhau xác nhận action1 thật sự được chọn và multipart theo đúng pose, không chỉ có metadata đổi.

### Quarrel407

Nguồn `ZzzOpenData.cpp` gọi AccessModel trong Npc nhưng **OpenTexture trong Monster**. Generator bây giờ giữ texture directory tách khỏi model directory; không copy texture vào Npc hoặc lấy texture ngẫu nhiên.

`ZzzObject.cpp` có BodyLight(0.5,0.5,0.8), đã áp tint tương ứng. Source còn vẽ thêm chrome/bright overlay; phần chrome chưa làm và UI báo rõ.

### Raul546

BMD tham chiếu7 texture không nằm trong Npc nhưng có file gốc cùng tên ở Effect/Item/Monster/Skill. `npc-shared-textures.mjs` chỉ cho phép các binding cụ thể của đúng model Raul, pin SHA256 và kiểm mọi bản sao được liệt kê. Ba texture có hai bản sao giống từng byte; các texture một bản cũng phải khớp baseline. Không search/fallback tùy ý cho model khác, không sửa Data gốc.

Đây là **adapter tra cứu shared asset của web**, chưa chứng minh cơ chế lookup của engine Windows. Cảnh báo này được giữ ở UI và metadata.

`ZzzObject.cpp` vẽ mesh4/14 với RENDER_BRIGHT; generator xác nhận pattern source và layer dùng additive cho đúng ordinal mesh. Các mesh còn lại giữ vật liệu thường.

### Kiểm chứng W0064

- **130/130 test client qua** trên bản copy tạm, log `/tmp/mu-w64-tests.log`. Không chạy test build lên output đang phục vụ người dùng. Các test bổ sung kiểm action được chọn, material/tint, model scope và hash validation của shared texture.
- WASM đã rebuild; Debug64/Release31 file, stage179asset preview và profile local. Chỉ sau test mới đưa output đã chuẩn bị sẵn vào web/dist; bản trước giữ tại `web/.build/deploy-backups/W0064-*`. Không restart MU server/gateway hoặc sửaDB.
- CPU:47patch môi trường tại tọa độ catalog có geometry hữu hạn; báo cáo vẫn phân biệt map thiếu terrain texture. Điều này không xác nhận toàn bộ scene/coordinate/UV/alpha đúng.
- Browser Devias212,63:188object,4NPC preview; helper371 hiện bộ giáp và idle. Hai record object không hữu hạn vẫn được báo, cùngNPC247/257 chưa có binding. Ảnh `/tmp/mu-w64-devias-helper.png`.
- Browser Crywolf62,239:186object, Quarrel nạp được; UI báo thiếu chrome và texture bons ở một số object. Ảnh có occlusion bởi môi trường, **chưa xác nhận nhìn rõ toàn thân hoặc visual parity**. Ảnh `/tmp/mu-w64-quarrel.png`.
- Browser map79 tại131,109:160object, Raul hiện trong cảnh, có cảnh báo shared lookup; vòng frame chạy. Ảnh `/tmp/mu-w64-raul.png`.
- Chưa live test NPC371/407/546 trên map online trong đợt này. Không dùng preview cấu hình thay bằng chứng packet/live interaction.

## NPC trong cấu hình còn thiếu

| Class | Tên từ server | Map ID |
|---|---|---|
| 236 | Golden Archer | 0 |
| 257 | Shadow Phantom Soldier | 0, 2, 3, 51 |
| 249 | Guard | 0 |
| 375 | Chaos Card Master | 0 |
| 247 | Guard | 2 |
| 522 | Jerint the Assistant | 2 |
| 217 | Crown Switch2 | 30 |
| 218 | Crown Switch1 | 30 |
| 216 | Crown | 30 |
| 220 | Gatekeeper | 30 |
| 223 | Senior | 30 |
| 367 | Gateway Machine | 38 |
| 381 | MU Allies General | 45, 46, 47, 48, 49, 50 |
| 382 | Illusion Sorcerer Elder | 45, 46, 47, 48, 49, 50 |
| 383 | Alliance Sacred Item Storage | 45, 46, 47, 48, 49, 50 |
| 384 | Illusion Castle Sacred Item Sto | 45, 46, 47, 48, 49, 50 |
| 385 | Mirage | 51 |
| 465 | Santa Claus | 62 |
| 467 | Snowman | 62 |
| 468 | Dasher | 62 |
| 469 | Kermit | 62 |
| 470 | Dancer | 62 |
| 471 | Cupid | 62 |
| 472 | Prancer | 62 |
| 473 | Donner | 62 |
| 474 | Vixen | 62 |
| 475 | Blitzen | 62 |

Nhóm tiếp theo là Guard247/249, Shadow Phantom257 và các NPC dùng Player/trang bị; sau đó các NPC sự kiện có source được gọi qua module khác. Không thay chúng bằng model type+1 hoặc NPC giả. Còn9map thiếu tile mapping, material/effect của nhiều object và các reference model chưa giải quyết.

## Tái lập

```sh
python3 web/tools/build_actor_catalog.py --source '/Users/angelale/Downloads/SOURCE-2.0.7/file thieu' --server /Users/angelale/Downloads/MuServerEX603/Data --output web/client/src/world/actor-catalog.json
node web/tools/audit_world_resources.mjs /Users/angelale/Downloads/MuClientEX603/Data web/client/src/world/resource-catalog.json
node web/tools/check_world_scenes.mjs /Users/angelale/Downloads/MuClientEX603/Data web/docs/world-resources-W0064.json
```

Để xem helper: `?scene=terrain-check&map=2&x=212&y=63&npc=1`. Raul: `?scene=terrain-check&map=79&x=131&y=109&npc=1`. Đây là chế độ kiểm resource offline, không thay đổi nhân vật online.

## W0065 — Lorencia ổn định và mái nhà (2026-10-26)

- Bỏ vùng nạp object 22 ô và terrain 14 ô riêng map0. Nạp toàn bộ terrain 255×255 cell và mọi instance có mapping sẵn; giới hạn environment tăng từ 1 triệu lên 5 triệu vertex riêng Lorencia. Dữ liệu đo được: 2.787 instance có model, 1.316.121 vertex. Chia batch theo chunk 16 ô, texture, material và roof; culling khi vẽ dùng bounds thật của geometry và vùng camera ±40 ô, không bỏ object trong giai đoạn nạp.
- Lỗi tải/decode/upload dependency của object đã ánh xạ làm scene báo chưa hoàn tất; không nuốt lỗi rồi hiện cảnh thiếu. Fetch retry tối đa 3 lần cho network/429/503, backoff tối đa 5 giây; 404 và hash sai không retry. Giữ nguyên hash/budget, allowlist, realpath confinement và 4 request đồng thời. Tách quota asset 12.000/phút khỏi identity/keys 600/phút, trả Retry-After cho429. Không tuyên bố tải thành công thiếu object.
- Source bổ sung `file thieu/ZzzObject.cpp` chọn HouseWall01+4/+5 (type125/126), `ZzzInterface.cpp` lấy HeroTile từ TerrainMappingLayer1, tile4 ở trong nhà. `ZzzAI.cpp` AlphaEnable=false nội suy alpha += (target-alpha)*.1. Web dùng tương đương theo thời gian 25Hz; roof pass sau player/NPC, alpha blend và không ghi depth khi đang fade. Khi khởi tạo ngay trong nhà, mái bắt đầu ẩn. Các mesh tường dù chung texture không gộp với mái.
- Lorencia cho đi bộ toàn map theo flags/server ACK; map khác giữ giới hạn 12 ô vì còn nạp từng vùng.
- Chưa parity 100%: type168 chưa có mapping source, effect placeholders130..133 chưa particle, animation/material/sky chưa đầy đủ. Không tự đoán model168.
- Validation: 132 test client pass trong bản sao cô lập; build/stage và thay thư mục dist sau khi hoàn tất, không xóa runtime đang phục vụ. Test mới kiểm tra alpha 25/60/120FPS, khởi tạo indoor, map khác không fade, retry429 và không retry404.
- Browser kiểm tra thực tế: tại125,125 trong quán bar render2.787 object,60FPS và mái đã ẩn. Refresh trước khi sửa helper tái hiện lỗi Tree12.bmd do quota; đã sửa quota asset riêng và restart đúng helper9001. Gateway16/16 test pass với socket loopback. Thêm test roof pass xác nhận tường vẫn ở opaque pass, mái fade không ghi depth, opacity0 không vẽ; 8/8 test render/fade/fetch pass.

## W0066 — phần bổ sung cho movement/combat

Giữ63NPC binding trước đó; thêm4monster binding nguồn rõ2/3/11/12 với clip0/2/3/4/6 và hit-height đo từ geometry. Thêm hash/decode audit model MoveTargetPosEffect và texture cursorpin01/02, texture phụ thuộc model. Catalog không đồng nghĩa full monster coverage. Test/nguồn/browser evidence/gap tại `MOVEMENT-COMBAT-W0066.vi.md`; client143/143 +WASM contracts pass. Online Spider#61 chết với damage78 do server xác nhận; marker đã thấy bằng resource gốc trong OFFLINE simulation riêng.

## W0067 — bóng cho model đang render

World batch giữ origin riêng từng object để chiếu bóng theo source CalcShadowPosition; body/equipment/NPC/monster dùng pose hiện tại. Một terrain height texture/map, stencil tránh cộng đen và không cast effect/additive/multiply; roofẩn thì shadowẩn.146/146 clienttests qua; Lorencia actor vàNPCfixture visual ~60FPS OFFLINE. Xem `SHADOWS-W0067.vi.md` cho thuật toán và giới hạn; missing bindings/resources không có bóng giả.
