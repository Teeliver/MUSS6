# W0033 — Texture gốc OZT/OZJ và dependency mapping

## Source / implementation

Native/client/assets.cpp decode_tga + jpeg_payload và native/client/asset_images.mm resolve_model_texture/decode_texture là nguồn adapter hiện có. Model texture reference .jpg→.OZJ, .tga→.OZT trong cùng directory; không tìm file cùng tên ở folder khác để che missing. JS textureReference giới hạn model Item/Player và filename an toàn; chưa nested/custom texture path.

Core `mu/texture.hpp`: OZT uncompressed truecolor type2,24/32bit BGR(A)→RGBA, TGA imageID và top/bottom,left/right origin theo native. Giữ alpha32bit và alpha255 cho24bit. Input<=16MiB, dimensions1..2048; native cho4096, portable chủ động giới hạn decodedRGBA<=16MiB. Không hỗ trợ RLE/palette/OZB hoặc tự chuyển format khi không đọc được.

OZJ bỏ wrapper24 byte như native, kiểm SOI/EOI và JPEG segment bounds/SOF dimensions trước khi gọi decoder browser. Hỗ trợ SOF baseline/progressive8bit1 hoặc3 components, không CMYK/arithmetic/lossless. Đây không phải decoder entropy JPEG C++: WASM trả JPEG payload nguyên, browser createImageBitmap giải mã trực tiếp qua Blob(image/jpeg). Không sinh file JPEG/glTF bên ngoài hoặc sửa Data.

Bridge `textureResult` checked-error/type/cap và copies bytes. inspectTexture cho metadata/payload; decodeOriginalTexture trả RGBA owned hoặc bitmap có dispose idempotent. JPEG dùng premultiplyAlpha:none,colorSpaceConversion:none,imageOrientation:none; kiểm kích thước bitmap so metadata. Abort trước decode không chạy; nếu abort khi createImageBitmap đang chạy, bitmap trả muộn được close rồi reject (chưa cancel ngay công việc decoder nội bộ/không deadline riêng). Chưa kiểm màu/EXIF/orientation trên browser thực.

Manifest generator/resolver W0029 được mở rộng cho suffix bmd/OZJ/OZT, còn giữ directory allowlist/case identity/hash/collision guards. Không dùng hình thay thế khi thiếu texture.

## Data thực và kiểm thử 23/10/2026

| Texture Data | Dimensions | Bytes |
|---|---:|---:|
| Item/sword02.OZJ |64×16|6316|
| Item/elfin_wing.OZJ |64×64|9682|
| Player/head_02.OZJ |64×64|12284|
| Player/hide.OZJ |9×9|4556|

Bốn wrapper thật qua WASM Node inspection. Manifest4 file,32838byte,0 case correction, SHA256 từngfile được tạo cục bộ tại `web/.local-baselines/2026-10-23-w0033/texture-manifest.json`; metadata tại textures.json. Đây chưa phải pixel decode browser hoặc asset HTTP load. hide9×9 không bị thay bằng ảnh khác.

Build WASM,61/61 npm test, test:wasm và2/2 Python manifest tests qua, không skip. OZT reference pixel so nguyên decode_tga native trên24/32bit và4origin; contract cũng chạy WASM. JS-WASM kiểm wrapper dimensions/length, RGBA alpha/channel; JPEG browser adapter dùng fake createBitmap để kiểm options/dimensions/disposal/abort, không claim JPEG pixels thật đã được kiểm. Debug/Release allowlist/cú pháp qua.

## TODO / bước tiếp theo

- HTTP staging và browser createImageBitmap thực của texture gốc; WebGL UV/row orientation/color parity/image regression chưa có.
- Renderer shader/material/blend/alpha-cutout/light, VBO/VAO/texture upload/context loss/disposal/camera và preview model đầu tiên.
- Chưa decode toàn Data, RLE/OZB, custom/nestedtexture policy, resource cache/scheduler/performance.
- Animation/effects/body class-tiers/GUI game và MU network/provisioning vẫn thiếu; M02/M03 PARTIAL.
- Không bật MU server/DB, không đổi native production/Data/config. Trang hiện chỉ canvas kiểm tra + overlay, chưa GUI game.

Bước sau: staging bộ3BMD+4texture và manifest cục bộ có phạm vi rõ, ghép WebGL preview textured model để kiểm ảnh browser. Không gắn nhãn preview là màn chọn nhân vật/gameplay đã hoàn thiện.
