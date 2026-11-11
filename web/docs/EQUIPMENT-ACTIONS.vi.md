# W0042 — Tư thế theo vũ khí và Item.bmd gốc

## DONE

- `client-core/include/mu/equipment_actions.hpp` port nhánh `equipped_actions` từ native character_resources.cpp: class idle/walk/run, vũ khí một/hai tay, special sword, spear, staff và bow/crossbow theo hand occupied. Sentinel-1 là không có món. Policy không thay equip eligibility.
- `item_two_hand` đọc trực tiếp field30 trong record84byte của Item.bmd EX603, XOR theo row và kiểm checksum toàn bảng688132byte trước khi dùng. Đây là reader hẹp cho twoHand, không phải port toàn bộ ItemCatalog.
- Bridge `equipmentActionsResult` nhận class/right/left và bytes Item.bmd; trả idle/walk/run/twoHand. Không đoán twoHand theo filename hoặc hardcode Sword01. Native field reference với Item_eng.bmd thật xác nhận Sword01 twoHand=false và5index mẫu khớp decoder native.
- Manifest chỉ mở thêm đúng `Local/Eng/Item_eng.bmd`, không cho mọi đường dẫn Local. Stage thêm1file gốc nguyên bytes/hash, tổng163asset; không chuyển đổi Data hay xuất config/key. Test manifest kiểm tra allowlist này.
- Thân và attached Sword01 dùng cùng action. Ngoài vùng an toàn, Sword01 tay phải dùng idle4 theo bảng, safe/back giữ base idle1/2/3. Left-only sword giữ base action theo đúng nhánh native hiện có; không tự đổi thành right-hand pose.
- Contract biên dịch trực tiếp hàm native với fixture catalog/character nhỏ và so573.510tổ hợp class/right/left/twoHand. Test WASM kiểm bảng checksum hợp lệ/corrupt, trường twoHand và single/double pose. Contract portable đưa vào CMake và test:wasm.

## PARTIAL

- Preview vẫn chỉ Sword01, frame0. Chưa animation interpolation, chọn walk/run từ gameplay, attack stance, equip online/CharSet/inventory. Hai tay true và bow/staff action đã reference-test nhưng chưa có geometry visual fixture từng loại.
- Field reader chỉ lấy twoHand, không có tên/requirements/class eligibility. Hiện đọc bảng Eng cụ thể cho technical fixture, chưa có locale selection.
- Skin dưới mũ/wing nền đen/hide, Windows visual parity và gamecamera/GUI/network chưa xong. RF idle chưa xác minh. Chưa profile frametime dài để kết luận60FPS.
- Không bật MU server/DB; Data/native production/config không sửa. M02/M03 vẫn PARTIAL.

## Validation

Kết quả cuối npm/WASM/browser ghi STATUS.vi.md. Baseline `.local-baselines/2026-10-24-w0042/` giữ native field log và ảnh pose. Bước sau là animation theo clip với tốc độ nguồn, giữ thân/weapon đồng bộ và đo chi phí cập nhật.

Kết quả cuối:80/80 npm test, build WASM + test:wasm và2Python manifest test qua. Browser DW action4 xác nhận thân/kiếm cùng pose, screenshot sword-stance.png.163asset staged Debug. Checksum Item.bmd theo native không phải hash bảo mật; SHA256 manifest tiếp tục kiểm file tải.
