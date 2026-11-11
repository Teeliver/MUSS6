# Audit terrain, placement và table — W0003

Đã chạy offline ngày 2026-10-23 trên Data hiện có. Công cụ dùng trực tiếp parser native; không sửa production/Data, không khởi động server hoặc database.

## Kết quả đã xác minh

Trong 47 thư mục `WorldN`, parser hỗ trợ 5 world: 1, 4, 52, 74, 75 (server map 0, 3, 51, 73, 74). Cả 5 đọc thành công terrain mapping/attribute/height và danh sách object. Mỗi terrain có 65.536 mẫu height và attribute. Điều này xác minh khả năng đọc, chưa xác minh hình ảnh hay va chạm đúng Windows.

| World | Record object | Object có scale khác 0 | Kết quả mapping model native |
|---|---:|---:|---|
| 1 | 2.870 | 2.865 | Type 168 chưa có mapping; 4 type được native bỏ qua |
| 4 — Noria | 9.399 | 9.399 | Thiếu Object44, 1 placement; type 38 được native bỏ qua |
| 52 — Elbeland | 5.136 | 5.136 | Thiếu Object166, 1 placement; 7 type được native bỏ qua |
| 74 | 375 | 375 | 7 tên model chưa tìm thấy, 10 placement |
| 75 | 112 | 112 | 4 tên model chưa tìm thấy, 10 placement |

World1 khai báo 2.871 record nhưng chỉ có 2.870 record hoàn chỉnh; parser hiện tại chấp nhận sai khác +1 này. Không tạo record giả để bù. Tổng 357 cặp world/type: 331 model hiện diện, 13 tên model thiếu (22 placement), 12 type native bỏ qua và 1 type chưa ánh xạ. “Hiện diện” chỉ là kiểm tra file thường, không phải decoder/render pass; xem thêm W0002 cho lỗi nội dung model/texture.

42 world còn lại được ghi rõ `unsupported_by_native_parser`. Không sửa giới hạn parser để ép báo đạt. Cách đặt tên model và bỏ qua marker lấy từ `world_view.mm` hiện tại, chưa chứng minh là mapping đầy đủ của engine Windows.

Đã chạy parser Text, Item, Skill cho Eng/Por/Spn: **9/9 bảng đọc được**. Mỗi ngôn ngữ: Text 3.258 entry, Item 8.192 slot, Skill 650 slot. Số slot không phải số item/skill có tên và không chứng minh mọi option gameplay đã được hỗ trợ. Item/Skill qua kiểm tra layout/checksum của parser hiện có; Text qua kiểm tra signature, record, index trùng và byte thừa.

149 BMD không mang magic model của W0002 được phân loại:

- 9 bảng Text/Item/Skill đã decode trong W0003.
- 79 minimap xác định theo đường dẫn, chưa decode.
- 57 bảng Local khác xác định theo đường dẫn, chưa decode.
- 4 file chưa phân loại format: Item/Itemtest.bmd, Item/item.bmd, Skill/skill.bmd, gate.bmd.

Báo cáo đầy đủ: [world-audit-W0003.json](world-audit-W0003.json). Phân loại 149 BMD dùng inventory W0002; đây không phải lần hash lại toàn bộ Data. Hai input report có SHA-256 trong báo cáo.

## Tái lập

Từ Emulator, cần macOS Command Line Tools:

```sh
mkdir -p web/.local-baselines/2026-10-23-m00-003
clang++ -std=c++17 -O2 -fobjc-arc -I native/client web/tools/world_audit.mm native/client/assets.cpp native/client/localization.cpp native/client/item_catalog.cpp native/client/skill_catalog.cpp -framework Foundation -o web/.local-baselines/2026-10-23-m00-003/world-audit
web/.local-baselines/2026-10-23-m00-003/world-audit /Users/angelale/Downloads/MuClientEX603/Data > web/.local-baselines/2026-10-23-m00-003/worlds.json
python3 -m unittest discover -s web/tests -v
```

Tổng hợp lại vào **file mới** để giữ bằng chứng cũ:

```sh
python3 web/tools/summarize_world_audit.py --worlds web/.local-baselines/2026-10-23-m00-003/worlds.json --assets web/.local-baselines/2026-10-23-m00-002/assets.jsonl --output /tmp/world-audit-new.json
```

Test thực tế **6/6 qua**, không skip (cả hai CLI audit đã build). Fixture kiểm tra: phân biệt world chưa hỗ trợ với lỗi parser, model thiếu, record count sai; bảng chưa decode không được đánh dấu đạt; các test W0001/W0002 tiếp tục qua.

## Còn thiếu / bước sau

M00.3b vẫn PARTIAL: chưa decode 42 world còn lại, 140 BMD không phải model, chưa kiểm tra terrain texture/light/fallback, toàn bộ animation frame, visual/collision. Không tự thay Data hoặc coi mapping thiếu là file nguồn hỏng.

Tiếp theo cần đối chiếu loader/mapping và các file thiếu với source hiện có, xác định format đủ bằng chứng để mở rộng decoder. Windows visual baseline, browser benchmark và backup DB trước online-write vẫn chưa làm. Chưa có client WebGL chạy được.
