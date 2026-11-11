# Chat, Whisper, Party — W0118 (2026-11-01)

## Nguồn đối chiếu

- `Main5.2/source/NewUICommandWindow.cpp`: `CommandParty` gửi `SendRequestParty(key)`; `CommandWhisper` đặt ID whisper rồi mở ô chat.
- `Main5.2/source/NewUIChatInputBox.cpp`: Enter gửi chat/whisper, xoá nội dung và đóng input; whisper gửi bằng `SendChatWhisper`, tự thêm dòng của mình vào chat log.
- `Main5.2/source/wsclient.cpp`: `ReceiveChat` phân loại `~` party, `$` gens, `@@` union, `@` guild, `#` GM; `ReceiveChatWhisper` đưa vào log màu whisper.
- `Main5.2/source/NewUIPartyListWindow.cpp`: danh sách 77×23 mỗi thành viên, thanh HP 10 nấc, nút X rời nhóm.
- `Main5.2/source/NewUICommonMessageBox.cpp`: hộp xin vào nhóm hiện tên + `GlobalText[425]`, OK gửi result 1, Cancel gửi result 0.
- `SOURCE/Source/Emulator/GameServer/Protocol.cpp` + `Party.cpp` + `Party.h`: layout packet và mã kết quả thực tế của server UPDATE603.

## Protocol đã port

| Packet | Chiều | Layout |
|---|---|---|
| `C1:00` | client→server | 10-byte name + message (tối đa 59 ký tự + null) |
| `C2:00` | server→client | broadcast chat, name tại offset 4 |
| `C1:02` | client→server | whisper: 10-byte target + message |
| `C1:0C` | server→client | `MsgNumber`; số 0 hiện `GlobalText[482]` ("No users") |
| `C3:40` | client→server | lời mời nhóm, index 2 byte, gửi mã hoá như `SendRequestParty` |
| `C3:41` | client→server | result 1 = đồng ý, 0 = từ chối; kèm key 2 byte, gửi mã hoá như `SendRequestPartyAnswer` |
| `C1:42` | server→client | result + count + mỗi thành viên 24 byte (name10, number, map, x, y, pad2, CurLife, MaxLife) |
| `C1:43` | server→client | thành viên bị mời ra/rời nhóm |
| `C1:44` | server→client | count + mỗi entry 1 byte: high nibble = số thứ tự, low nibble = 10 nấc HP |

Chat gửi đúng `Hero->ID`; whisper gửi đúng target ID. Cả hai đều chờ server echo/broadcast, trừ whisper được thêm ngay vào log như source.

## GUI đã làm

- **Chat log** nổi phía trên HUD, 60 dòng gần nhất, màu theo source:
  - chat thường `rgb(205,220,239)`, system `rgb(100,150,255)`, error đỏ;
  - whisper nền vàng chữ đen, party nền cyan, guild nền xanh lá, union nền vàng, gens nền xám, GM nền đen chữ vàng.
- **Ô chat**: Enter mở, Enter gửi, Esc đóng; hết nội dung thì đóng như source.
- **GUI lấy trực tiếp từ Data OZJ** theo `NewUIChatInputBox::LoadImages`:
  - nền `Interface/newui_chat_back.OZJ` (281×47)
  - icon kênh `newui_chat_normal_on/party_on/guild_on/gens_on` (27×26)
  - icon trạng thái `newui_chat_whisper_on/system_on/chat_on/frame_on`
  - ô nội dung tại `(72,30)` 176×14; ô target tại `(5,30)` 50×14
  - hàng nút tại `y=0`, các mốc `x=0/27/54/81/114/141/168/200` như source
- Chọn kênh chat bằng 4 nút đầu; gửi kèm tiền tố `~` party, `@` guild, `$` gens như `NewUIChatInputBox`.
- Nút whisper bật/tắt trạng thái block whisper; nút system chỉ hiện tin hệ thống; nút chat bật/tắt log; nút frame bật/tắt khung.
- **Whisper kiểu MU**: gõ `/Tên nội dung` rồi Enter. Ví dụ `/Seller hi`. Chat thường không có dấu `/`. Các lệnh dành riêng `/warp`, `/move`, `/trade`, `/exchange`, `/party`, `/post` vẫn gửi dạng chat để server xử lý.
- **`/whisper off` / `/whisper on`** xử lý cục bộ như `NewUIChatInputBox`: tắt thì bỏ qua whisper đến, bật lại thì nhận; hiện `GlobalText[267]/[268]`.
- Chọn lệnh Whisper trong Command Window rồi click người chơi sẽ tự điền sẵn `/Tên ` vào ô chat; click tên trong chat log cũng vậy.
- **Party**: chọn lệnh Party rồi click người chơi để gửi `C3:40`; kiểm tra trưởng nhóm như `CommandParty` và báo `GlobalText[257]` nếu mình không phải trưởng nhóm; người nhận thấy hộp tên + `GlobalText[425]`, OK/Cancel gửi `C1:41` result 1/0.
- **Party list**: mỗi thành viên 77×23, tên + thanh HP 10 nấc, nút X gửi `C1:43`; vị trí đặt bên trái panel túi đồ bên phải.
- **Kết quả nhóm**: hiện đúng các câu `GlobalText[497..502, 2990, 2997, 2998]` trong chat log.

## Giới hạn hiện tại

- Chưa port thanh 10 nút chat (kênh chat, F2/F3/F4, size/alpha, frame) và lịch sử chat/whisper. Whisper dùng cú pháp `/Tên nội dung` theo yêu cầu gameplay; source `NewUIChatInputBox` gốc dùng ô target riêng, web không hiển thị ô đó nữa.
- Chưa port `Party Info Window` đầy đủ (kéo thả, đổi thứ tự, cờ nhóm, tìm nhóm).
- Chat command `/`, `!`, `~`, `@`, `$` mới hiển thị phân loại; chưa port toàn bộ `NewUICommandFilter`.
- Trade, Guild, Guild Union, Guild Rival, Cancel Rival, Add Friend, Follow, Battle vẫn chưa làm trong lượt này.

## Kiểm tra

- `336/336` test pass.
- Test mới: layout `C1:00/02/40/41/42/43/44`, decode chat C1/C2, server message 0, request/list/life/leave cập nhật state, gửi chat/whisper/party đúng byte.
- Build debug 150 file.

## UTF-8 và giới hạn ký tự (2026-11-01)

- `Main5.2/WSclient.h`: `MAX_CHAT_SIZE = 90` **byte**, gồm cả null. `NewUIChatInputBox` giới hạn số ký tự nhập theo `90 / GetNumByteForOneCharUTF8()`, nên tiếng Việt có dấu bị giảm số ký tự tùy độ dài UTF-8.
- `GameServer/Protocol.h`: `PMSG_CHAT_RECV.message[60]`, server đặt `message[59]=0`, nên thực tế chỉ nhận **59 byte UTF-8** cho mỗi tin nhắn.
- Web đã chuyển packet chat/whisper sang **UTF-8**, tên nhân vật vẫn ASCII, giới hạn nội dung **59 byte** để không bị server cắt giữa ký tự. Tiếng Việt có dấu gửi/nhận được; khoảng 19–29 ký tự tùy dấu.
- Nếu vượt 59 byte, web báo lỗi và giữ nguyên ô nhập, không gửi packet.

## Định dạng tên chat theo source

- `NewUIChatLogWindow` render `ID + " : " + Text` (có dấu hai chấm).
- Không in đậm tên; tên dùng cùng màu với nội dung theo loại message.
- Party: nền cyan, chữ đen; Whisper: nền vàng, chữ đen; chat thường: chữ `rgb(205,220,239)`.
- CSS `.mu-chat-name` đổi thành `color:inherit; font-weight:inherit`.
- `appendChat` nối tên bằng `" : "`.
