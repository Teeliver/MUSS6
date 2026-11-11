# W0026 — CharSet cánh và pet

## Nguồn / implementation

GameServer/DSProtocol.cpp DGCharacterListRecv, nhánh UPDATE>=601 (build603), đóng gói wing slot7 và pet slot8. GameServer/CustomWing.h cho phép50 entry. Core mới `client-core/include/mu/companions.hpp`, bridge checked-result và `decodeCompanions` trong appearance-codec.js đọc cùng raw18 byte; không đổi seven-slot decoder W0025.

Status được trả rõ: 0=không có, 1=item tích hợp, 2=cần custom catalog, 3=mã chưa hiểu, 4=mơ hồ giữa builtin/custom. `index` chỉ có nghĩa item xác định khi status1; status4 là ứng viên builtin, chưa được chọn model. `customIndex` là ứng viên entry catalog zero-based khi status2/4. Giữ raw copy để sau này xử lý/xác minh, không biến unknown thành none.

Wing group12: các nhánh source0–6,30,36–43,49–50,130–135,262–267 được đối chiếu. Mã130–135 dùng high bits byte17, có thể trùng custom index15/31/47 và các entry khác theo mã byte; parser đánh dấu status4, không khẳng định builtin chỉ dựa bit. Custom dùng `(Index+1)<<1`, cần catalog đồng nhất server/client để ánh xạ và phát hiện collision. Chưa load catalog thật, chưa giải quyết ambiguity bằng giả định thứ tự ưu tiên renderer.

Pet group13:0–4,37,64,65,67,80,106,123; sentinel511 là none. Fenrir37 giữ variant1/2/4 theo ưu tiên server (bit1 trước bit2 trước bit4), không coi nhiều bit là nhiều pet. Extended pet ở byte16 high bits được xét trước base bits; wing và pet chia sẻ byte nhưng dùng mask riêng. Unknown code giữ status3.

## Bằng chứng

Reference trích nguyên đoạn packing wing/pet của DSProtocol.cpp, chạy nhánh603. CustomWing trong fixture là catalog giả duy nhất ánh xạ item12:500→entry3; không đọc config thật hoặc giả rằng catalog đó tồn tại trong game. Cross-product toàn bộ nhánh builtin đã liệt kê +none/custom với mọi pet và Fenrir variant0–7 được kiểm trên native/WASM. Bridge kiểm builtin/custom/unknown, raw ownership và input length sai.

Các lệnh: `npm --prefix web run build:wasm`, `npm --prefix web test`, `npm --prefix web run test:wasm`. Kết quả: build WASM qua, 46/46 test và test:wasm qua, không skip. Test fixture không phải interoperability/browser hoặc bằng chứng render đúng model/effect. Debug/Release allowlist/cú pháp kiểm cùng bộ test, UI giữ nguyên.

## TODO / chưa đạt

- Custom catalog runtime và validation index/collision; chưa cover mọi custom entry/extension ngoài nhánh đã khảo sát.
- Model path/catalog, cánh/pet animation, attach bone, shader/particle; chưa hiển thị scene.
- Chưa packet network MU/browser, provider/profile/key provisioning hoặc gameplay performance.
- Không bật MU server/DB, không đổi Data/config/native production. M02/M03 vẫn PARTIAL.

Bước sau: audit catalog custom wing và ánh xạ item→resource theo source/Data, ghi rõ collision/missing asset trước render. Không đoán model từ tên item.
