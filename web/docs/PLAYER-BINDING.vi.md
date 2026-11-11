# W0035 — Player skeleton binding và đối chiếu material

## Source và phần đã làm

Native/client/character_resources.mm load_character_body tải Player.bmd rồi kiểm bone index/name/parent của equipment trước khi dùng pose skeleton. Core bmd_geometry.hpp có bound_geometry: kiểm vertex/normal bone identity, action/frame availability, dùng pose đã retain của Player cho cùng mesh equipment; không dùng equipment self-pose hoặc ghép xương theo vị trí đoán. Normal có index=-1 được giữ static; vertex equipment yêu cầu bone hợp lệ như native.

Bridge boundGeometryResult và JS decodeBoundGeometry nhận byte equipment +Player, action/frame explicit (mặc định1/0 trong preview, male idle theo native character_resources.cpp). Core chỉ retain action được chọn, không load cả284action vào output. Chưa có animation handle/time interpolation hoặc mapping class đầy đủ. Preview staging thêm Player/Player.bmd, tổng8asset trong Debug; Data gốc giữ nguyên.

Material đối chiếu entity_view.mm: SCNLightingModelConstant, Repeat UV; alpha source từ texture, discard alpha<0.25 nếu transparent. Preview bỏ diffuse light giả, dùng texture màu không nhân lighting và Repeat; shader discard alpha<0.25. JPEG opaque nên điều này KHÔNG xóa nền đen của Wing01. Không tự sinh alpha theo RGB đen vì source đã khảo sát chưa có quy tắc đó. Main_EX603 extension không có kết quả tìm elfin_wing/Wing01/RENDER_BRIGHT/RENDER_ALPHA; negative search không chứng minh engine Windows không có rule.

## Kiểm thử / bằng chứng 24/10/2026

Build WASM,63/63 npm test và test:wasm qua, không skip. Fixture bound pose, invalid action/frame, bone name mismatch; geometry contract native/WASM có kiểm pose binding. Host thực so mọi triangle corner HelmMale01 sau binding với native posed_vertices dùng Player clip1/frame0, tolerance1e-4: qua. Player thật có60bones/284actions; chỉ pose được chọn dùng cho export.

Browser IAB đã tải8asset/hash, decodeBMD/texture và render trạng thái Player bind. Screenshot ignored web/.local-baselines/2026-10-24-w0035/preview.png, buffer1508×1798. Overlay có snapshot60FPS trong preview nhỏ; chưa benchmark gameplay hoặc render leak. Debug/Release separation và cleanup tests qua; Release không nhận preview/assets.

## Phần chưa đúng / không đánh dấu hoàn thiện

- Mũ rời vẫn chưa có hình ảnh đủ đúng để nghiệm thu: numeric pose đã khớp native nhưng cần whole-body context, UV/orientation và quy tắc mesh visibility/material. Không gọi binding đúng số là visual đã đúng.
- Cánh vẫn nền đen, source render-pass/material đầy đủ chưa tìm thấy trong Main extension. Không che lỗi bằng ngưỡng RGB tùy ý.
- Chưa toàn thân/armor class/tier, hide-material semantics, multi-pass/render effect, animation hoặc GUI game.
- Chưa browser reference Windows, crossbrowser/performance, MU network/provider/provisioning. M02/M03 tiếp tục PARTIAL.
- Không bật MU server/DB hoặc đổi native production/Data/config; chỉ HTTP preview loopback giữ từ W0034.

Bước sau: ghép đủ body parts cùng skeleton Player và texture, kiểm mesh hide/UV với nguồn native; tiếp tục điều tra wing render pass riêng. Ưu tiên kiểm hình ảnh không đoán alpha hoặc model thay thế.
