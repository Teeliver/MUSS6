# W0034 — Preview model có texture trong browser

## Kết quả / trạng thái

Ngày24/10/2026 đã ghép luồng HTTP → manifest/hash → BMD/WASM geometry → texture wrapper/WASM → createImageBitmap JPEG thật → GPU upload → WebGL2 draw. Ba model Sword01/Wing01/HelmMale01 xuất hiện trong browser, xoay toàn vật thể để quan sát. Đây là technical preview, chưa scene/menu chọn nhân vật hoặc animation gốc.

URL local: http://127.0.0.1:8080/debug/?preview=models . Debug thường không có query vẫn canvas kiểm tra. Preview và shader không nằm trong Release; test kiểm allowlist tách mode. Toàn viewport, chỉ diagnostics góc trái; không thêm font/UI giả vào game.

## Tổ chức code / cách chạy

- platform/rendering/model-preview.js: bounds/fit, shader/VAO/VBO/texture, draw3panels và cleanup. Cấu trúc viewport preview là lựa chọn kỹ thuật để so asset, không phải camera MU.
- preview/start-preview.js: orchestration Debug-only, loader/hash/decode/upload, rAF visibility pause/resume, page cleanup qua app, context loss dừng và yêu cầu reload. Context restore chưa tự rebuild asset/GPU.
- tools/stage_preview.py: staging đúng3BMD+4OZJ theo allowlist, giữ bytes/file-case, tạo manifest/hash. Chỉ copy vào dist/debug/preview-assets; không expose cả Data, config/keys/server hoặc đưa asset vào Release. Không chuyển đổi BMD/texture. Staging bị xóa khi build debug/test build lại, cần chạy sau build.

```sh
npm --prefix web run build:debug
npm --prefix web run stage:preview
python3 -m http.server 8080 --bind 127.0.0.1 --directory web/dist
```

Server trên là HTTP static loopback, không phải MU server/DB. Browser trước bị chặn đọc Downloads, quyền đã được xác nhận có hiệu lực trước khi tiếp tục sửa source. Không đổi Data/config/native production.

## Kiểm chứng

62/62 npm test qua, không skip; test mới kiểm bounds, cleanup GPU đúng một lần và upload failure. Build Debug/Release kiểm syntax/allowlist và Release không chứa preview/diagnostics. Không đổi C++/WASM đợt này, không rebuild/test:wasm riêng; app/test dùng build W0033.

Browser IAB WebGL2 thực đọc7asset, kiểm hash, decode JPEG thật và render. Screenshot tại web/.local-baselines/2026-10-24-w0034/preview.png; buffer2560×1440, overlay quan sát60FPS. Đây là số đo snapshot của3model nhỏ, KHÔNG phải bảo đảm60FPS gameplay/scene/map, chưa benchmark sustained/memory/leak hoặc Safari/Chrome độc lập. Snapshot browser sau build/stage cuối xác nhận trạng thái preview loaded.

## Sai khác / chưa hoàn thiện

- Cánh còn nền đen: material đang opaque, chưa source-specific alpha-cutout/additive/render pass. Không tự dùng ngưỡng đen suy đoán rồi gọi chuẩn gốc.
- Mũ render rời có hình dạng chưa đúng nhân vật: model equipment cần skeleton/bone binding Player gốc; parser self-pose không thay thế bind theo class. Chưa khẳng định normal/light/UV/màu đúng Windows.
- Shader diffuse đơn giản, camera/fit/rotation là technical preview; không có upgrade/excellent/ancient particles, wing animation hoặc attach nhân vật.
- Chưa image reference gốc, texture filtering/material conventions đầy đủ, contextrestore autorebuild, GPU failure/end-to-end cancellation browser matrix.
- Chưa GUI game, character scene/world, MU online/provider/provisioning. M02/M03 vẫn PARTIAL; M04 có preview đầu, chưa đủ nghiệm thu renderer.

Bước tiếp theo: đối chiếu material/alpha trong native/source và bone binding Player để khắc phục đúng hai lỗi nhìn thấy; tiếp tục ghi unresolved thay vì tuyên bố model/game chuẩn100%.
