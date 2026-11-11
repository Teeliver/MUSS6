# Audit asset W0002 — 2026-10-23

Đã chạy offline bằng `assets.cpp` và `asset_images.mm` của client native, không sửa Data hoặc khởi động server. Đây là kiểm tra khả năng đọc của implementation hiện tại, chưa phải xác nhận tương đương engine Windows.

| Phạm vi | Kết quả |
|---|---:|
| Model có magic BMD | 5.034 |
| Model parser đọc được | 5.025 |
| Model parser báo lỗi | 9 |
| OZJ/OZT đọc được pixel | 7.278 |
| OZJ/OZT parser báo lỗi | 8 |
| BMD không mang magic model, chưa audit | 149 |
| Tham chiếu texture chưa resolve trong thư mục model | 143, thuộc 103 model |

Chi tiết từng lỗi và từng dependency: [asset-audit-W0002.json](asset-audit-W0002.json). Báo cáo đầy đủ mỗi file nằm cục bộ tại `web/.local-baselines/2026-10-23-m00-002/assets.jsonl`; SHA-256 được ghi trong JSON tổng hợp.

Trong Object52, audit xác nhận lại lỗi decoder ở Object90 và các tham chiếu cục bộ chưa resolve: sword02, shield02, shield03, sword08, medicine1, medicine2, bottle. Không kết luận file gốc hỏng: cần đối chiếu loader, shared texture và fallback của source. Audit này không kiểm tra model bị thiếu hoàn toàn như Object44/Object166 vì chưa đọc danh sách object của map.

Parser kiểm tra cấu trúc mesh, index vertex/normal/UV, parent skeleton, số action/key và kích thước layout. Audit không giữ clip trong RAM: parser bỏ qua float của animation không được giữ. Vì vậy **chưa xác minh toàn bộ giá trị animation, tốc độ hoặc pose**. Model đọc được cũng chưa chứng minh render đúng.

Chưa audit terrain, placement, table BMD, OZB, JPG/TGA thô, WAV, font hoặc browser. Resolver giữ hành vi tìm cùng thư mục của native; chưa mô phỏng chính sách dùng chung resource của engine gốc. Chưa đóng M00.3b.

Tái lập từ Emulator trên macOS, cần Command Line Tools:

```sh
mkdir -p web/.local-baselines/2026-10-23-m00-002
clang++ -std=c++17 -O2 -fobjc-arc -I native/client web/tools/asset_audit.mm native/client/assets.cpp native/client/asset_images.mm -framework Cocoa -framework ImageIO -o web/.local-baselines/2026-10-23-m00-002/asset-audit
web/.local-baselines/2026-10-23-m00-002/asset-audit /Users/angelale/Downloads/MuClientEX603/Data > web/.local-baselines/2026-10-23-m00-002/assets.jsonl
python3 -m unittest discover -s web/tests -v
```

Kiểm thử W0002: 4/4 qua, gồm baseline và fixture model hợp lệ, model bị cắt, parent sai, table không nhầm thành model, texture hỏng, dependency thiếu, bỏ qua symlink và nguồn không đổi. Test audit được skip nếu chưa build CLI; phải build trước để có bằng chứng tích hợp.
