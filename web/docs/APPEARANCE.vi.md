# W0025 — Giải mã CharSet class và bảy slot trang bị

## Source và phạm vi

Server GameServer/DSProtocol.cpp DGCharacterListRecv đóng gói CharSet18 byte; adapter native/client/character_resources.cpp đã có phép giải mã class/tier, armor/weapon và bảng visual grades. Main_EX603 hiện là DLL extension, không có toàn bộ engine render Windows trong bộ source đã khảo sát. Không coi việc có parser là đã khôi phục renderer gốc.

Core `client-core/include/mu/appearance.hpp` và adapter `web/client/src/protocol/appearance-codec.js` giải mã baseClass0–6, tier0–2 và bảy slot: hai vũ khí, helm/armor/pant/glove/boot. Weapon index 12 bit, sentinel4095; armor type9 bit sentinel511, item group7–11. Slot index giữ nguyên, không đổi tay Elf theo suy đoán model. Mỗi part có present/index/visualLevel và boolean excellent/ancient. Raw18byte được copy để còn xử lý wing/pet/custom sau này.

VisualLevel là cấp hiển thị từ mã3bit với bảng 0,3,5,7,9,11,13,15 theo native; KHÔNG phải level item chính xác. Hai cấp có thể cùng grade. Excellent/ancient chỉ có cờ tồn tại, không có option bitmask hoặc giá trị đầy đủ; không tạo option giả. Vắng item luôn present=false và không bật hiệu ứng từ bit rác.

Đối chiếu source cho thấy vòng đóng gói level/flag bỏ qua cả item index511 lẫn65535. Với weapon index511, id vẫn có thể được truyền nhưng level/flag không được ghi. Fixture giữ đúng hiện tượng này; không tự sửa server hoặc dựng grade15 chỉ vì input fixture là +15. Byte class/tier được đối chiếu phép tách native; test source-reference dưới đây tập trung vào equipment packer.

## Kiểm thử

```sh
npm --prefix web run build:wasm
npm --prefix web test
npm --prefix web run test:wasm
```

Source reference trích nguyên đoạn packing bảy slot trong DGCharacterListRecv, chạy trong namespace fixture. Sweep mọi id0..4095 cho mỗi tay và0..511 cho mỗi phần giáp, từng cấp0..15, cờ excellent/ancient và sentinel. Decode được kiểm lại với dữ liệu nguồn packer, không chỉ encode/decode cùng implementation. Contract chạy native và WASM. Test class/tier0–6/0–2, size sai, class ngoài phạm vi; bridge JS kiểm index/grade/flags và raw buffer ownership. Initial fixture giả định mọi weapon id nhận level/flag đã được sửa cho trường hợp511 đúng source; không đổi decoder để che khác biệt.

Kết quả cuối: build WASM qua, 44/44 test và test:wasm qua, không skip. Debug/Release allowlist/cú pháp module mới được kiểm qua npm test; UI không tự thay đổi.

## Chưa hoàn thiện

- Chưa wing/pet/custom charset decoding, full item options, model catalog hoặc hiệu ứng render/shader.
- Chưa nối appearance vào scene chọn nhân vật hoặc trong game; chưa kiểm tay/animation/effect bằng hình ảnh.
- Chưa MU/browser network, key/profile provisioning hoặc identity provider thật, performance/soak.
- Không copy/move Data, không sửa native production/config hoặc bật MU server/DB. M02/M03 còn PARTIAL.

Bước sau: giải mã phần wing/pet và liệt kê trường custom cần catalog; sau đó mapping model/resource có kiểm chứng. Không hardcode hình đại diện thay dữ liệu gốc.
