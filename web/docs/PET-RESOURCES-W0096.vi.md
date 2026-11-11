# W0096 — Pet từ Data gốc và source

Đọc Data/Local/Pet.bmd bằng đúng header/ver100, BuxConvert theo từng row, checksum0x7F1D; generator build_pet_catalog.mjs tạo pet-catalog.js kèm SHA256. Không dùng chung scale0.7 cho các pet có bảng cấu hình.

| Pet | Model gốc | Scale/nhánh |
|---|---|---|
| Guardian Angel | Player/Helper01.bmd | 0.7 |
| Imp | Player/Helper02.bmd | Satan gắn bone34; cùng scale owner |
| Demon | Item/partcharge4/demon.bmd | 0.2 |
| Spirit of Guardian | Item/partcharge4/maria.bmd | 0.4 |
| Pet Rudolf | Item/xmas/xmas_deer.bmd | 0.8 |
| Pet Panda | Item/PandaPet.bmd | 0.8 |
| Pet Unicorn | Item/partcharge7/pet_unicorn.bmd | 0.6 |
| Pet Skeleton | Item/skeletonpet.bmd | 0.4 |

## Satan/Imp

ZzzOpenData.cpp:918 nạp Player/Helper02.bmd; texture mở từ Item, gồm satan.OZJ và satan2.ozt. Các đường dẫn, kích thước và SHA256 được kiểm tra trong PET-RESOURCES-W0096.json. Không thay bằng Demon(partcharge4/demon.bmd).

ZzzCharacter.cpp:14911–14968: bone34, offset(20,0,0), tốc độ0.5 mỗi tick25Hz =12.5keys/s. RF giáp59/60/61/73/83 dùng offset(20,-5,20); đã nối cả hình khởi tạo và BodyAnimation cập nhật. Giữ ma trận bone theo nguồn, không lật model180 theo cảm tính.

Sau khi người dùng xác nhận pet đi đúng, Satan vẫn có cánh ngược/đầu bị che: đã bù chiềuV riêng cho satan2.tga; đây là sửa upload texture/mặt nạ, không đổi phép gắn vai. Asset thân satan.OZJ chỉ16x16 trong bộ Data được cung cấp.

## Các helper pet khác

Bỏ rx180 khỏi pet-layer. Tỷ lệ/blend/tốc độ lấy từ Pet.bmd. Hover height dựa w_PetActionStand/Demon(230×ownerScale), Unicorn(200×ownerScale), Skeleton(50×ownerScale), Rudolf/Panda(+20units). Đổi hướng theo CreateAngle, trục trước native -Y. Người dùng xác nhận pet đi đúng sau bản sửa hướng.

Đã xem8pet với main và remote; thử tháo Skeleton: số pet tự do2→1, đeo lại1→2; không nhân đôi. Satan là attachment nên không tính trong petActors. Kiểm tra RF giáp59 bằng fixture riêng.

## Giới hạn cần tiếp tục

Follow/orbit mới dùng khoảng cách và vận tốc theo source nhưng chưa port hết FindZen/collection state, random Angel, hành vi attack/skill và sprite/particle của mọi pet. Đã nối sprite đỏ BITMAP_LIGHT bằng Effect/flare01.OZJ, tại bone34 offset(20,0,15), RF(20,-5,35), scale1.5, màuđỏ ngẫu nhiên.35..495 theo nguồn. Anchor lấy từ pose bone hiện tại trong WASM, chuyển cùng owner và gỡ khi tháo helper; dùng cả main/remote. Thú cưỡi Uniria/Dinorant/DarkHorse/Fenrir và Dark Raven dùng controller/model riêng; không dùng model item trong túi để giả làm model cưỡi. Chưa chứng nhận toàn bộ hành vi/FX100%.

Kiểm tra trước sửa UV Satan cuối:261 test client qua,22 test mục tiêu qua, debug/release/WASM đã build. Audit8model và toàn bộ texture gốc đọc được,6mẫu animation mỗi model không có tọa độ lỗi. Chưa test đeo/tháo trên phiên online sau code cuối.

## Chốt bản kiểm tra Satan

262/262 test client qua sau UV riêng Satan và sprite đỏ; WASM, debug/release đã phát hành. Đã xem các hướng1/2/5/7 trong fixture; không xoay model180. Tháo/đeo Satan tăng actorEquipmentRevision0→1→2; khi tháo main không còn pet/đốm đỏ, remote vẫn giữ pet/đốm đỏ. Ảnh: /tmp/mu-w96-satan-facing-camera.png, /tmp/mu-w96-satan-removed.png. Người dùng xác nhận pet đi đúng, nhưng chưa xác nhận riêng hình Satan sau bản UV/đốm sáng cuối. Không đánh dấu bản cuối đã được người dùng duyệt.
