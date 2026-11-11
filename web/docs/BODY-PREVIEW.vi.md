# W0036 — Thân đầy đủ và sửa chiều pixel OZJ bằng reference native

## Thay đổi và nguồn

Preview/body-geometry.js ghép5part từ native character_resources.cpp armor_model: class0/tier0/no armor dùng Helm/Armor/Pant/Glove/BootClass01.bmd. Mọi mesh giữ chung hệ tọa độ Player clip1/frame0; không căn giữa từng part. Outfit=helm thay head bằng HelmMale01 để kiểm binding với đủ thân, không chứng minh nhân vật DW có quyền mặc item tương ứng. Đây là technical asset test, chưa có rule equip/requirements/game inventory.

Preview/start-preview.js cache byte skeleton và decoded texture trong lúc chuẩn bị, dùng chung ảnh giữa5part; model-preview.js reuse GPU texture theo decoded image identity, dispose một lần. Model Player vẫn được parse lại trong từng bound decode call: chưa model-handle/cache C++ chung; không claim tối ưu hết startup. Không parse/skin model theo rAF, preview vẫn pose tĩnh và chỉ xoay vật thể.

Staging Debug từ8→14file, thêm5base body BMD và skin_barbarian_01.OZJ. Release không chứa preview hoặc stagedData. Các URL kỹ thuật:

- /debug/?preview=models : base body DW.
- /debug/?preview=models&outfit=helm : test helmet +4base parts.

## Tìm đúng lỗi texture

Native asset_images.mm decode_texture(OZJ) có CGContext row flip (game_hud.mm cũng ghi rõ decoder này flips rows for GPU UVs). Browser trước dùng createImageBitmap imageOrientation:none nên JPEG rows không giống native. Shader đã giữ1-v theo geometry native; không sửa UV tùy ý để che sai pixel.

Đã build CLI native RGBA reference từ asset_images.mm/assets.cpp cho Player/skin_barbarian_01.OZJ (256×256), kiểm từng pixelRGB trong browser bằng OffscreenCanvas. Trước sửa mean absolute RGB error cùng hàng83.5277405/255, so với hàng đảo0.06129964. Sau đổi createImageBitmap imageOrientation:flipY, cùng hàng0.06129964, hàng đảo83.5277405. Khác nhỏ còn lại là decoder/platform; không claim bit-identical/color-management parity. OZT RGBA path không đổi.

Fixture source/rows/record được giữ ignored tại web/.local-baselines/2026-10-24-w0036/; trang row-check và reference RGBA tạm đã bỏ khỏi dist sau khi kiểm. Không đổi/convert file Data gốc. Browser screenshots body-base.png và body-helm.png xác nhận thân/mũ đã hết kiểu texture chắp sai do đảo hàng. Texture hide.jpg còn render như opaque khi xuất hiện; quy tắc mesh visibility không được đoán thành xóa mesh tự động.

## Kiểm thử / giới hạn

65/65 npm test qua, không skip. Test mới kiểm5part/slot order/shared skeleton pose/no per-part recenter và GPU texture reuse/lifetime. JPEG adapter test assert flipY. Debug/Release allowlist/cú pháp qua; không đổi C++/WASM, không rebuild/test:wasm riêng trong W0036. Các bound decode assets thực chạy bằng WASM Node trước ghép browser.

Browser IAB thực render5part base và testhelmet với texture JPEG decode thật. Overlay snapshot khoảng60FPS cho preview nhỏ, không phải gameplay performance hoặc benchmark dài. HTTP static loopback giữ từ W0034; MU server/DB không bật.

## Còn thiếu / bước tiếp theo

- Wing nền đen chưa sửa, cần source render pass/material semantics; không RGB threshold phỏng đoán.
- UV/color/material parity đầy đủ/mesh hide convention và image reference Windows chưa nghiệm thu.
- Chưa body các class/tier khác, item requirements/equip thật, weapon attach/upgrade/wing particles.
- Chưa animation interpolation, GPU skinning/model handle và startup cache/memory benchmark.
- Chưa GUI game/character select/online MU, provider/profile/key provisioning; M02/M03 vẫn PARTIAL.
- Không sửa native production/Data/config, không thêm font game hoặc UI thay nguồn.

Bước sau: tách body resource mapping class/tier theo native và thử các class cơ bản; phần wing material còn phải tìm đủ bằng chứng trước khi đổi blending/alpha.
