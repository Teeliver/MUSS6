# W0038 — Resolver model trang bị theo class

## DONE

- Thêm `client-core/include/mu/armor_resources.hpp`, port nguyên các nhánh của `armor_model` trong `native/client/character_resources.cpp`: thân type511, Summoner ElfC, RF Monk (trừ glove), DL mask, mũ thay bằng đầu cơ bản, MaleTest/t_PantMale, HDK/CW và các giới hạn loại đã biết. Không dùng resolver icon inventory để chọn giáp mặc.
- Bridge `armorResourceResult` và adapter JS `src/protocol/armor-resources.js` có kiểm tra part0..4/type0..511/class0..6/tier0..2; loại chưa map trả lỗi rõ, không lấy model khác thay thế. Header C++ cũng từ chối class/part/tier sai.
- Test host biên dịch trực tiếp hàm native, so cả đường dẫn lẫn lỗi cho 53.760 tổ hợp. Contract C++ chạy thêm trong WASM; test bridge JS chạy với WASM thật, có kiểm tra các nhánh thay thế.
- Debug preview `armor=<type>` chọn cùng type cho 5 phần qua core resolver. Đây là fixture render, không phải cấp đồ hay kiểm tra điều kiện mặc. Không kết hợp với `outfit=helm`. Ví dụ DW `?preview=models&class=0&armor=0`, Summoner `class=5&armor=10`.
- Audit Data mở rộng thêm fixture DW0, Elf10, Summoner10, DL0, RF5; cùng base-body tổng 26/26 tổ hợp decode/bone/action/texture dependency đạt. Chỉ 162 asset được staging vào Debug; không sao chép cả Data.
- Phát hiện 4 texture có extension gốc viết thường `.ozj/.ozt`; manifest Python/JS chấp nhận case extension, vẫn kiểm tra path, hash, size, cùng identity và giới hạn folder. Không đổi tên Data. Test JS về lowercase và Python manifest hiện có qua.
- Build WASM bằng Emscripten đã pin; **71/71 npm test**, `test:wasm`, 2 test Python manifest qua. Debug/Release allowlist qua. Không bật MU server/DB hay sửa native production/Data/config.
- Browser kiểm tra DW/Summoner, ảnh tại `.local-baselines/2026-10-24-w0038/`; tab giữ ở DW armor0. FPS60 chỉ là snapshot technical preview.

## PARTIAL / lỗi quan sát

- Mapping khớp adapter native hiện có; chưa có bằng chứng toàn bộ Windows engine giống100%. Audit chỉ kiểm tra 5 bộ giáp cụ thể, không chứng nhận tất cả model của mọi type đều tồn tại/hiển thị đúng.
- Summoner fixture armor10 còn thiếu phần đầu bên dưới mũ; cần chuyển logic skin/body bổ sung và visibility từ render source, không vá bằng model đoán. Wing vẫn có nền đen, hide material chưa giải quyết. Đây là lỗi hình ảnh chưa hoàn thiện, dù mapping/binding test đạt.
- Chưa gắn vũ khí/wing vào bone, chưa animation interpolation hoặc effect cấp cộng/Excellent. Hiện vẫn frame0 và xoay cả object.
- Resolver chỉ chọn model, không thay server equip eligibility hoặc cập nhật trang bị gameplay. Chưa nối inventory/CharSet vào scene online và chưa có GUI chọn nhân vật/game. MU network/provisioning và RF idle/Windows parity còn thiếu.
- Base-body helper JS của W0037 vẫn có mapping riêng đã reference-test; cần gom về một resolver core khi tích hợp character scene.

## Bước sau

Ưu tiên skin/body bổ sung và visibility (đầu dưới mũ, hide material), rồi weapon attachment theo bone nguồn; sau đó animation action/frame. Không coi preview giáp là hoàn tất character renderer. M02/M03 vẫn PARTIAL.
