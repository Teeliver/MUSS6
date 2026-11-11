# W0037 — Thân cơ bản theo class và cấp chuyển chức

## Đã làm

- Debug preview nhận `class=0..6`, `tier=0..2`: DW, DK, Elf, MG, DL, Summoner, RF. Ví dụ `/debug/?preview=models&class=2`.
- `body-geometry.js` chọn 5 phần Helm/Armor/Pant/Glove/Boot theo nhánh `armor_model(type=511)` trong `native/client/character_resources.cpp`. MG/DL/RF tier1 dùng thân tier0 như native. Test biên dịch trực tiếp hàm native và so đủ 105 đường dẫn, không lấy tên file từ suy đoán.
- Pose đứng: Elf action2, Summoner action3, các class khác action1 theo adapter native. RF giữ cảnh báo: male idle của native chưa được xác minh với Windows.
- `tools/body_asset_audit.mjs` đọc BMD qua core WASM hiện có, ghép từng tổ hợp với Player.bmd, kiểm tra bone/action và thu texture reference từ mesh. 21/21 tổ hợp qua; 115 đường dẫn duy nhất cho nhóm thân. Đây là kiểm tra decode/binding, không phải chứng nhận visual parity.
- Staging Debug 123 asset tổng cộng, chỉ các asset preview đã khai báo/kiểm tra; không xuất toàn bộ Data. Có `preview-assets/body-audit.json` ghi kết quả từng tổ hợp. Data gốc không sửa, Release không chứa preview hoặc các asset staging.
- 67/67 npm test qua, không skip. Không đổi C++/WASM nên không rebuild WASM hay chạy riêng test:wasm đợt này. npm test kiểm tra cả build Debug/Release và hợp đồng WASM liên quan.
- Browser kiểm tra trực tiếp Elf và Summoner; ảnh trong `.local-baselines/2026-10-24-w0037/`. Chỉ static preview server loopback, MU server/DB không bật.

## Chưa hoàn thiện

- Chỉ chuyển nhánh thân cơ bản type511 sang preview JS, chưa chuyển toàn bộ armor substitutions và equipment resolver sang portable core. `outfit=helm` là fixture riêng DW tier0; class khác bị từ chối thay vì giả định mũ tương thích.
- Chưa có character selection/game GUI, trang bị/weapon attachment, animation key interpolation, effects, network MU thật hoặc gameplay. Nhân vật hiện dùng frame0; chuyển động xoay là xoay cả model.
- Wing JPEG nền đen và quy tắc hide material vẫn chưa giải quyết. Chưa xác nhận hình cả 21 tổ hợp với Windows; RF idle chưa xác minh. FPS preview không bảo đảm 60FPS gameplay.
- Audit yêu cầu WASM đã build; stage chạy audit lại trước khi sao chép. Chưa tối ưu parse skeleton lặp lúc startup; cache bytes/image và shared GPU texture hiện có vẫn giữ.

## Tiếp theo

Port đầy đủ resolver trang bị theo class, kiểm tra bone attachment và material visibility trước khi ghép scene chọn nhân vật; thêm animation theo action/frame dựa source, rồi mới mở rộng GUI. M02/M03 vẫn PARTIAL.
