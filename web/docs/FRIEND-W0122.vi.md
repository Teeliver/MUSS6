# Add Friend / Friend List / Private Message — W0122 (2026-11-02)

## Nguồn

- Client: `Main5.2/source/UIWindows.cpp` (`CFriendList`, `CUIFriendListTabWindow`), `wsclientinline.h`.
- Server: `GameServer/Friend.cpp/.h`, `Protocol.cpp`.
- Text: `Local/Eng/Text_eng.bmd` các ID 1018, 1019, 1021, 1022, 1023, 1024, 1047–1052, 1068, 1075.

## Protocol

| Packet | Chiều | Layout |
|---|---|---|
| `C1:C0` | client→server | xin friend list, 3 byte |
| `C2:C0` | server→client | wide list: mailCount, mailTotal, count, mỗi entry name10 + server1 |
| `C1:C1` | client→server | thêm bạn, name10 |
| `C1:C1` | server→client | result + name10 + server (15 byte) |
| `C1:C2` | server→client | lời mời kết bạn, name10 (13 byte) |
| `C1:C2` | client→server | result + name10 (14 byte) |
| `C1:C3` | client→server | xoá bạn, name10 |
| `C1:C3` | server→client | result + name10 (14 byte) |
| `C1:C4` | server→client | trạng thái online/offline: name10 + server |

- `server >= 0xFD` = offline theo `CFriendList::UpdateFriendState`.
- Add result:
  - 1 thành công
  - 0 ID không tồn tại
  - 3 không thêm được nữa
  - 4 đã có trong danh sách
  - 5 không thể tự thêm mình
  - 6 nhân vật phải trên cấp 6

## GUI

- Panel `Friends` 190×429, bên phải.
- Hai nút đầu:
  - `GlobalText[1018]` Add Friend
  - `GlobalText[1019]` Delete Friend
- Header:
  - `GlobalText[1021]` Friend's Name
  - `GlobalText[1022]` Server
- Danh sách hiển thị:
  - tên
  - `[Online]` nếu server < 0xFD
  - `[Offline]` nếu server >= 0xFD
- Click chọn; click lần 2 vào người đã chọn → mở chat whisper tới người đó.
- Xoá bạn có xác nhận `GlobalText[1024]`.
- Thêm bạn dùng hộp nhập tên, gợi ý `GlobalText[1023]`.
- Lời mời kết bạn dùng modal riêng, không bị `closeSale()` xoá:
  - `<Tên> GlobalText[1051]`
  - Đồng ý gửi result 1, Huỷ gửi result 0.

## Trạng thái

- Đã có: add friend, list, delete, accept/decline, online/offline, whisper riêng.
- Chưa port: mail/letter (Write/Reply/Delete/Prev/Next), chat room riêng, sort cột, giới hạn 50 mail.
- Test: **344/344 pass**, build debug **152 file**.

## File chính

- `web/client/src/protocol/friend.js`
- `web/client/src/protocol/game-state.js`
- `web/client/src/session/world-session.js`
- `web/client/src/ui/game-ui.js`
- `web/tests/friend.test.mjs`

## Sửa nút Add Friend

- Nút Add Friend chuyển sang dùng `nativePopupFields` thay vì popupFields thường, tránh trường hợp hộp nhập không hiện.
- Ô nhập dùng gợi ý `GlobalText[1023]`.
- Tên chỉ nhận 1–10 ký tự Latin.

## Bỏ Ctrl + Click mở shop

- Đã xoá toàn bộ nhánh `ctrlHeld/e.ctrlKey` mở personal shop trong `world-controls.js`.
- Bỏ cursor `shop` khi giữ Control.
- Bỏ Ctrl + right-click mở shop.
- Không còn hỗ trợ Windows Ctrl + click để mở shop; chỉ còn click trực tiếp/name shop hoặc lệnh.

## Add Friend result code 5

- Theo client source `ReceiveAddFriendResult`:
  - code 0: `GlobalText[1047]` ID does not exist
  - code 3: `GlobalText[1048]` cannot add more
  - code 4: `GlobalText[1049]` already registered
  - code 5: `GlobalText[1050]` **cannot register your own ID**
  - code 6: `GlobalText[1068]` other character must be over level 6
- Đã thêm kiểm tra trước khi gửi: nếu tên nhập trùng `selfName` thì báo `GlobalText[1050]`, không gửi packet.
- Add Friend qua Command Window dùng mode `addfriend`: click người chơi → gửi `C1:C1` tên người đó.

## Bỏ Ctrl + Click

- Đã xoá toàn bộ nhánh Ctrl/Ctrl+click mở shop trong `world-controls.js` và cursor shop.
- Chỉ còn click trực tiếp tên/người bán hoặc lệnh Buy.

## Sửa lỗi Add Friend thật sự (2026-11-02)

### Nguyên nhân
- Database `MuOnline` chỉ có bảng `T_FriendMain`, `T_FriendList`, `T_FriendMail`, `T_WaitFriend`, `T_CGuid` nhưng **thiếu toàn bộ stored procedure**:
  - `WZ_UserGuidCreate`
  - `WZ_WaitFriendAdd`
  - `WZ_WaitFriendDel`
  - `WZ_FriendAdd`
  - `WZ_FriendDel`
- Vì vậy `DataServer/Friend.cpp` gọi procedure thất bại, `GetResult(0)` trả 0 và client luôn nhận code 0 (`ID does not exist.`) dù tên hợp lệ.

### Đã thêm
- `native/runtime/mysql/migration/friend-procedures.sql`
- Result đúng contract của `DataServer/Friend.cpp`:
  - Add: 0 không tồn tại, 1 đã gửi lời mời, 3 danh sách đầy, 4 đã là bạn, 5 tự thêm mình, 6 nhân vật dưới cấp 6, 8 lời mời chéo (tự chấp nhận).
  - Delete: 1 xoá thành công, 0 không tìm thấy.
- Cần chạy file SQL bằng user có quyền `CREATE ROUTINE` (root), sau đó `mu_native` chỉ cần `EXECUTE`.

### Thông báo lỗi
- Map đúng `GlobalText` như client gốc:
  - 0 → 1047 `ID does not exist.`
  - 3 → 1048 `You cannot add more...`
  - 4 → `<Tên> is already registered.` (1049)
  - 5 → 1050 `You cannot register your own ID.`
  - 6 → 1068 `The other character must be over level 6.`
  - 1 → 1075 trạng thái Offline cho tới khi cả hai đăng ký.
- Xoá bạn thất bại hiện 1052.

### Friend list lúc vào game
- `world-session` gửi `C1:C0` ngay sau khi nhận entry ACK, giống client gốc gọi `SendRequestFriendList()` khi kết nối.

## GUI Bạn bè theo `CUIFriendWindow` + `CUIFriendListTabWindow`

- Cửa sổ **250×170**, khung/titlebar cổ điển:
  - `win_titlebar.jpg` cho thanh tiêu đề.
  - `win_size.jpg` cho nút thu nhỏ / phóng to / đóng.
  - `win_resize.tga` góc resize.
  - `win_button.tga` cho 4 nút dưới.
  - `win_check.tga` cho checkbox.
- Tiêu đề: `GlobalText[990]` = **My Friend**.
- Tab 53×19:
  - `GlobalText[1032]` Friends List
  - `GlobalText[1033]` Window List
  - `GlobalText[1034]` Letter Box
- Góc phải tab: `GlobalText[1035]` **Refuse Chat** + checkbox 9×9.
- Header danh sách:
  - `GlobalText[1021]` Friend's Name
  - `GlobalText[1022]` Server
  - Click header để đổi kiểu sắp xếp.
- Mỗi dòng cao **13px**, đúng `CUIChatPalListBox`:
  - Tên ở cột trái.
  - Cột server:
    - `GlobalText[1039]` Offline khi server `0xFC/0xFE/0xFF`
    - `GlobalText[1041]` Cannot Use khi `0xFD`
    - còn lại `%2d Server` với `server + 1`
  - Dòng được chọn: nền trắng mờ, chữ đen.
  - Double click / click lần 2: mở whisper.
- 4 nút dưới, 50×14, đúng thứ tự nguồn:
  - `GlobalText[1018]` Add Friend
  - `GlobalText[1019]` Delete Friend
  - `GlobalText[1020]` Chat
  - `GlobalText[1015]` Write (chưa hỗ trợ thư)
- Add Friend dùng hộp nhập với `GlobalText[1023]`.
- Xoá dùng xác nhận `GlobalText[1024]`.

### Asset mới trong manifest
- `Interface/win_titlebar.OZJ`
- `Interface/win_button.OZT`
- `Interface/win_check.OZT`
- `Interface/win_size.OZJ`
- `Interface/win_resize.OZT`
- `Interface/win_scrollbar.OZJ`

### Hộp nhập Add Friend
- Dựng lại `CUITextInputWindow` 150×100:
  - `win_titlebar.jpg` + tiêu đề `GlobalText[1023]`.
  - Input 80×14 tại (30,14) trong vùng nội dung.
  - Nút `GlobalText[228]` OK và `GlobalText[229]` Cancel, 50×20, dùng `win_button.tga`.
  - Enter = OK.

### Rà soát GUI theo ảnh test (2026-11-02)

- Sửa tab bị trống chữ: `Friends List` / `Window List` / `Letter Box` giờ lấy đúng `GlobalText[1032/1033/1034]`.
- Sửa header bị trống chữ: `Friend's Name` / `Server` lấy đúng `GlobalText[1021/1022]`.
- Nút dưới không còn tự xuống dòng; dùng `white-space:nowrap`, cỡ 8px như nút cổ điển.
- Checkbox `Refuse Chat` dời sang trái nhãn, không đè lên tab thứ 3.
- Cửa sổ mặc định mở tại `(50,50)` như `CUIFriendWindow::SetPosition(50,50)` và **kéo được bằng thanh tiêu đề** (window moveable của client gốc).
- Vị trí không còn phụ thuộc vào cạnh phải Inventory; người dùng có thể kéo tới góc mong muốn.

### Giới hạn còn lại

- Tab `Window List` (chat room) và `Letter Box` (mail) mới có giao diện, chưa có chức năng vì server/web chưa port chat room + mail.
- `Refuse Chat` là checkbox hiển thị theo source; emulator hiện không xử lý `C1:C4` chat state nên chưa gửi packet.
- Nút `Write` báo chưa hỗ trợ thư.
- Các phần còn lại: list, sort, add/delete, chat/whisper, online-offline, lời mời kết bạn đã chạy theo packet gốc.

## Hoàn thiện Window List / Refuse Chat / Write (W0124)

### Write — thư (mail)
- Protocol mới `client/src/protocol/mail.js`:
  - `C2:C5` gửi thư: window DWORD, Name[10], Title[60], action, direction, mailsize, Mail[1000].
  - `C1:C9` xin danh sách thư.
  - `C1:C7` đọc thư (index).
  - `C1:C8` xoá thư (index).
  - Nhận `C1:C5` kết quả gửi, `C3:C6`/`C1:C6` thông báo thư, `C3:C7`/`C1:C7` nội dung, `C1:C8` kết quả xoá.
  - UTF-8 cho tiêu đề/nội dung; tên nhân vật Latin 10 byte.
- DB `migration/friend-procedures.sql` thêm:
  - `WZ_WriteMail`: kiểm tra người nhận, level >= 6, không tự gửi, hộp thư < 50, cấp MemoIndex > 10, tăng `MemoTotal`.
  - `WZ_DelMail`: xoá theo GUID + MemoIndex, giảm `MemoTotal`.
- Result gửi thư map đúng client:
  - 0 → 1053, 1 → 1046 (1000 zen), 2 → 1061, 3 → 1064,
  - 4 → 1065, 6 → 1068, 7 → 423.
- UI Letter Box theo `CUILetterBoxTabWindow`:
  - Cột `GlobalText[1028]` Sender, `[1029]` Date, `[1030]` Title.
  - Nút Write / Read / Reply / Delete: `1015/1027/1010/1011`.
  - Double-click hoặc Read để mở thư.
- UI Write theo `CUILetterWriteWindow` 250×216:
  - `GlobalText[1071]` tiêu đề + cost 1000 zen.
  - Receiver `GlobalText[1000]`, Title `[1005]`.
  - Textarea 238×134.
  - Send `[1001]`, Close `[1002]`.
- UI Read theo `CUILetterReadWindow` 250×182:
  - `GlobalText[1054]` tiêu đề thư, sender/date.
  - Nội dung cuộn 238×118.
  - Reply/Delete/Previous/Next/Close: `1010/1011/1012/1013/1002`.
  - Reply tự điền `Re: <tiêu đề>`.
- Xoá thư có xác nhận `GlobalText[1017]`.

### Window List + chat riêng
- Emulator **không có ChatServer 55980** và GameServer không xử lý `C1:CA/CB`, nên không thể dùng chat-room protocol gốc.
- Đã triển khai chat riêng 1–1 trên nền whisper có sẵn:
  - Double-click friend hoặc nút Chat mở cửa sổ `Talking: <tên>`.
  - Gửi/nhận bằng `whisper` hiện có; cửa sổ có log, input, kéo được, nút đóng.
  - Nhiều cửa sổ cùng lúc.
- Tab Window List theo `CUIChatRoomListTabWindow`:
  - Header `GlobalText[1026]` Window Title.
  - Liệt kê các cửa sổ đang mở; click để ẩn/hiện.
  - Nút `GlobalText[1025]` Hide All.
- Ghi chú: đây là thay thế tương thích UI, không phải packet chat room gốc.

### Refuse Chat
- Checkbox `GlobalText[1035]` hoạt động:
  - Khi bật: hỏi xác nhận `GlobalText[1036]` Yes/No (`1037/1038`), đóng toàn bộ cửa sổ chat.
  - Khi đang bật: whisper đến vẫn vào chat log nhưng không tự mở cửa sổ chat.
  - Tắt: cho phép mở lại.
- Client gốc gửi `C1:C4` chat state; emulator không có ChatServer nên trạng thái này được giữ phía client.

### Test
- Thêm `tests/mail.test.mjs`: packet layout, decoder C1/C3, session list/read/delete/send.
- Tổng **347/347 pass**, build debug **153 file**.

## Rà soát lại Letter Box + Write theo Source (W0125)

### Letter Box — `CUILetterListBox`
- Thêm cột checkbox chọn thư và **chọn tất cả** ở header, dùng `win_check.tga`.
- Thêm icon thư đã đọc / chưa đọc `win_mail.tga` (13×9 chưa đọc, 13×10 đã đọc).
- Header đúng thứ tự source:
  - checkbox + icon thư ở x=1/11
  - `GlobalText[1028]` Sender
  - `GlobalText[1029]` Date Rcvd.
  - `GlobalText[1030]` Title
- Ngày hiển thị `m_szDate + 2` như source (bỏ 2 ký tự đầu của năm).
- Dòng chọn nền trắng 70%, chữ đen; đọc/chưa đọc phân biệt bằng icon, không đổi màu chữ.
- Xoá thư dùng **các dòng đã tick**:
  - Không tick → `GlobalText[1031]`
  - Có tick → xác nhận `GlobalText[1017]` rồi gửi xoá từng thư.
- Cuộn danh sách bằng con lăn chuột (6 dòng/lần).

### Write / Read — kích thước theo source
- Write: **370×216** (250 nội dung + 120 vùng ảnh nhân vật).
  - Receiver `1000`, Title `1005`, input 238×14.
  - Textarea 238×134.
  - Send `1001` tại (12,200), Close `1002` tại (63,200).
  - Vùng ảnh nhân vật 114×147 tại (251,30).
- Read: **370×182**.
  - Nội dung 238×118.
  - Reply/Delete/Previous/Next/Close đúng toạ độ source `2/53/104/145/186`.
  - Vùng ảnh nhân vật 114×138 tại (251,20).
- Reply dùng `GlobalText[1016]` = `Re: %s`.
- Xoá thư đơn dùng xác nhận `GlobalText[1017]`.

### Ảnh nhân vật
- Nối `CPhotoViewer` với `live-view.portrait(canvas)`:
  - Dựng lại body hiện tại từ `lastEquipment` + `currentWorld`.
  - Render một frame vào canvas 114×147 (Write) / 114×138 (Read).
  - Nếu chưa đủ model/texture thì khung vẫn giữ đúng layout và canvas tự bỏ.

## Sửa disconnect khi gửi thư (W0126)

### Nguyên nhân
- `HACK_LOG` báo:
  `Packet encryption error (Index: 197, Value: -1, Encrypt: [0][1])`
- `Data/Hack/HackPacketCheck.txt` yêu cầu packet `0xC5` (gửi thư) **phải mã hoá**.
- `PacketCipher.encode` chỉ mã hoá khi header gửi là `0xC3` hoặc `0xC4`.
- Trước đó client gửi `C2:C5` nên bị coi là không mã hoá → GameServer `CloseClient`.

### Đã sửa
- `mailSendPacket` đổi header wide từ `0xC2` sang **`0xC4`**.
- `PacketCipher` sẽ mã hoá, xuất ra `C4` với serial; GameServer giải mã thành `C2:C5` và HackPacketCheck nhận `Encrypt = 1`.
- Kích thước plain vẫn khai báo đúng ở byte 1–2 trước khi mã hoá.
- Test codec cập nhật header `0xC4`.

Test **347/347 pass**, build **153 file**.

## Đồng bộ tỉ lệ cửa sổ Write/Read với UI game (W0127)

- Trước đó Write/Read dùng kích thước CSS cố định 370×216 / 370×182 nên trên màn hình lớn trông nhỏ hơn nhiều so với cửa sổ gốc.
- Đã thêm `transform: translate(-50%,-50%) scale(var(--mu-scale))` để cửa sổ phóng theo đúng tỉ lệ UI của game, giống các panel Inventory/Friend.
- Ảnh nhân vật trong Write/Read là **nhân vật của chính bạn** (`CopyPlayer` trong source), không phải người nhận.

## Thông báo thư mới + icon thư chưa đọc (W0128)

Theo `ReceiveLetter` + `CUIFriendMenu::RenderFriendButton`:

- `WZ_WriteMail` giờ insert `bRead = 2` (thư mới chưa thông báo).
  - Bảng `T_FriendMail` đã nới CHECK thành `bRead IN (0,1,2)`.
  - Đọc thư → DataServer set `bRead = 1`.
- Khi client nhận `C3:C6` có `read = 2`:
  - Thêm thư vào danh sách.
  - Gửi chat system `GlobalText[1062]` “New mail has arrived.”
  - Phát `SOUND_FRIEND_MAIL_ALERT`.
  - Bật icon phong bì trên HUD.
- Icon phong bì `win_letter.jpg` (15×9) vẽ trên HUD khi còn thư `bRead != 1`.
- Icon trong danh sách:
  - `bRead == 1` → `win_mail` crop đã đọc 13×10
  - `bRead == 0/2` → crop chưa đọc 13×9
- Mỗi thư chỉ thông báo một lần trong phiên (`mailAlerted`), tránh spam khi mở lại Letter Box.

Test **347/347 pass**, build **153 file**.

## Fix đọc thư bị kẹt “Đang tải nội dung thư…” (W0129)

### Nguyên nhân
- `PMSG_FRIEND_MAIL_READ_SEND` dùng `PWMSG_HEAD` nên GameServer gửi `C4:C7`.
- `PacketCipher.decode` chuẩn hoá `C4` → **`C2`** (wide), head nằm ở byte 3, không phải `C1` head ở byte 2.
- `decodeMailText` cũ chỉ nhận `C1/C3` và đọc head ở byte 2 → bỏ qua gói `C2:C7`.
- Vì vậy DataServer đã set `bRead=1` nhưng client không mở được nội dung.

### Đã sửa
- `decodeMailText` nhận cả `C2`; head = `b[0]===0xC2 ? 3 : 2`.
- Giữ nguyên offset index (4), mailsize (6), action (26), direction (27), text (28) vì layout wide vẫn đặt index tại byte 4.
- Thêm test gói `C2:C7`.

## Ảnh nhân vật trong Write
- Nếu portrait lỗi, client hiện notice `Ảnh nhân vật: <lỗi>` thay vì im lặng.
- Portrait dựng lại body hiện tại; nếu vẫn lỗi, gửi mình đúng dòng notice để xử lý tiếp.

## Test end-to-end bằng Playwright (W0130)

Đã dựng 2 nhân vật test `PWTest` / `PWRecv`, đăng nhập thật qua gateway, vào map, gửi và đọc thư.

### Kết quả test
- Gửi thư: `Letter has been sent (cost: 1000 zen)`, DB có `bRead=2`, `Memo` dài 1000 byte.
- Nhận thư khi login: chat hiện `New mail has arrived.`
- Letter Box hiện đúng dòng: sender, ngày `26-11-01`, title, icon chưa đọc.
- Bấm Read: mở cửa sổ `Read letter: Test subject`, nội dung hiển thị đúng, DB chuyển `bRead=1`.
- Ảnh nhân vật trong Write/Read render đúng (đọc pixel canvas: 5668 pixel đục).

### Lỗi đã sửa thêm
1. **Mail read `C2:C7`**: head ở byte 3, decoder chỉ nhận C1/C3 → sửa.
2. **Portrait crash** `Cannot read properties of undefined (reading 'kind')`:
   - `sampleResult()` không mang `mesh.image`.
   - Chuyển sang `actor.upgrade.apply(actor.model)` để giữ texture đã nạp.
3. **Gửi thư tới nhân vật chưa từng mở Friend**: `WZ_WriteMail` tự tạo `T_CGuid` + `T_FriendMain` cho người nhận.
4. **Text canh giữa sai**: Write/Read đặt `text-align:left`, input left, nội dung thư left.
5. **Notice “Đang tải nội dung thư…” còn treo**: xoá khi cửa sổ Read mở.
6. **Thông báo thư mới lặp**: không clear `mailAlerted` khi bấm tab Letter Box.

Đã xoá nhân vật/mail test `PWTest`, `PWRecv` khỏi DB sau khi test.

## Lệnh Follow (W0131)

Theo `CNewUICommandWindow::CommandFollow` + `ZzzInterface`:

- `CommandFollow` chỉ lưu `g_iFollowCharacter = SelectedChaIndex`.
- Mỗi tick di chuyển, client pathfind tới vị trí người được theo và gửi `SendMove`.
- Người đó chết/biến mất → tự dừng.
- Click/đánh/skill/di chuyển thủ công → huỷ follow.

### Web
- Command Window: `GlobalText[948]` Follow.
- Click Follow rồi click người chơi → bám theo.
- Click trực tiếp vào tên người chơi cũng theo (qua `mu-follow-player`).
- Chat command: `/follow Tên`.
- Mỗi tick:
  - tìm entity theo index
  - nếu chết/mất → dừng
  - nếu cách > 1.5 ô → `findRoute` tới vị trí hiện tại
  - target di chuyển > 1.5 ô so với đích cũ → tính lại đường
- Click đất/quái, phím mũi tên, skill, mở UI → huỷ follow.

Test mới `tests/follow.test.mjs`: gửi move packet đúng hướng, dừng khi target chết.
Tổng **348/348 pass**, build **153 file**.

### Follow — xác nhận lại theo source

- Source dùng `Selectindex` = nhân vật đang được **chọn** (click), rồi `g_iFollowCharacter = Selectindex`.
- Web hỗ trợ 2 luồng:
  1. Đang trỏ vào người chơi → `D` → Follow: dùng `commandTarget()` để theo ngay.
  2. `D` → Follow → **click vào tên hoặc người chơi**: vào chế độ follow rồi bám theo.
- Chỉ **hover** mà không click thì source cũng không bắt đầu follow trừ khi nhân vật đã được chọn; nếu hover không nhận, hãy click vào tên/người.
- Đã test thật bằng Playwright với 2 tài khoản online:
  - click vào tên DKAD sau khi chọn Follow → panel báo `Đang theo DKAD.`
  - có packet di chuyển gửi đi, nhân vật di chuyển theo.
  - target chết/mất → dừng.

### Follow — sửa nhận target khi chọn command (W0133)

Các lỗi đã sửa:

1. **Command name plate** theo source (`newui_Cursorid_wnd.jpg` 128×32):
   - hiện tại con trỏ khi chọn command và trỏ vào người chơi.
2. **Target theo tên/GUI**:
   - trước chỉ nhận khi trỏ đúng thân nhân vật.
   - thêm `pointerTarget` theo hit-box tên + `selectedTarget` bền.
   - `commandTarget()` trả `pointerTarget → hoveredTarget → selectedTarget`.
3. **Click command không nhận target**:
   - `world-controls` khi ở commandMode dùng `pickEntity() ?? commandTarget()`, nên click vùng tên vẫn nhận đúng người.
4. **Click người chơi tự mở shop**:
   - thêm kiểm tra `hasShop(index)`.
   - code != 1 → `clearShopTitle` xoá tiêu đề shop cũ.

Luồng đã test thật:
- `D → Follow → trỏ tên PWAdmin` → plate hiện `PWAdmin`
- click → panel báo `Đang theo PWAdmin.`
- nhân vật bám theo target khi target di chuyển.

### Follow — auto nhận khi chỉ chuột (W0134)

- Source chạy command bằng chuột phải (`MouseRButtonPush`); web hỗ trợ cả trái/phải.
- Bổ sung: khi đã chọn **Follow**, chỉ cần **trỏ chuột vào tên/người** khoảng 0.7s là tự bắt đầu follow (không cần click).
- Vẫn giữ click trái/phải để follow ngay.
- Test E2E 2 tài khoản:
  - D → Follow → hover PWAdmin
  - panel báo `Đang theo PWAdmin.`
  - PWAdmin chạy sang phải, P1 bám theo: X 141 → 164.
