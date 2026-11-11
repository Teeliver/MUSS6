# W0032 — BMD geometry buffers cho WebGL

## Đã triển khai

`client-core/include/mu/bmd_geometry.hpp` xuất geometry ở tư thế ban đầu sau parse BMD trực tiếp. Phép biến đổi Euler-radian, thứ tự nhân parent/local và bone-local vertex dựa trên native/client/assets.cpp posed_vertices. Ma trận xương chỉ xây một lần cho cả model, dùng lại cho mesh; index=-1 giữ world/static position. Normal dùng bone index riêng, chỉ xoay không cộng translation và normalize (normal0 giữ0). Đây là phần normal pipeline mới, chưa đối chiếu shader/lighting Windows.

Mỗi mesh xuất positions/normals/uv liên tục dưới dạng triangle corners de-indexed. Tách vertex theo corner để không làm mất UV/normal seam; chưa deduplicate hoặc tối ưu index buffer. Giữ winding và UV/native coordinate nguyên vẹn, không đảo Y/UV tùy ý trước khi có renderer/texture convention. Output tối đa262144 corners (8float/corner ~8MiB payload), không phải hard cap tổng heap khi parse/copy; vượt giới hạn báo lỗi.

Bridge `bmdGeometryResult` trả Float32Array JS sở hữu bản sao, không expose view WASM đã bị giải phóng. Model/geometry C++ được giải phóng sau export, không có handle animation/cache hoặc GPU buffer lifetime. `decodeBmdGeometry(core,bytes)` trong platform/assets/decode-model.js dùng cùng input cap16MiB và checked error như inspector. Metadata inspector W0031 vẫn dùng được.

Chỉ tư thế ban đầu action0 theo parser hiện tại; không có skinning liên tục/interpolation/clip selection, root-motion hoặc attach trang bị/class animation. Chưa gọi WebGL upload/draw.

## Kiểm chứng 23/10/2026

Build:wasm qua,57/57 npm test và test:wasm qua, không skip. Contract C++ W0031 được mở rộng: triangle fixture plain/encrypted/locked, initial transformed position và normal; fixture xoay90độ với xương cha/con kiểm translation và normalized direction. Host link native assets.cpp so mỗi triangle corner position với posed_vertices native, tolerance1e-4. So normal bằng synthetic numeric expectation, chưa lighting/image reference. JS-WASM kiểm Float32 type, nội dung positions/normals/uv, ownership sau input sửa/decoder gọi lại và invalid input.

Ba model gốc qua host/native pose comparison và WASM Node geometry export:

| File Data | Triangles | Output buffers |
|---|---:|---:|
| Item/Sword01.bmd |68|6528byte|
| Item/Wing01.bmd |16|1536byte|
| Player/HelmMale01.bmd |62+6|5952+576byte|

Tất cả buffer kiểm finite và positions/normals/UV length tương ứng. Texture references đọc được lần lượt sword02.jpg,elfin_wing.jpg,head_02.jpg,hide.jpg; chưa resolve/download/decode các texture trong đợt này. Report ignored: web/.local-baselines/2026-10-23-w0032/geometry.json. Không có browser/HTTP model test hoặc hình ảnh render.

Debug/Release build allowlist/cú pháp qua. Không đổi Data/config/native production hoặc bật MU server/DB; không chuyển BMD sang định dạng khác.

## TODO / bước tiếp theo

- Texture reference → file OZT/OZJ exact-case manifest, decoder/màu/alpha/orientation.
- WebGL shader/VAO/VBO/upload/dispose/context loss/camera và model preview đầu tiên; hiện canvas kỹ thuật, chưa GUI game.
- Animation clips/normal skinning conventions/effects/attach và visual parity.
- Chưa coverage toàn model, performance/gameplay60FPS, MU/browser network/provider/provisioning.
- M02/M03 vẫn PARTIAL, renderer/game UI chưa hoàn thiện.

Bước sau: resolve và đọc texture gốc của ba model thử, rồi ghép bản preview model có texture vào canvas để kiểm hình ảnh.
