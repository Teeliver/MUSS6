# W0027 — Audit catalog custom và quy tắc đường dẫn resource

## Kết quả thực tế ngày 23/10/2026

Ba file được đọc đúng phạm vi, không mở credential/config chung:

- MuServerEX603/Data/Custom/CustomWing.txt: 0 entry active.
- MuServerEX603/Tools/GetMainInfo/Data/CustomWing.txt: 0 entry active.
- MuServerEX603/Tools/GetMainInfo/Data/CustomItem.txt: 0 entry active.

Các entry ví dụ nằm trong comment; không coi chúng là model đang được sử dụng. Hai catalog wing text khớp trong trạng thái rỗng. Không có custom model active để kết luận asset present/missing hoặc kiểm texture dependency. Không suy ra toàn bộ resource builtin đã đúng chỉ vì custom rỗng.

Artifact JSON cục bộ (ignored): `web/.local-baselines/2026-10-23-w0027/custom-catalog.json`, chứa hash ba catalog, count, mapping/asset checks nếu có row và collision evidence; không chứa ServerInfo.sse/serial/account. Không copy Data vào web/dist.

## Đối chiếu source

- GameServer/CustomWing.cpp Load: Index, item group/type, numeric options, ModelType, ModelName. Header server giới hạn50 entry.
- GetMainInfo_EX603/GetMainInfo/CustomWing.cpp cùng schema text; Main_EX603/Main/CustomWing.h cho phép100 entry. Không tự cho server nhận100 hoặc thay đổi cấu hình.
- GetMainInfo_EX603/GetMainInfo/CustomItem.cpp: Index tự tăng; group/type, RGB, ModelName. Main giới hạn2000.
- Main_EX603/Main/Item.cpp ItemModelLoad: wing luôn Item/ModelName; custom item nhóm7 đến dưới12 dùng Player/, còn lại Item/. Audit thêm đuôi .bmd và kiểm exact/case-mismatch/missing/ambiguous-case, không tự đổi tên file trên filesystem.
- GetMainInfo.cpp tạo Path/ServerInfo.sse; Main.cpp ReadMainFile đọc ServerInfo.sse. Catalog text không chứng minh binary đang được Windows client sử dụng trùng với text. Chưa kiểm chứng binary catalog; không trích toàn bộ binary vì có cả profile/private fields.

Packer CharSet `(Index+1)<<1` trên byte17: index15→32 trùng cánh130,31→64 trùng131,47→96 trùng132 trong phạm vi server50. Không có active collision vì catalog hiện rỗng. Các status ambiguous từ W0026 vẫn giữ bảo thủ cho đến khi runtime catalog đồng nhất được cấp và kiểm chứng; chưa tự resolve bằng catalog cục bộ audit.

## Công cụ / kiểm thử

`web/tools/custom_catalog_audit.py` là công cụ offline read-only với output JSON do caller chỉ định. Parser hỗ trợ format mỗi row một dòng, quoted model stem, dấu phẩy group/type, comment // và end; giới hạn schema/index/item/RGB/model stem, từ chối duplicate hoặc path traversal. Đây là subset parser audit chặt, không thay CMemScript runtime hoặc tự rewrite text. Asset scan chỉ kiểm đường dẫn file BMD cho entry custom, chưa decode model/texture.

```sh
python3 -m unittest discover -s web/tests -p test_custom_catalog.py -v
python3 web/tools/custom_catalog_audit.py --server-wing /Users/angelale/Downloads/MuServerEX603/Data/Custom/CustomWing.txt --client-catalogs /Users/angelale/Downloads/MuServerEX603/Tools/GetMainInfo/Data --data /Users/angelale/Downloads/MuClientEX603/Data --output web/.local-baselines/2026-10-23-w0027/custom-catalog.json
```

3/3 unit test qua: schema/directory mapping, malformed/unsafe/duplicate rejection, asset case/missing và collision index. Audit thực chạy thành công, ba count0, active collision0. Không sửa frontend/core trong đợt này; bộ46/46 + WASM của W0026 là kết quả trước đó, không chạy lại/không tính thành test mới W0027.

## TODO / bước sau

- Builtin item→model mapping chưa triển khai trong đợt này; Main chỉ gọi engine Windows qua địa chỉ0x00617D30 để load builtin, không có thân engine trong DLL extension. Cần đối chiếu adapter native và Data/catalog, ghi rõ nguồn thay vì claim có toàn bộ engine.
- Chưa compiled ServerInfo.sse catalog verification hoặc runtime provisioning; không cấp nhầm catalog từ text vào client.
- Chưa model/texture decode mới, render/animation/effect, UI, MU network hoặc hiệu năng game.
- M02/M03 vẫn PARTIAL. Không bật MU server/DB, không sửa native production/Data/config.

Bước sau: ánh xạ builtin equipment từ adapter native và kiểm từng file thật trong Data; unresolved mapping phải hiện trong báo cáo thay vì chọn model gần giống.
