# Bảng trạng thái 47 map — đợt 7

“Terrain tải được” là catalog + decode/CPU geometry; không xác nhận đủ hiệu ứng hay gameplay server. Các placement lỗi vẫn được giữ nguyên.

| Map | Terrain | Placement chưa giải quyết | Lỗi terrain |
|---:|---|---:|---|
| 0 | loadable | 1 | — |
| 1 | loadable | 0 | — |
| 2 | loadable | 1 | — |
| 3 | loadable | 1 | — |
| 4 | loadable | 1 | — |
| 6 | loadable | 0 | — |
| 7 | loadable | 0 | — |
| 8 | loadable | 49 | — |
| 9 | loadable | 0 | — |
| 10 | blocked | 0 | Missing/ambiguous texture World11/TileGrass02.OZJ; Missing/ambiguous texture World11/TileRock01.OZJ; Missing/ambiguous texture World11/TileRock02.OZJ; Missing/ambiguous texture World11/TileRock03.OZJ; Missing/ambiguous texture World11/TileRock04.OZJ |
| 11 | loadable | 9 | — |
| 18 | loadable | 0 | — |
| 24 | loadable | 0 | — |
| 30 | loadable | 1 | — |
| 31 | loadable | 0 | — |
| 33 | loadable | 0 | — |
| 34 | loadable | 2 | — |
| 37 | loadable | 0 | — |
| 38 | loadable | 22 | — |
| 39 | blocked | 0 | Missing/ambiguous texture World40/TileWood01.OZJ; Missing/ambiguous texture World40/TileRock05.OZJ |
| 40 | loadable | 8 | — |
| 41 | blocked | 0 | Missing/ambiguous texture World42/TileRock05.OZJ |
| 42 | loadable | 0 | — |
| 46 | loadable | 3 | — |
| 51 | loadable | 5 | — |
| 54 | loadable | 0 | — |
| 56 | loadable | 3 | — |
| 57 | loadable | 65 | — |
| 58 | loadable | 13 | — |
| 62 | loadable | 6 | — |
| 63 | blocked | 1 | Missing/ambiguous texture World64/TileRock06.OZJ |
| 64 | loadable | 2 | — |
| 65 | loadable | 23 | — |
| 66 | loadable | 0 | — |
| 67 | loadable | 0 | — |
| 68 | loadable | 3 | — |
| 69 | loadable | 4 | — |
| 70 | loadable | 0 | — |
| 71 | loadable | 0 | — |
| 72 | loadable | 5 | — |
| 73 | loadable | 10 | — |
| 74 | loadable | 7 | — |
| 77 | loadable | 0 | — |
| 78 | loadable | 1 | — |
| 79 | loadable | 4 | — |
| 80 | loadable | 3 | — |
| 81 | loadable | 1 | — |

## Phân loại toàn bộ placement còn lại

243 placement thiếu model tại binding hiện tại; 9 thiếu/không phân giải được texture; 1 model lỗi decode; 1 type chưa có mapping. Đây là phân loại lỗi kỹ thuật, không phải chứng minh243 placement đều cần file mới. Nhánh source có thể cho thấy một số type là marker hoặc binding khác. Xem readiness.json cho từng map/type, đường dẫn và file cùng tên ở nơi khác. Các candidate cùng tên không được tự động sử dụng.
