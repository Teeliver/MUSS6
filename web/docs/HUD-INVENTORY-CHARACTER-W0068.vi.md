# W0068 — HUD gốc, Inventory và thông số nhân vật

2026-10-26. **PARTIAL cho toàn giao diện/gameplay**, có HUD bằng ảnh gốc, dữ liệu server, túi chính/ô trang bị và bảng C. Không tuyên bố parity100% hay full inventory/equipment gameplay.

## Nguồn và resource

- Đọc lại STATUS/KNOWN-GAPS và các milestone movement/resource/shadow trước. Layout đối chiếu `native/client/game_hud.mm`, `inventory_panel.mm`, `character_info.mm`, `item_placement.cpp`, `item_icons.mm`, `item_catalog.cpp`, `localization.cpp`, `inventory.cpp`, `client.cpp`. Nguồn server: `GameServer/Protocol.h/cpp`, `ItemManager.h/cpp`, `ObjectManager.cpp`.
- Bộ NewUI Windows tương ứng chưa có đầy đủ implementation trong thư mục bổ sung; không coi AppKit port là chứng minh pixel parity Windows. Dùng atlas gốc và tọa độ640×480/HUD, panel190×429 của native hiện có. Panel túi8×8 cell20px,12ô trang bị theo equipment_rect; khung C, viền bảng và nút cộng/đóng từ resource gốc.
-47ảnh/bảng đã hash và kiểm decode, manifest `client/src/ui/asset-manifest.js`, sinh bởi `tools/audit_ui_assets.mjs`. Gồm HUD01/02/03, bìnhHP/MP, SD/AG, EXP/digits/skill/buttons; khung inventory/cell/equipment/money; character textbox/level button/table corners; Item_eng và Text_eng.
- Helper chỉ cho đúng manifest, không cho tùy ý Interface hoặc mọi Local file. Giữ origin/host, hash, realpath confinement, quota/concurrency. Restart riêng helper9001; game server/gateway/DB/Data gốc không sửa.
- Decode Item84×8192+checksum, XOR FC/CF/AB, metadata tên/size/wearslot/requirements/classes. Decode Text GT,count/index/length/XOR, chặn trùng/truncated/NUL. Thực tế8.192item và3.258text decode được.

## Dữ liệu và transaction

- `protocol/hud-layout.js` sinh sizeof/offsetof **9struct thật** bằng clang++, UPDATE603/EXTRA1. Không tự đoán padding/endian. Test regenerate so artifact đúng source.
- `protocol/game-state.js`: F3:03/E0/E1,level5/point6,HP26/MP27,damage11/F3:07,reward9C,regenF3:04; HP/MP/SD/AG32bit, EXP/level,base+bonus,damage/defense/speed. Inventory C2:F3:10,modifyF3:14,dur2A/delete28/pick22/buy32/drop23/money33/34,skillsF3:11. State không tạo item/điểm giả khi chưa có packet.
- HUD hiện current/max qua tooltip/accessible label; số trên bình dùng newui_number1. Skill đã học hiện atlas; quick-cast/QWER dùng potion, chat/friends chưa nối. F9 bật/tắt Debug overlay; mặc định bỏ panel test lớn để thấy HUD gốc.
- I hoặc V mở Inventory; C mở thông số; Esc hủy chọn item/đóng bảng. Hover item có tooltip tên/level/durability/luck/skill/options/basic damage/defense. Đây chưa đầy đủ mọi option-derived stat/requirement/color/upgrade tooltip gốc.
- Icon lấy model/texture Data thật; armor bind Player action0, camera/rotation orthographic đối chiếu item_icons.mm, nền transparent. Dùng một WebGL canvas phụ và queue cache, không một context/item. +7..15 effect/chrome/pulse/hover rotation và mọi item-model alias chưa hoàn chỉnh; thiếu icon có nhãn thay thế, không fake hình.
- Túi chính hỗ trợ kéo thả và click nhấc→click ô trống. Kiểm đủ rectangle theo Item dimensions, biên/overlap. C3:24 19byte, SourceFlag0/TargetFlag0. Chỉ commit sau ACK24, kiểm type/target/originalwire. Timeout giữ transaction lock cho tới ACK/đóng để không gửi lặp gây lệch đồ. Không hỗ trợ drop xuống đất, exchange, shop, repair, use item hoặc warehouse trong đợt này.
- **Ô trang bị chỉ xem**, chưa kéo thay gear vì cần đồng bộ skeleton/material/attachment/body model runtime. Không cho UI đổi gear mà hình nhân vật giữ đồ cũ. Túi mở rộng có trang đọc theoExtInventory, chưa chuyển đồ; personal shop inventory chưa là panel hoàn chỉnh.
- Bảng C có nút cộng điểm: C1:F3:06 type0..4,Command chỉDL; chờ ACK và dữ liệu mới. Không optimistic decrement; timeout giữ lock chờ ACK/đồng bộ. **Không cộng điểm thật khi test phiên người dùng.** Đây là cập nhật persistent khi người dùng tự bấm, không có nút giả tăng điểm cục bộ.

## Kiểm thử và browser

- Test packet/state32bit, nguồnABI, decode/corruption, danh sách item trùng/invalidslot, chữ kýwire, ACKmove/failure/serialization/timeout, stat ACK, kích thước/overlap túi; renderer có inventory orthographic mode/transparent, preview/shadows cũ giữ regression tests.
- Gateway16/16pass, log `/tmp/mu-w68-gateway-tests.log`; client log `/tmp/mu-w68-tests.log`. Test/build trong temp copy rồi stage preview/profile và swap nguyên output, không xóa dist đang phục vụ.
- Offline `?scene=ui-check&map=0&x=140&y=125` có nhãn rõ **fixture không GameServer**, chỉ để test visual/các nút; không làm bằng chứng data online. Test nút cộng offline giữ Points10, báo không cộng điểm thật. Bản đầu icon dùng camera preview, phát hiện góc staff/shield không đúng; đổi sang hướng/camera inventory theo native, đã thấy Kris/potion/jewel/armor đúng khung. CSS nút cộng ban đầu che sprite, đã sửa để thấy nút + gốc.
- Online MuOptions do người dùng đăng nhập: Lv7,HP66/66,MP766/766,SD991/991,AG167/167,EXP5535/7840,Zen211.330.843; bảngC Points9399,Strength200,Agility120,Stamina15,Energy377,Damage61–79,Defense269(47),Speed31,Magic45–98. Inventory hiện Legendary+9 đang mặc và Bone+2/Kris/PadBoots trong túi đúng packet. Không thay số này để tạo ảnh.
- Ảnh online `/tmp/mu-w68-live-hud-inventory-character.png` là trước sửa cuối góc icon/nút cộng; ảnh OFFLINE `/tmp/mu-w68-offline-ui-final.png` dành visual bản cuối. Không dùng fixture làm dữ liệu nhân vật thật.
- Browser native drag thử Kris ô16→66 không tạo ACK/không đổi style vị trí; không kết luận đã chuyển đồ live thành công. Đã bổ sung click nhấc/đặt để tương thích browser; transaction mới có test socket giả lập, được kiểm chứng ACK live/hoàn trả trong phần bổ sung bên dưới.

## Giới hạn còn lại

Thay trang bị/body refresh; full expanded bag/shop/storage; full tooltip/upgrade shaders; potion/skill quickbar/chat/friends; buff/quest/master/event HUD; đủ localization/layout Windows/screen matrix và item catalog aliases. Dữ liệu hiển thị và các mutation được tách rõ, không tích full milestone.

### Build cuối trước kiểm ACK live

153/153 clienttests,16/16 gatewaytests pass; Debug80file/Release31file,179previewasset và authorized profile stage xong trước swap. Backup `web/.build/deploy-backups/W0068-20261026-161155`; log `/tmp/mu-w68-tests.log`. C3:24 header được sửa theo `native/client/inventory.cpp::item_move_packet` header.setE; click nhấc/đặt là bổ sung cho native HTML drag, không làm mutation trước ACK.


### Kiểm thử ACK online hoàn tất sau khi người dùng vào lại

- Trên MuOptions, đọc vị trí Kris hiện tại từ DOM là left55/top200,width20,height40, tương ứng **ô14**. Người dùng đã tự đổi một số ô trước đó; không dùng ô16 cũ làm vị trí hoàn trả.
- Click Kris → ô66: UI báo **Server đã xác nhận chuyển vật phẩm**, vị trí đổi sang left135/top320 (ô66, item1×2). Sau đó click Kris → ô14: ACK thành công, vị trí trở lại left55/top200. Không đổi trang bị/Zen, không bỏ item xuống đất, không cộng điểm.
- Sau hoàn trả: Points vẫn9399,Zen211.330.843,HP66/66,MP766/766,SD991/991,AG167/167. UI dùng bản cuối: góc staff/shield/armor và nút+gốc đã hiện đúng. Ảnh live cuối `/tmp/mu-w68-live-ui-verified.png`.
- Kết luận phạm vi: **click nhấc/đặt trong túi chính có ACK live và hoàn trả pass**. Native browser drag trước đó chưa phát raDrop; không gộp kết quả click thành bằng chứng drag pass. Equipment/túi mở rộng vẫn chỉ xem.
