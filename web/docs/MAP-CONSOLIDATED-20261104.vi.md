# Map — lượt sửa tổng hợp và hiện trạng toàn bộ — 04-11-2026

Tài liệu này thay cách chia tiếp thành các đợt8E/8F nhỏ. Phạm vi lượt này: các nhánh chung Empire Guardian69–74, ánh sáng gắn bone MarketPlace/Elbeland/Empire, giải mã texture và kiểm kê toàn47map. Không coi nạp được map là đạt100% bản PC.

## Đã sửa trong lượt này

| Nhóm | Thực hiện theo source |
|---|---|
|Empire69–74|GMEmpireGuardian1/2/3/4: marker79 một trong3 family HIK màu + glow cam;80 glow xanh pulse;82/83/84 WATERFALL5 subtype9,3 subtype14,2 subtype4;86/129/130 CLOUD21 với màu riêng;131 SMOKE22+21;132 hai SMOKE60+21. Xác suất lần lượt theo từng branch, không nhân đôi HIK1 của Raklion.|
|Animation/material|Empire type36=0,5keys/s;type64=16keys/s riêng72–74;type81 UV V+0,375/s. Elbeland56 mesh0 additive/pulse theo GMNewTown.|
|Ánh sáng theo bone|Market79 type30 bones1/2/3 và35 bone2;Elbeland51 type63 bone5 offset(-40,-10,0),121 bones3…8 offset(5,-4,-1);Empire72/73/74 type37 bone1. Texture Flare/flare01, màu, nhịp pulse, scale từng branch. Market có terrain light theo source.|
|BMD bridge|sampleBonesResult trả ma trận bone theo đầy đủ parent chain; tối đa64 bone/request, kiểm tra index/time. Áp scale/Euler/origin placement đúng một lần. Chia sẻ rig theo type, sample25Hz, giải phóng khi rời map.|
|Nước dùng chung|Sửa scatter lúc tạo WATERFALL3 subtype8/14, scale subtype14 phụ thuộc placement;WATERFALL5 subtype9 scale0,6+placementScale; fade/ramp brightness theo lifetime cho cả0/9. Hồi quy Market và Elbeland.|
|Resource thực tế|Flare.OZJ có48byte trailer khác0 sau EOI. Parser nay đọc segment/entropy JPEG, dừng đúng EOI thay vì bắt EOI ở cuối file; vẫn từ chối stream bị cắt. Không sửa file Data. Lỗi texture bone được báo và bỏ riêng effect, không chặn toàn map.|
|Pool|Bỏ cap256 sai cho CLOUD21/SMOKE22/60 và SMOKE21 Empire; tính theo emitter×lifetime, tính hai SMOKE60, giữ tổng layer≤3000. Bone/sprite pool≤1000 tách riêng. Có bộ đếm lỗi/dropped, dispose idempotent.|

Nguồn đối chiếu: `Main5.2/source/GMEmpireGuardian{1,2,3,4}.cpp`, `GMUnitedMarketPlace.cpp`, `GMNewTown.cpp`, `ZzzEffectParticle.cpp`, `ZzzOpenData.cpp`, `ZzzBMD.cpp`. Không áp các branch event có điều kiện vào mọi frame để làm đầy hình ảnh.

## Kiểm chứng

- WASM biên dịch lại thành công; build debug171file, kiểm tra graph import trước publish.
- **456/456 test web qua**. Toàn bộ test và test mới: xem [log cuối](map-consolidated-20261104/client-tests.log). Test mới dùng BMD thật của cả5 attachment family tại0/0,04/0,21/1/10/60giây; kiểm tra transform, request lỗi, dispatch6map, nước, trailerJPEG, cô lập lỗi và pool.
- Audit47map: **98.353 placement**,47/47terrain decode,43/47texture+CPU patch pass;254placement chưa giải quyết. Kiểm kê effect dispatcher trên đủ47map, không phải chứng nhận pixel parity.
- Browser offline kiểm tra đủ69/70/71/72/73/74,51,79. [Số đo từng vị trí](map-consolidated-20261104/browser-samples.json). Dùng Data thật: Market79 có20bone sprite/12glow/14water emitter, lỗi bone0;Empire72 có20bone sprite/120glow/57water emitter, lỗi bone0;Elbeland51 có162bone sprite/251water emitter, lỗi bone0. Các mẫu này dropped0.
- Empire69 tại155,54 phát hiện cap256 cắt cloud, đã sửa và kiểm tra lại:600particle/66sprite/90waterparticle, dropped0. Mẫu60FPS, CPU p958,9ms; không cam kết mọi vùng/máy60FPS.
- Mẫu Market60FPS CPU p955,4ms;Empire72 60FPS CPU p956ms;Elbeland60FPS CPU p955,7ms. Đây là mẫu đơn lẻ buffer2560×1440, không phải benchmark dài hạn hoặc gameplay online.

[Market](map-consolidated-20261104/market.png) · [Empire](map-consolidated-20261104/empire.png) · [Elbeland](map-consolidated-20261104/elbeland.png)

## Danh sách còn vướng chung — chưa được đánh dấu hoàn tất

### Tài nguyên/binding

|Map|Terrain chưa nạp đủ do thiếu/ambiguous|
|---:|---|
|10 Icarus|World11/TileGrass02.OZJ,TileRock01…04.OZJ|
|39 Kanturu3|World40/TileWood01.OZJ,TileRock05.OZJ|
|41|World42/TileRock05.OZJ|
|63|World64/TileRock06.OZJ|

254placement gồm243thiếu model ở binding hiện tại,9texture thiếu/ambiguous,1model/decode không hợp lệ,1type chưa mapping. Đây là phân loại kỹ thuật: chưa khẳng định tất cả243là file phải bổ sung, vì một số có thể là marker hoặc mapping khác cần chứng minh bằng source. Không tự lấy file trùng tên từ map khác.

Chi tiết từng map/type/path: [readiness](map-consolidated-20261104/readiness.json), [coverage](map-consolidated-20261104/map-coverage.json), [terrain](map-consolidated-20261104/terrain.json).

### Hiệu ứng/source chưa port hết

- Cập nhật sau lượt tổng hợp: marker85 FLAME6, khói máy64, rate20/122/123/124/128 và một nhóm vật liệu đã được xử lý trong [báo cáo hiệu ứng có vòng đời](MAP-EVENTS-20261104.vi.md). Joint spark, fire theo bone, chrome, event/action và render-after-character còn lại chưa hoàn tất.
- Lượt tiếp theo đã phục hồi50emitter lửa Elbeland và kiểm tra bone recipe trên47map; type50/map72 bị chặn vì source cần bone2–7 nhưng BMD chỉ1bone. [Báo cáo](MAP-BONE-VALIDATION-20261104.vi.md).
- Các controller khác vẫn còn effect theo bone/joint/event, thời tiết và nhánh trạng thái chưa đối chiếu đầy đủ. [Inventory34file source](map-consolidated-20261104/source-effect-inventory.json) đếm call-site văn bản; không lấy số call-site làm phần trăm hoàn thành.
- [Inventory effect47map](map-consolidated-20261104/effects.json) ghi phần dispatcher đang có; số0 không đồng nghĩa bản PC không có effect. Các effect legacy khác không nằm trong bảng này.
- Terrain lighting web vẫn giới hạn8đèn gần nhất; pool web tách khỏi skill/character, chưa đồng nhất global particlepool của PC. Chưa chứng nhận pixel parity.
- Browser đã kiểm tra các vị trí đại diện, chưa kiểm tra mọi tile/góc nhìn và trạng thái event online của47map. Vùng phía trong công trình có thể bị che; không dùng ảnh tại đó để kết luận roof đúng.

## Vận hành và dung lượng

Không sửa DB/account/Data gốc hay restart stack. Đã publish bản debug mới; tab đang mở cần reload để nhận module/WASM mới. Duy trì một archive code `native/runtime/backups/mu-web-code-latest.tar.gz`, không sinh chuỗi backup đợt. Giữ backupDB riêng vì là dữ liệu khác. Cây test cô lập được dọn sau khi lưu log; không xoá resource/source gốc để giải phóng dung lượng.
