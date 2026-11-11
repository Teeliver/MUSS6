# W0022 — Session đăng nhập

## Phạm vi implementation

`web/client/src/session/account-session.js` ghép establishGameIdentity W0020 và account codec W0021. Source wire/result giữ nguyên theo GameServer/Protocol.cpp, JSProtocol.cpp và native/client/client.cpp; không sửa server. API `establishAccountSession(options, policy)` chờ đến identity-sent rồi trả handle ở ready-for-login. Caller phải cung cấp profile/key/identity provider như các đợt trước.

`login(input)` kiểm input qua codec, tạo một packet C3 F1:01 và gửi qua channel. Chỉ một lần gửi login hợp lệ trên mỗi kết nối. Input sai trước gửi được từ chối nhưng vẫn cho sửa input; sau khi đã gửi không retry tự động. Chính sách một lần là giới hạn adapter hiện tại, không phải tuyên bố source cấm mọi lần thử tiếp. Server gốc có LoginMessageCount và mã 8; muốn hỗ trợ lại trên cùng kết nối cần đối chiếu vòng retry trước khi mở rộng.

Trạng thái: ready-for-login → awaiting-login → authenticated nếu response đúng schema và code=1. Nếu mã khác, trả `{code, accepted:false}`, phát login-rejected rồi đóng phiên; giữ raw code để UI sau này lấy text đúng Data/Language. Timeout, hủy, lỗi gửi/queue, malformed response hoặc peer close từ chối pending promise và đóng phiên. Không lấy socket-open/identity-sent làm bằng chứng account được chấp nhận.

Sau authenticated, packet được chuyển cho onPacket; send chỉ mở ở trạng thái này. Handle hiện là lớp protocol cấp thấp, chưa có allowlist opcode gameplay hoặc API nhân vật. Kết quả login và packet kế tiếp cùng chunk được xử lý theo thứ tự mà không mất packet. Callback lỗi làm phiên đóng. Timer/listener được dọn qua channel/handshake; AbortSignal tồn tại suốt lifetime.

Buffer login tạm được zero sau send; không lưu input/credential trong state session và không log packet. Không cam kết xóa string JS hoặc mọi bản sao core/transport/socket. API chưa nối UI, chưa nhận credential thật.

## Kiểm thử ngày 23/10/2026

```sh
npm --prefix web test
npm --prefix web run test:wasm
```

36/36 test qua, không skip; contract/bridge WASM qua. Bốn test session mới dùng WASM thật, key/profile/tài khoản tổng hợp và socket EventTarget mô phỏng. Không chạy MU server/DB hoặc mạng gateway cho session mới.

- Đúng thứ tự identity trước login, không gửi login hai lần, packet C3 đã qua crypto khôi phục đúng fixture.
- Response success + packet tiếp theo cùng chunk; chỉ code=1 authenticated; rejection 0/6/8/255 giữ raw code và đóng.
- Timeout, AbortSignal, malformed response, peer/local close từ chối request và không nhận kết quả muộn.
- Input sai không gửi, queue đầy, phase observer hoặc consumer exception đóng phiên và dọn timer.
- Test build Debug/Release kiểm allowlist/cú pháp module mới, UI giữ W0010.

## Chưa hoàn thiện / bước sau

- Chưa browser/MU login thật, gateway network integration với account session hoặc credential/profile provisioning.
- Identity provider thật vẫn thiếu. Không bypass ClientVerify, blacklist/count hoặc verify timeout.
- Chưa UI thông báo theo Language, retry gốc, account login menu hoặc danh sách nhân vật. Một packet bất ngờ lúc đang chờ login hiện làm phiên đóng; cần dispatcher cho notice/close packet sau khi đối chiếu source và test thật.
- Chưa phiên bản compatibility policy, soak/load hoặc benchmark gameplay; M02/M03 tiếp tục PARTIAL.
- Không đổi Data/config/native production hoặc bật MU server/DB.

Bước sau: tách request/response danh sách nhân vật đúng layout EX603 và kiểm chứng bằng fixture, rồi ghép sau account accepted. Không giả lập scene hoặc danh sách thành giao diện hoàn thiện.
