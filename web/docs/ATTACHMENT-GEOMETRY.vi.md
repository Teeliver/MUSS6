# W0041 — Geometry vũ khí gắn vào Player pose

## DONE

- `client-core/include/mu/attachment_geometry.hpp`: tính world matrix qua cha của bone (Euler theo cùng quy ước geometry trước), nhân grip W0040 rồi geometry item action0. Giữ tọa độ chung Player, không center/scale riêng món đồ. Normal chỉ xoay và normalize, không dịch chuyển.
- Kiểm tra pose/action/frame, bone thiếu, hierarchy cycle/out-of-range, dữ liệu transform hữu hạn. Bridge `attachmentGeometryResult` và JS `decodeAttachmentGeometry` trả Float32 buffers sở hữu riêng, dùng giới hạn input hiện có16MiB.
- Contract synthetic kiểm tra rotation/translation, normal, pose/slot/bone thiếu và cycle. Contract native đọc trực tiếp Sword01.bmd/Player.bmd thật, dùng decoder native + Apple SIMD + grip native để so từng góc tam giác với output portable. 12 trường hợp action1/2/3 × slot0/1 × safe false/true qua với tolerance0.002 world unit. Không sửa native để khớp test.
- Debug query `weapon=0` gắn Sword01 vào tay phải, `weapon=1` tay trái; thêm `safe=1` đeo lưng. Model fixture cố định item0, không cần asset mới (162asset hiện có). Mesh vũ khí và5phần thân được render trong cùng một panel/body coordinate frame.
- Frame0 dùng chung action của class với thân hiện tại. Reuse bytes/texture startup cache; không decode lại theo từng frame rAF.

## PARTIAL / giới hạn

- Chỉ thử Sword01; chưa chọn model từ equipped inventory/CharSet. Chưa đổi stance theo vũ khí, chưa thử bow/crossbow/shield/wing geometry dù policy grip đã có kiểm tra W0040. Không phải hoàn thành weapon gameplay.
- Mesh static pose frame0, chưa bone animation interpolation hoặc live equip/unequip GPU swap. Preview xoay cả model và vẫn dùng camera kỹ thuật.
- Chưa xác nhận Windows visual parity. Kiểm thử numeric so native adapter hiện có. Skin dưới mũ, wing nền đen/hide visibility, upgrade effects, game GUI/network và RF idle còn thiếu.
- Thêm vũ khí có thể làm bounds panel thay đổi vì camera preview fit cả body+weapon; chưa phải camera cố định của game. Cần dùng camera scene thật ở bước tích hợp.
- Chưa profile frametime dài, không cam kết60FPS từ screenshot. Không bật MU server/DB, không đổi Data/native production/config.

## Validation/evidence

Kết quả build/npm/WASM/browser ghi trong STATUS.vi.md. Native geometry log và screenshot lưu `.local-baselines/2026-10-24-w0041/`. Core contract attachment được thêm vào CMake và build/test WASM.

## Tiếp theo

Stance theo loại vũ khí và metadata item nguồn, rồi animation pose theo thời gian và perf profiling. Chưa bỏ qua các lỗi skin/wing đã ghi.

Kết quả cuối:77/77 npm test, build WASM và test:wasm qua. Browser DW tay phải và safe/back có screenshot; tab giữ safe/back. FPS60 tại snapshot không phải benchmark dài. Left-hand đã numeric reference, chưa chụp visual riêng.
