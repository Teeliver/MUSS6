# W0062 — Menu M và kiểm tra 47 World (2026-10-26)

## Đã triển khai

- Menu **M**, nút **Map (M)** và phím **Esc**. Menu không bắt phím khi đang nhập form; đóng menu trả focus về canvas. Đi bộ bị khóa trong lúc menu mở.
- **39 điểm chuyển online** đọc từ `Move.txt` và `Gate.txt` của bộ MuServerEX603 đang chạy. Hiển thị tên, map ID, level và Zen theo bảng. Server tiếp tục kiểm tra điều kiện; không sửa DB, tăng level hay tắt kiểm tra trang bị để vào map.
- **47 link kiểm tra Data offline**, tương ứng 47 thư mục World có thật. Link mở trang riêng, không gửi packet và không đổi vị trí nhân vật online. Có ô X/Y từ 0 đến 255 để xem từng vùng terrain quanh tọa độ. Không có nhân vật/NPC giả trong trang này.
- Đọc và hash height, mapping, attributes, texture tham chiếu, object placements và TerrainLight. Đã quét **98.362 record placement**, ghi nhận 2 giá trị không hữu hạn và 4 scale âm, không tự thay chúng bằng 0 hoặc clamp. Chưa render các object này.
- **47/47 terrain giải mã được**; **38 map** có bộ texture theo adapter hiện tại và CPU geometry hợp lệ. **9 map** còn ánh xạ texture cần đối chiếu. Menu giữ cả 9 map và báo lỗi cụ thể; khóa chuyển online tới scene chưa đủ tài nguyên.
- Dời entry HUD xuống dưới Debug overlay để giảm lỗi chồng chữ W0061.

Catalog: `web/client/src/world/map-catalog.json`. Công cụ tạo: `web/tools/build_map_catalog.mjs`. Catalog chỉ chứa metadata dùng cho client và hash đầu vào, không chứa credential hay cấu hình DB.

## Nguồn và các quyết định

### Height

Nguồn bổ sung: `file thieu/ZzzLodTerrain.cpp`.

- `OpenTerrainHeight`, quanh dòng 877, đọc wrapper 4 byte rồi header 1080 byte, nên pixel bắt đầu ở **offset 1084**. Hàm này không validate header BMP. Vì vậy không chặn World7/11/19/42/55 chỉ vì chúng khác BMP chuẩn. Height bằng byte × 1,5 đơn vị source; riêng login world 55 dùng × 3. Chuyển sang tile unit bằng cách chia 100.
- Hai file thiếu 2 byte cuối vẫn dùng quy tắc padding bằng byte cuối như adapter trước đây. Đây là sai khác xác định được; không sao chép việc đọc bộ nhớ chưa khởi tạo của source.
- `OpenTerrainHeightNew`, quanh dòng 974, đọc 24 bit từ **offset 58**, ghép BGR thành integer RGB rồi cộng `g_fMinHeight = -500`. Áp cho World43/64/67 (map42/63/66); fixture kiểm tra offset, thứ tự kênh và min-height.
- Source bổ sung chứa các nhánh cho nhiều phiên bản mới. Chưa có bằng chứng nó cùng build chính xác với EX603; không dùng việc decode thành công để kết luận visual parity.

### Chuyển map online

Đối chiếu `GameServer/Move.h`, `Move.cpp` (`Move`, `CGTeleportMoveRecv`), `Gate.cpp`, `User.cpp` (`gObjMoveGate`) và `Protocol.cpp` case 8E.

- Request **C1:8E**, 12 byte; trường `number` là WORD little-endian ở offset 8. Gửi **move index**, không dùng map ID thay gate/index.
- Reply **C3:1C**, 10 byte; map ở offset 6, X/Y ở 7/8 và hướng ở 9. Test lấy nguyên khai báo struct từ `Move.h`, compile bằng Clang với UPDATE603 để xác nhận sizeof/offsetof.
- Session chỉ có một yêu cầu chuyển map đang chờ, deadline 8 giây, reject và xóa timer khi đóng. Chỉ đổi map/tọa độ sau reply server; reply đến muộn vẫn là trạng thái authoritative. Sau reply, dựng lại terrain và actor.
- Server giữ quyền quyết định level, Zen, reset, loại account, trang bị, Gens, PK, interface và server đích. Tọa độ gate trong catalog chỉ chọn vùng xem offline; tọa độ online cuối cùng luôn lấy từ server.
- Chưa giải mã notice từ chối để hiển thị đúng thông báo gốc: hiện báo lỗi tổng quát khi hết deadline. Chưa hỗ trợ handoff sang GameServer khác; chưa cập nhật Zen HUD sau phí chuyển map (vẫn là snapshot lúc vào game).

### Texture và UI gốc

Index texture 0–13 theo adapter `native/client/world_view.mm`; World52 dùng AlphaTileGround01 ở index 2 theo adapter có sẵn. Fallback Noria → World1 giữ nguyên adapter trước và ghi rõ path/hash.

Folder bổ sung có `ZzzOpenData.cpp` tham chiếu `MoveCommandData`/`movereq.bmd`, nhưng chưa có đủ implementation `MoveCommandData`, `NewUIMove` và `MapManager::LoadWorld` cùng các tile policy riêng từng map. **Menu hiện tại là UI web mới**, dữ liệu chuyển online theo server. Phím M làm theo yêu cầu người dùng, chưa tuyên bố đã tìm đủ source binding/GUI gốc.

Thiếu ánh xạ theo adapter không đồng nghĩa Data gốc hỏng. Không tự chọn texture khác để che lỗi.

## Chín map cần đối chiếu

| Map ID / folder | Lỗi mapping hiện tại |
|---|---|
| 7 / World8 | Missing/ambiguous texture TileWater01.OZJ |
| 10 / World11 | Missing/ambiguous texture TileGrass02.OZJ; Missing/ambiguous texture TileRock01.OZJ; Missing/ambiguous texture TileRock02.OZJ; Missing/ambiguous texture TileRock03.OZJ; Missing/ambiguous texture TileRock04.OZJ |
| 39 / World40 | Missing/ambiguous texture TileGround02.OZJ; Missing/ambiguous texture TileWater01.OZJ; Missing/ambiguous texture TileWood01.OZJ; Missing/ambiguous texture TileRock05.OZJ |
| 40 / World41 | Missing/ambiguous texture TileWood01.OZJ; Missing/ambiguous texture TileRock04.OZJ |
| 41 / World42 | Missing/ambiguous texture TileRock05.OZJ |
| 42 / World43 | Missing/ambiguous texture TileRock05.OZJ |
| 46 / World47 | Missing/ambiguous texture TileWood01.OZJ |
| 63 / World64 | Missing/ambiguous texture TileRock05.OZJ; Missing/ambiguous texture TileRock06.OZJ |
| 67 / World68 | Missing/ambiguous texture TileWater01.OZJ |

## Kiểm chứng

- **121/121 test client**, **16/16 test gateway/helper** qua. Có kiểm ABI trực tiếp từ source, packet, malformed reply, request pending, timeout/late ACK, close, M/Esc/focus/typing, gửi move index và tách link offline khỏi yêu cầu online.
- Build Debug **58 file**, Release **31 file**; stage lại 179 asset preview và profile Debug. Menu/catalog/warp UI chỉ nằm trong Debug.
- Helper được restart riêng để nhận allowlist World1..255; giữ kiểm Origin, Host, loopback, realpath, size và traversal. Test chặn World0/256/leading zero/key/config. Không restart MU server/gateway hoặc ép logout phiên đang chơi.
- `map-terrain-W0062.json`: 47 terrain decode, 38 texture wrapper/CPU patch geometry pass; 9 map giữ lỗi mapping.
- `map-browser-W0062.json`: mở đủ 47 trang kiểm tra. **38 map chạy vòng vẽ và cập nhật FPS**, 9 map hiển thị lỗi texture. Đây là smoke test của terrain patch ở một tọa độ, không chứng minh toàn bộ map đúng. FPS ngắn quanh 60 không phải benchmark dài hoặc cam kết hiệu năng.
- Đã thử M trên browser offline và đọc đủ danh sách 47 World. Chưa có chuyển map online thành công trên nhân vật hiện tại. Fixture C3:1C không thay thế live test; 39 dòng Move.txt không phải 39 map đã chơi được.

## Giới hạn còn mở

- Chưa render placements, object models/materials, lightmap, sky, water animation, particles, NPC/quái và âm thanh cho 47 World. Vùng đen/cutout trong ảnh terrain chưa được xác nhận với scene gốc.
- 47 **thư mục vật lý** không đồng nghĩa toàn bộ **map logic** của server. Alias của event/instance dùng chung resource và chuyển server còn thiếu.
- Trang offline vẽ vùng bán kính 14 ô, có thể đổi X/Y để kiểm tra vùng khác; chưa vẽ toàn map cùng lúc hoặc có di chuyển/pathfinding offline.
- Chưa đủ GUI/font/localization/movereq của client gốc, chưa so frame với Windows. **Chưa đạt 100% parity**, M05/M06/M07/M11 vẫn PARTIAL.
- Bước tiếp: lấy đủ LoadWorld/tile policy để đóng 9 mapping; thêm object resolver và environment rendering; test gate/alias/handoff, lưu vị trí và visual regression theo từng map.

## Tái lập

```sh
node web/tools/build_map_catalog.mjs /Users/angelale/Downloads/MuClientEX603/Data /Users/angelale/Downloads/MuServerEX603/Data web/client/src/world/map-catalog.json
node web/tools/audit_map_terrain.mjs /Users/angelale/Downloads/MuClientEX603/Data web/docs/map-terrain-W0062.json
npm --prefix web test
npm --prefix web/gateway test
```

Mở `http://127.0.0.1:8080/?scene=terrain-check&map=0`, bấm M và chọn phần kiểm tra Data. Trang online cần tải lại để nhận module M mới; tải lại sẽ đóng phiên cũ. Không ép reload phiên người dùng khi triển khai.

### Bằng chứng bàn giao browser

- Đã nhập X/Y từ135,119 sang140,120 bằng form; URL và giá trị ô nhập cập nhật đúng, terrain nạp lại.
- Menu M đã mở, cuộn tới phần47World và lưu ảnh `/tmp/mu-w62-map-menu.png`, `/tmp/mu-w62-map-list.png`.
- Giữ tab kiểm tra offline để người dùng tiếp tục, không reload/đóng tab online có sẵn. Chưa dùng menu online để tiêu Zen/chuyển nhân vật trong lượt kiểm tra này.
