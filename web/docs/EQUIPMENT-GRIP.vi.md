# W0040 — Bone và transform gắn trang bị portable

## DONE

- Thêm `client-core/include/mu/equipment_grip.hpp` không phụ thuộc Apple SIMD: ma trận column-major, Euler Z*Y*X, phép nhân giữ đúng thứ tự native.
- Port chính sách từ `native/client/equipment_grip.hpp`: slot0 tay phải bone33, slot1 tay trái bone42; vùng an toàn hoặc tên dùng bone47. Wing slot7 dùng bone47, robe12:40 dùng bone19. Giữ toàn bộ ngoại lệ transform cho cung/nỏ, staff, shield, tay trái, robe.
- Bridge WASM `equipmentGripResult` trả bone/16 hệ số. Adapter `src/protocol/equipment-grip.js` kiểm tra slot0/1/7, index0..8191 và boolean safe; dữ liệu kết quả immutable, copy khỏi WASM. Policy này không kiểm tra item có được phép mặc hay không.
- Contract host biên dịch và gọi trực tiếp header native Apple SIMD, so 49.152 tổ hợp index × slot × safe: bone bằng tuyệt đối, từng hệ số sai số <0.0001. Không tự sửa hoặc sao chép lại hàm reference thành thuật toán giả lập.
- Có contract portable trong CMake/Emscripten, test bridge với WASM thật. Kiểm tra hand/back/arrows, bow offset, robe, input sai và ownership.

## PARTIAL / chưa có thay đổi hình ảnh

- Đợt này hoàn thành quy tắc bone và local transform; CHƯA áp dụng lên mesh vũ khí trong preview. Trang bị chưa xuất hiện trên thân, chưa chuyển stance theo vũ khí, chưa nhân với bone world matrix của pose. Không tuyên bố hoàn tất weapon renderer.
- Bước sau: pose bone world matrix × grip local matrix × mesh vũ khí; kiểm tra numeric geometry với native trước khi chụp preview. Cần giữ cùng coordinate system, không center hoặc normalize vũ khí riêng.
- Skin dưới mũ, wing nền đen/hide visibility, animation interpolation/effects, GUI/network MU và RF idle còn thiếu như W0039. Chưa có đủ dữ liệu cam kết60FPS; cần frametime profile khi render tích hợp.
- Header native là nguồn port hiện có, không phải bằng chứng mọi nhánh Windows engine đầy đủ. Không sửa native production/Data/config, không bật MU server/DB.

## Validation

Build WASM Emscripten đã pin; kết quả npm test và test:wasm ghi trong STATUS.vi.md. Debug/Release vẫn tách; chỉ thêm module policy thuần vào shared allowlist, không đưa overlay/preview/assets vào Release. Sau test build cần stage lại các asset Debug.
