# W0031 — Đọc BMD trực tiếp trong portable core/WASM

## Đã triển khai

`client-core/include/mu/bmd_model.hpp` port bộ đọc model từ native/client/assets.cpp cùng kiểu mesh/bone/clip sang C++ không filesystem/Cocoa. `decode_model(bytes)` nhận trực tiếp byte BMD gốc, không tạo glTF hoặc file chuyển đổi. Hỗ trợ signature BMD version10 và12; version12 kiểm encrypted payload size rồi dùng thuật toán decrypt_file native. Không coi mọi file đuôi .bmd là model: Item/Skill/Text tables có schema khác.

Parser đọc vertex/bone indices, normals, UV, triangle/quad (quad tách triangle theo native), texture name, action/key metadata, bone hierarchy và position/rotation. Giữ action0 theo default native; các action khác vẫn được đọc/validate nhưng chưa giữ để phát animation qua JS. Action requested khác chỉ có API C++ lúc này, chưa expose trong bridge. Locked root-position vectors được validate nhưng chỉ lưu locked flag theo native hiện có; chưa xuất root-motion track. Chưa tuyên bố playback đầy đủ giống engine Windows.

Bounds riêng cho WASM: input16MiB; mesh<=128,bone<=256,action<=512,vertex/mesh<=30000, tổng geometry records có trọng số<=262144; mỗi action keys<=4096, tổngkeys<=100000; retained key×bone<=32768 và retained action keys<=256. Không tăng heap vô hạn để nhận model bất thường. Native reference có một số giới hạn rộng hơn; đây là giới hạn portable hiện tại, không sửa file Data để vượt qua.

Float được đọc LE32, yêu cầu finite và abs<1e7; áp cả action không retain và locked vectors (native trước đây skip vùng này). Kiểm hierarchy cycles và parent range, face/UV/normal/vertex indices, texture name traversal, EOF/trailing data. Không sửa native production để chia sẻ parser trong đợt này.

Bridge `bmdModelResult` kiểm size/type trước copy (không dùng packet cap262144), bắt exception và trả metadata mesh/count/texture/bone parents/retained clip frame counts. JS `platform/assets/decode-model.js` chỉ cung cấp `inspectBmdModel`, CHƯA trả GPU vertex/index/skin buffers hay giữ handle model. C++ bên trong đã đọc geometry nhưng model được giải phóng sau metadata inspection. Bước này là parser contract, chưa renderer.

## Kiểm chứng ngày 23/10/2026

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Build WASM,56/56 test và test:wasm qua, không skip. C++ fixture triangle có UV/normal/bone/action và root-lock tắt/bật, plaintextv10/encryptedv12; mọi prefix bị cắt, trailing byte, version sai, float NaN và parent tự trỏ bị từ chối. Host test link nguyên native/client/assets.cpp, ghi fixture trong temp test directory rồi so geometry/metadata với load_model native. WASM chạy cùng portable contract, bridge test metadata/size/type/error. Reference native filesystem không link vào WASM.

Model thật đã kiểm: Data/Item/Sword01.bmd (1mesh,1bone,1action), Item/Wing01.bmd (1mesh,11bones,1action), Player/HelmMale01.bmd (2meshes,56bones,1action). Cả ba version10, được đọc từ byte gốc bằng WASM trong Node và host comparison với native parser; không qua network/browser. Geometry reference kiểm vertices/face indices/UV/texture, bone parent/name và một số pose components; chưa so toàn bộ animation/effect. Version12 hiện bằng fixture, không khẳng định kiểm tất cả encrypted model thật.

Metadata thực lưu ignored tại `web/.local-baselines/2026-10-23-w0031/bmd-models.json`. Debug/Release allowlist/cú pháp module mới qua. Không copy/chuyển đổi/ghi đè Data.

## Chưa hoàn thiện / bước sau

- Chưa geometry/pose typed-buffer API JS, skinning/normal transforms, animation interpolation/root motion hoặc WebGL upload/render.
- Chưa OZT/OZJ texture decoder portable/resolver/alpha/blend/light/shader/effect; chưa thử hình ảnh game.
- Chưa sweep parser mới toàn285 manifest hoặc5034 model; kết quả audit cũ không tự coi là parser mới qua.
- Chưa HTTP/browser asset loading hoặc GUI game; hiện chỉ canvas kiểm tra + Debug overlay.
- Provider/profile/key provisioning và MU network, FPS/memory/soak chưa đạt. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc sửa native production/config/resource gốc.

Bước sau: xuất geometry/pose buffers có giới hạn từ model và kiểm numeric trước WebGL, rồi giải mã texture gốc để render model đầu tiên từ BMD.
