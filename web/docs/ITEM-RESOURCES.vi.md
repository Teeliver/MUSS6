# W0028 — Portable item resource mapping và audit Data

## Phạm vi và nguồn

Core `client-core/include/mu/item_resources.hpp` chuyển quy tắc native/client/item_models.cpp sang std::string relative path, không filesystem/OS runtime. Adapter JS `protocol/item-resources.js` kiểm index0..8191 và level0..15 trước Embind. Kết quả unmapped trả mapped=false/path rỗng, không lấy model gần giống. Model mapping hiện có là phạm vi adapter native, KHÔNG phải toàn bộ Windows engine; Main_EX603 gọi builtin loader qua địa chỉ như W0027 đã ghi.

Giữ chính xác các nhánh vũ khí/armor item icons/cánh/sách/jewel/potion/event đã có trong adapter, kể cả nhánh level-dependent RingOfLord. Không đổi tay vũ khí hoặc dựng thêm đường dẫn bằng đoán tên. Mô hình trang bị đang mặc còn cần character_resources.cpp armor_model với class/tier; không thay bằng mapping icon armor có phạm vi hẹp hơn. Wings/pet decoder W0026 không tự trở thành model khi mapping trống.

## Kiểm chứng và Data thực

Build WASM, npm test **48/48 qua**, test:wasm qua, không skip. Native test trích nguyên function item_model_path từ adapter vào fixture C++ với Item tối thiểu, so portable output cho toàn8192 ID và levels0/1/4/15. WASM chạy sweep domain + bridge checks; source-reference std::filesystem chỉ chạy host, không claim reference chạy trong browser. Debug/Release allowlist/cú pháp qua.

Công cụ `web/tools/item_resource_audit.py` compile executable portable inventory trong thư mục temp rồi đối chiếu file BMD trong Data, không mở config/credential hoặc đổi file gốc. Lệnh:

```sh
python3 web/tools/item_resource_audit.py --data /Users/angelale/Downloads/MuClientEX603/Data --output web/.local-baselines/2026-10-23-w0028/item-resources.json
```

Kết quả thực: 1142 mapped cases ở levels đã sweep, 285 unique paths. 1098 case exact,44 case lệch chữ hoa/thường; 0 missing và0 ambiguous-case trong phạm vi mapped. Có11 unique path lệch case: Jewel22, Saint, Shield16/17, Spear11, Sword21/22, Wing08/09/10/11 — file Data dùng chữ đầu lowercase. Chưa sửa Data hoặc tự thay source canonical paths; manifest resolver cho host phân biệt case là việc kế tiếp.

Đây là domain scan của adapter, không phải coverage của mọi Item.bmd active row. Không coi8192 ID đều là item tồn tại. Không kiểm texture dependencies, geometry hợp lệ hoặc hình ảnh chỉ bằng file existence. Kết quả JSON ignored chứa path/status, không copy Data vào dist.

## Chưa hoàn thiện / bước sau

- Chưa manifest runtime ánh xạ canonical→exact filename theo hash/version và từ chối collision;11 path lệch case cần giải quyết trước HTTP asset load.
- Chưa body armor class/tier mapping port, mọi builtin item mapping, pet/custom catalog hoặc model/texture decoding/render.
- Chưa UI chọn nhân vật, image regression, MU network/provider/provisioning hoặc FPS gameplay.
- M02/M03 vẫn PARTIAL. Không bật MU server/DB, không đổi native production/Data/config.

Bước sau: manifest resource đúng case có kiểm collision/bounds rồi port body armor class/tier, trước khi dựng scene.
