# W0039 — Ghép giáp độc lập theo slot

## DONE

- Preview chọn riêng 5 loại theo thứ tự Helm/Armor/Pant/Glove/Boot, giống mảng armor trong native. Type511 nghĩa là không mặc món ở slot đó, dùng phần thân class theo resolver gốc.
- Bỏ mapping tên file base-body và test helmet riêng trong JS; tất cả đi qua cùng `armorResource`/WASM resolver W0038. Không còn hai bộ quy tắc tên model cần bảo trì song song.
- Query `armor=10` vẫn là fixture cùng loại cho cả5slot. `armor=511,10,10,10,10` giữ đầu Summoner cơ bản và mặc4phần giáp. Parser chỉ chấp nhận một hoặc5số0..511, từ chối rỗng, số mũ, thiếu slot, dấu âm, type ngoài miền; không kết hợp outfit fixture với equipped fixture.
- Copy mảng đầu vào, kiểm tra thứ tự/path từng phần và giữ chung Player pose. Test mixed/unequip path không giữ model giáp cũ, dữ liệu immutable không bị sửa. Đây là kiểm tra ghép dữ liệu, chưa có live equip UI.
- Audit mở rộng5mixedfixture DW/Elf/Summoner/DL/RF: tổng31/31 tổ hợp base/equipped/mixed decode/binding/texture dependency qua. Không tăng phạm vi staging soW0038:162asset vì dùng lại các file đã có. Chưa kiểm tra toàn bộ armor trong Data.
- **73/73 npm test** qua, không skip, bao gồm native reference và WASM bridge; Debug/Release build separation qua. Không đổi core C++/bindings nên không rebuild WASM hoặc chạy test:wasm riêng đợt này.
- Browser Summoner mixed hiển thị đầu/tóc class cùng giáp. Ảnh `.local-baselines/2026-10-24-w0039/summoner-mixed.png`. Screenshot có FPS55.0; chưa đủ dữ liệu để kết luận nguyên nhân hoặc bảo đảm60FPS, cần profile frametime thực tế sau. Không che số đo hay coi test pass là hiệu năng đạt.

## PARTIAL / chưa sửa

- Giữ đầu khi KHÔNG đội mũ đã kiểm chứng. Đây không phải sửa thiếu skin/đầu bên dưới mũ ở full armor10: trường hợp có mũ vẫn còn lỗi nhưW0038.
- Không tìm thấy thân hàm render Windows trong bộ Main extension hiện có; chỉ có địa chỉ gọi pRenderPartObjectEffect. Native body loader chỉ nạp5model, không có quy tắc bổ sung skin đủ để chứng minh cách sửa. Không tự đoán mesh hay xóa pixel đen.
- Wing nền đen, hide visibility, animation interpolation, weapon attachment, upgrade effects và Windows visual parity vẫn chưa xong. RF idle chưa xác minh.
- Chưa có equip eligibility/CharSet-to-scene/inventory online/game GUI. Mixed array là fixture render, không sửa server hay cho nhân vật mặc đồ trái điều kiện.
- MU server/DB không bật; Data/native production/config không sửa. M02/M03 vẫn PARTIAL.

## Tiếp theo

Weapon attachment: port transform và hand/back bone policy từ native, kiểm tra bằng numeric reference trước khi ghép preview; tiếp tục cần nguồn render đầy đủ để sửa skin dưới mũ và wing visibility. Đo frametime để xác định chi phí render trước khi khẳng định mục tiêu60FPS.
