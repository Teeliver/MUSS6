# Party GUI — W0121 (2026-11-01)

## Nguồn

- `Main5.2/source/NewUIPartyListWindow.cpp/.h`
- `Main5.2/source/NewUISystem.cpp`
- `Main5.2/source/NewUICommonMessageBox.cpp` (`CPartyMsgBoxLayout`)
- `GameServer/Party.cpp`, `Party.h`

## GUI party list theo asset gốc

Asset Data:

- `Interface/newui_Party_Back.OZT` — 77×23
- `Interface/newui_Party_HpBar.OZJ` — 69×3
- `Interface/newui_Party_X.OZT` — 11×22 (2 frame 11×11)
- `Interface/newui_Party_flag.OZT` — 9×10

Theo `CNewUIPartyListWindow::Render`:
- Mỗi thành viên là một hàng 77×23, cách nhau 24px
- Nền: `newui_Party_Back.tga`
- Tên tại `(4,4)` trong hàng
- Leader có cờ tại `(53,3)` và tên màu cam `rgb(255,148,22)`
- Thành viên thường tên trắng
- HP bar tại `(4,16)`, rộng tối đa 69px, cao 3px, tỉ lệ theo 10 nấc HP
- Nút X 11×11 tại `(63,3)` chỉ hiện khi mình là leader hoặc hàng đó là mình
- Vị trí tổng: lệch trái Inventory, `top` khoảng 18px theo source
- Bấm X gửi `C1:43` (`SendRequestPartyLeave`)

## Lời mời party

Theo `CPartyMsgBoxLayout`:
- Hiện tên người mời + `GlobalText[425]` ("Someone requests you to join their a party")
- OK gửi `C1:41 result 1`
- Cancel gửi `C1:41 result 0`
- Hộp mời là modal riêng, không bị `closeSale()` của NPC xoá

## Kết quả party

Hiện đúng text source:
- 0/1/2/3/4/5 tương ứng `GlobalText[497..502]`
- 6 `GlobalText[2990]`
- 7 `GlobalText[2997]`
- 8 `GlobalText[2998]`

## Hiện trạng

- Party list đã dùng asset OZJ/OZT thật, không còn CSS vẽ tay.
- Inventory vẫn nằm bên phải, party list ở bên trái Inventory.
- Chưa port `CNewUIPartyInfoWindow` (cửa sổ party lớn) và kéo/thả đổi thứ tự.
- Đã test 341/341, build debug 151 file.

## Lệnh chat /party

- `/party` hoặc `/pt`: mời nhóm nhân vật đang trỏ chuột.
- `/party Tên` hoặc `/pt Tên`: mời đúng tên.
- Dùng chung `commandTarget` / `world.entities` với `/trade` và `/buy`.

## Sửa disconnect khi đồng ý Party

- `PMSG_PARTY_REQUEST_RESULT_RECV` là `header3 + result1 + index2` = **6 byte**.
- Web cũ gửi `C3:41` header size **5** nhưng thực tế 6 byte → `PacketCipher` kiểm tra size mismatch, đóng kết nối.
- Đã sửa header size thành **6**; test cập nhật `[c3,6,41,result,indexH,indexL]`.

## Chống disconnect khi nhận party list

- `decodePartyPacket` trước chỉ nhận entry 24 byte. Nếu server trả 22 byte (packed) hoặc 36 byte (UPDATE>=802 có ServerCode/Mana) sẽ throw và session đóng.
- Đã sửa: tự nhận entry 22/24/36 byte, đọc HP/MaxHP theo offset tương ứng.
- Bật log packet party 0x40..0x44 hai chiều tại `/tmp/mu-trade-up.log`, `/tmp/mu-trade-down.log` (dùng chung log trade).
- Gateway restart; cần login lại 2 tab.

## Sửa disconnect khi bấm mời Party

- `partyInvitePacket` ghi header size **4** nhưng thực tế gửi **5 byte** (`C3 40 indexH indexL`).
- `PacketCipher.encode` kiểm tra `size == packet.length` → throw outbound-error → WebSocket đóng (gateway log `1000 closed`), không có HACK_LOG vì packet chưa tới server.
- Đã sửa header size thành **5**.
- Party answer giữ size **6**.

## Sửa quyền kick party

- Source `CNewUIPartyListWindow::Render` hiện nút X nếu `Party[0].Name == Hero->ID` (leader) **hoặc** hàng đó là mình.
- Web cũ dùng `i===0 || m.name===selfName` → leader chỉ thấy X ở hàng đầu, member lại thấy X ở hàng leader.
- Đã sửa thành `isLeader = members[0].name === selfName`; leader thấy X mọi hàng, member chỉ thấy X hàng của mình.
- Server vẫn kiểm tra quyền leader trong `CGPartyDelMemberRecv`.
