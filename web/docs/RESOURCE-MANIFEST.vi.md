# W0029 — Manifest model đúng case và hash

## Đã triển khai

`web/tools/resource_manifest.py` đọc domain audit W0028, tìm đúng tên file Data theo case-insensitive key nhưng từ chối thiếu/collision, symlink ra ngoài root, model path ngoài Item/Player hoặc size0/>64MiB. Hash SHA-256 stream từng file và kiểm size/mtime không đổi trong lúc đọc. Canonical path được giữ để adapter lookup ổn định, actual path lưu đúng tên file trên Data; không rename/copy asset gốc.

Manifest schema1 chứa canonical/path/size/sha256, version là SHA-256 deterministic của entries JSON sort-keys. Đây là content version cho phạm vi manifest, không phải chữ ký chứng thực nguồn. Manifest phải đến từ nguồn provisioning tin cậy; runtime hiện chỉ validate format version/hash, chưa xác minh lại digest toàn document hoặc trust/signature.

`platform/assets/manifest.js` validate schema/path/size/hash/collision, chỉ cho sửa khác biệt case của cùng model identity; snapshot entry bất biến và resolve canonical sang URL exact-case dưới base HTTP(S) explicit. Không fallback filename hoặc model khác. Không fetch manifest/asset tự động, không làm cache hoặc verify bytes tải về trong đợt này. Dependency mới có trong allowlist Debug/Release, không chứa diagnostics.

## Kết quả thực 23/10/2026

Artifact ignored `web/.local-baselines/2026-10-23-w0029/item-manifest.json`:285 models,5.399.772 byte,11 case corrections. Đã đưa manifest thực qua JS validator và resolve toàn285 entry, không tải network. Input audit là mapping domain adapter native, chưa tất cả active Item.bmd và chưa texture dependencies.

```sh
python3 -m unittest discover -s web/tests -p test_resource_manifest.py -v
python3 web/tools/resource_manifest.py --data /Users/angelale/Downloads/MuClientEX603/Data --audit web/.local-baselines/2026-10-23-w0028/item-resources.json --output web/.local-baselines/2026-10-23-w0029/item-manifest.json
npm --prefix web test
```

2/2 Python test và50/50 Node/native/WASM-backed test qua, không skip; build Debug/Release allowlist/cú pháp qua. Test generator kiểm exact case/hash/determinism/content version change và missing/unsafe/symlink escape. JS kiểm ownership/path/remap/size/hash/collision/base URL. Không sửa C++/WASM trong đợt này nên không rebuild hoặc chạy lại test:wasm riêng; WASM-backed test dùng build W0028 hiện có.

## Trạng thái giao diện và phần thiếu

Hiện web chỉ có canvas kiểm tra + Debug overlay góc trái; CHƯA có giao diện game, scene chọn server/nhân vật hoặc gameplay. Các câu “UI chưa đổi” ở log trước có nghĩa chưa triển khai UI game, không phải đã có game UI rồi giữ nguyên.

- Chưa expose/stage asset qua HTTP, fetch loader streaming với byte cap/hash verification/AbortSignal hoặc cache lifecycle.
- Chưa model/texture decoder runtime, body armor class/tier mapping, renderer/animation/effect và UI.
- Chưa manifest provisioning/trust, MU network/provider/profile/key provisioning hoặc performance game.
- M02/M03 vẫn PARTIAL, M04+ chưa đạt. Không bật MU server/DB, không đổi native production/Data/config.

Bước sau: asset loader có giới hạn byte, abort và kiểm SHA-256 theo entry; kiểm bằng fixture trước khi nối model decoder/render vào canvas.
