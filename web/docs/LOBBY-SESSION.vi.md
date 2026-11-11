# W0024 — Session lấy danh sách nhân vật

## Implementation và nguồn

`web/client/src/session/lobby-session.js` ghép account session W0022 và character codec W0023. Theo native/client/client.cpp Session::characters và GameServer/DSProtocol.cpp DGCharacterListRecv, client gửi C1 F3:00 sau account accepted, server gửi list rồi policy DE:00. Không sửa packet source hoặc tự tạo nhân vật để điền danh sách.

`establishLobbySession(options, policy)` trả handle sau identity-sent, có login, characters, send, close và state/phase. `characters({signal})` chỉ mở khi account authenticated, chưa có roster request khác. Nó gửi một list request và chờ awaiting-roster → awaiting-roster-policy → roster-ready. Chỉ resolve khi parse thành công cả hai gói; result giữ metadata classCode raw từ list và policy riêng, classCode trong policy dùng cho chính sách tạo nhân vật sau này. Không coi list chưa đủ policy là ready.

Một deadline bao toàn bộ request/pair, không gia hạn sau list hoặc byte drip. Timer partial-frame ở transport vẫn hoạt động độc lập. Timeout, cancel đang chờ, packet sai/thứ tự sai, peer/local close, lỗi callback hoặc queue đóng phiên và reject request; không retry packet muộn trên cùng kết nối. AbortSignal đã aborted trước request thì không gửi/không đóng session. Session có thể refresh tuần tự sau khi request trước hoàn tất, phục vụ tạo/xóa sau này. Không có credential/roster cache lâu dài.

Callback onPacket nhận packet ngoài request pending; khi pending, adapter hiện đòi đúng list rồi policy, chưa phân phối notice/close xen giữa. `send` bị chặn lúc pending; ngoài lúc này vẫn chịu kiểm authenticated của account session. Đây là lớp protocol, chưa phải API gameplay có allowlist đầy đủ.

## Kiểm chứng 23/10/2026

```sh
npm --prefix web test
npm --prefix web run test:wasm
```

42/42 test qua, không skip; WASM contract/bridge qua. Bốn test mới dùng core WASM thật và socket mô phỏng: chạy đủ identity → login → list/policy, kiểm bytes request qua XOR/transport, giữ metadata/policy, không publish sớm, coalescing và packet tiếp theo, refresh, gửi trùng, đảo/thừa/sai packet, deadline cả pair, abort/peer/local close và kết quả muộn, callback/queue lỗi, hai session độc lập. Debug/Release allowlist và cú pháp module qua. Không có WS/TCP gateway hoặc MU network integration cho lobby mới.

## Chưa hoàn thiện / bước sau

- Chưa UI/render màn chọn nhân vật, decode charset → model/trang bị/effect, hoặc thao tác chọn/tạo/xóa.
- Chưa dispatcher notice/close xen kẽ, browser/MU interoperability hoặc provisioning key/profile/identity provider thật.
- Chưa Data/Language/font UI, soak/load hoặc benchmark gameplay; M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không đổi Data/config/native production, không dùng tài khoản thật.

Bước sau: đối chiếu/tách charset nhân vật sang class và trang bị theo source, giữ fixture regression để tránh lặp lỗi vũ khí/độ sáng ở màn chọn nhân vật. UI chỉ ghép sau khi dữ liệu này có hợp đồng rõ.
