# W0020 — Greeting và gửi ClientVerify identity

## Source và phạm vi

Main_EX603/Main/Protocol.cpp, GCConnectClientRecv: nhận greeting rồi tạo F3:09 và DataSend HardwareId. GameServer/HidManager.cpp quyết định ClientVerify; source không có ACK riêng. Tham chiếu format/provider: [CLIENT-IDENTITY](CLIENT-IDENTITY.vi.md); crypto channel: [GAME-CHANNEL](GAME-CHANNEL.vi.md).

`web/client/src/session/game-handshake.js` cung cấp `establishGameIdentity(options, policy)`. Options là core/profile/key/route/socket explicit của channel; policy có identityProvider, AbortSignal, callback onPhase/onPacket, clock và phaseDeadlineMs. Provider được truyền AbortSignal nội bộ để có thể hủy công việc; provider không hỗ trợ hủy vẫn bị bỏ qua nếu trả kết quả muộn.

Luồng: connecting → awaiting-greeting → awaiting-identity → identity-sent. Chỉ sau greeting đúng schema/result/server code mới gọi provider, mã hóa F3:09 và gửi qua channel. Promise resolve trả greeting và handle send/close/state/phase sau khi gửi identity. Không gọi trạng thái này là verified/authenticated: WebSocket gửi thành công không chứng minh server đã chấp nhận. Chưa kiểm version match; encodedVersion giữ raw theo W0015.

Deadline riêng được đặt lại ở bước connect, chờ greeting và chờ provider; không đặt thời hạn chờ ACK không tồn tại. Timeout, AbortSignal, key/transport/provider/greeting lỗi hoặc callback phase lỗi đóng phiên và dọn timer/listener. Abort sau khi resolve vẫn đóng handle. Packet sau identity-sent được chuyển cho callback; callback lỗi làm channel đóng. Trong giai đoạn provider còn chờ, packet khác bị từ chối để không làm rơi packet hoặc cho login chạy sớm. Chính sách chặt này chưa được kiểm chứng với server MU đang chạy, có thể cần dispatcher có giới hạn cho thông báo sớm sau khi đối chiếu trace thật.

Identity provider bắt buộc, không random/localStorage fallback. Chuỗi identity chỉ dùng tạm; buffer packet gửi được xóa sau send hoặc khi kết quả provider đến muộn. Không cam kết xóa string JS bất biến hoặc mọi bản sao WASM. Lỗi provider trả ra ngoài dùng thông báo chung, không đưa chi tiết provider/identity vào log.

## Kiểm thử ngày 23/10/2026

```sh
npm --prefix web test
npm --prefix web run test:wasm
```

30/30 test qua, không skip; WASM contract/bridge qua. Năm test handshake mới dùng WASM thật và socket mô phỏng: greeting chia nhỏ; không gọi provider trước greeting; F3:09 qua transport/XOR đúng byte; greeting sai result/code/schema; thiếu/provider lỗi/format sai; timeout/hủy và provider trả muộn; packet đến sớm; observer lỗi; remote close và abort sau khi đã gửi identity. Không có HWID thật, tài khoản hoặc server MU tham gia.

Test trước W0019 giữ nguyên sau khi gom core/key/socket tổng hợp vào tests/helpers/game-fixture.mjs. Debug/Release build allowlist và cú pháp module đều qua; không đổi UI.

## Chưa hoàn thiện và bước sau

- Browser/native identity provider và provisioning profile/Enc1/Dec2 thật vẫn chưa có.
- Chưa login codec/state machine, account, menu runtime hoặc MU network integration; chưa browser test handshake, soak hoặc performance gameplay.
- Chưa có bằng chứng server chấp nhận identity. Không bypass ClientVerify/blacklist/count/timeout.
- Không bật MU server/DB, không đổi Data/config/native production. M02/M03 tiếp tục PARTIAL.

Bước sau có thể làm độc lập: đối chiếu packet đăng nhập và phản hồi theo source, tách codec bằng fixture không credential, trước khi nối UI hoặc dùng tài khoản thật.
