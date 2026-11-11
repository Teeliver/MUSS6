# W0088 — Tài nguyên, animation và hiệu ứng map

Trạng thái: đang thực hiện; chưa đạt toàn bộ hiệu ứng gốc.

## Đã triển khai

- Đọc toàn bộ terrain và object placement trên các map vật lý, bỏ giới hạn vùng quanh điểm xuất hiện. Chia terrain/object thành khối và loại ngoài camera khi vẽ GPU.
- Animation clip 0 cho model có chuyển động; tốc độ riêng theo ZzzObject, GMNewTown và MapManager (Elbeland, Ice City, Karutan). Chưa xử lý đầy đủ trạng thái cửa, operate và chuyển action.
- Một số nhánh UV nước của map cũ; lửa Fire01 và ánh sáng địa hình cho các loại vật thể đã đối chiếu trong fire-policy.js.
- Elbeland: marker 54/58/59 dùng WATERFALL2/5/3 và các bước particle theo GMNewTown/ZzzEffectParticle. Có giới hạn số hạt; chưa phải toàn bộ nhánh waterfall của tất cả map.
- Bộ nhớ asset có LRU trong RAM; không bật cache HTTP lâu dài.

## Bằng chứng và giới hạn

Audit 47 map vật lý chứa 98.353 placement. 801 placement thuộc 75 cặp map/type chưa giải quyết: một phần là marker hiệu ứng, một phần thiếu model/texture hoặc lỗi giải mã. Không được coi tất cả là model cần thay thế. Chi tiết: MAP-COVERAGE-W0088.json.

MAP-EFFECT-SOURCE-W0088.json ghi vị trí gọi hiệu ứng trong source, không chứng minh đã port các lệnh đó. Còn thiếu các sprite/joint, ánh sáng theo bone, chuyển động môi trường đặc thù và render nhiều lớp/chrome của từng map. Ánh sáng hiện giới hạn 8 nguồn gần camera; chưa khớp nội suy terrain gốc hoàn toàn.

## Kiểm tra trình duyệt

Lorencia (120,87): 2.787 object, FPS 60, frame p95 17,5 ms ở lần kiểm tra offline. Elbeland (30,219): sau fallback có 4.639 object, còn 4 nhóm cảnh báo resource; lần kiểm tra offline đạt khoảng 60 FPS. Đây là mẫu tại hai vị trí, không phải cam kết FPS cho mọi map/máy.

## Kiểm tra tự động

243 test qua trong bản W0088, gồm placement xa spawn, hướng UV, tốc độ animation riêng, vòng đời lửa/thác nước và culling giao camera. Build debug/release và stage Data thành công. Test không thay thế đối chiếu hình ảnh từng map.

## Danh sách theo map

- Bổ sung fallback texture theo CGMModelManager::OpenTexture → FindTextureByName: khôi phục 19 model bằng file cùng tên đã kiểm SHA-256 đồng nhất giữa mọi bản tìm được. Giữ cảnh báo nếu tên trùng nhưng nội dung khác; không suy đoán thứ tự load gốc.

| Map | Placement | Chưa giải quyết | Loại có clip động |
| --- | ---: | ---: | ---: |
| 0 | 2865 | 1 | 19 |
| 1 | 4488 | 9 | 6 |
| 2 | 2235 | 1 | 12 |
| 3 | 9399 | 1 | 22 |
| 4 | 5380 | 1 | 7 |
| 6 | 1485 | 0 | 12 |
| 7 | 5215 | 0 | 12 |
| 8 | 3381 | 49 | 9 |
| 9 | 564 | 0 | 2 |
| 10 | 670 | 0 | 3 |
| 11 | 210 | 9 | 5 |
| 18 | 194 | 0 | 0 |
| 24 | 2266 | 0 | 6 |
| 30 | 3724 | 1 | 13 |
| 31 | 7789 | 0 | 5 |
| 33 | 3752 | 0 | 19 |
| 34 | 6644 | 294 | 9 |
| 37 | 3645 | 0 | 20 |
| 38 | 2014 | 26 | 8 |
| 39 | 1459 | 0 | 6 |
| 40 | 670 | 8 | 9 |
| 41 | 1531 | 0 | 14 |
| 42 | 354 | 0 | 6 |
| 46 | 1964 | 3 | 5 |
| 51 | 5136 | 21 | 18 |
| 54 | 194 | 0 | 9 |
| 56 | 4176 | 226 | 10 |
| 57 | 1474 | 65 | 17 |
| 58 | 162 | 13 | 5 |
| 62 | 679 | 6 | 7 |
| 63 | 2290 | 1 | 7 |
| 64 | 1347 | 2 | 1 |
| 65 | 421 | 23 | 7 |
| 66 | 253 | 0 | 3 |
| 67 | 373 | 0 | 8 |
| 68 | 433 | 3 | 9 |
| 69 | 836 | 4 | 7 |
| 70 | 729 | 0 | 6 |
| 71 | 748 | 0 | 8 |
| 72 | 1670 | 5 | 12 |
| 73 | 375 | 10 | 9 |
| 74 | 112 | 10 | 2 |
| 77 | 628 | 0 | 7 |
| 78 | 96 | 1 | 4 |
| 79 | 517 | 4 | 7 |
| 80 | 2086 | 3 | 23 |
| 81 | 1720 | 1 | 13 |
