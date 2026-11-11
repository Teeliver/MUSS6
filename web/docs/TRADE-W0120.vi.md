# Trade — W0120 (2026-11-01)

## Nguồn

- Client GUI: `Main5.2/source/NewUITrade.cpp/.h`, `NewUICommonMessageBox.cpp` (`CTradeMsgBoxLayout`).
- Packet: `Main5.2/source/wsclientinline.h`, `WSclient.h` (`PTRADE`, `PHEADER_TRADE_MONEY`).
- Server: `GameServer/Trade.cpp/.h`, `ItemManager.cpp` (`CGItemMoveRecv`), `Protocol.cpp`.

## Packet đã port

| Packet | Chiều | Layout |
|---|---|---|
| `C3:36` | client→server | index 2 byte |
| `C3:36` | server→client | name 10 byte (lời mời) |
| `C1:37` | client→server | response 1 byte (0 từ chối, 1 đồng ý) |
| `C1:37` | server→client | response + name10 + level2 + guild4 (20 byte) |
| `C1:38` | server→client | slot item đối phương xoá |
| `C1:39` | server→client | slot + 12 byte ItemInfo |
| `C1:3A` | client→server | Zen giao dịch (DWORD) |
| `C1:3A` | server→client | result 1 byte (xác nhận đặt Zen) |
| `C1:3B` | server→client | Zen đối phương đặt |
| `C3:3C` | client→server | cờ xác nhận 1 byte |
| `C1:3C` | server→client | cờ xác nhận đối phương; `2` reset cả hai |
| `C3:3D` | client→server | huỷ/thoát giao dịch |
| `C1:3D` | server→client | kết quả: 0 huỷ, 1 thành công, 2 lỗi, 3/4/5/6 lỗi đặc biệt |
| `C3:24` | hai chiều | item move, `SourceFlag/TargetFlag = 1` cho trade |

- Item vào trade: `SourceFlag=0`, `TargetFlag=1`, `TargetSlot=0..31`.
- Item trả về túi: `SourceFlag=1`, `TargetFlag=0`, `TargetSlot` túi.
- Zen gửi bằng `C1:3A` 7 byte (DWORD little-endian).

## GUI theo source

`NewUITrade::Create`:
- Panel 190×429
- Grid đối phương 8×4 tại `(15,68)`
- Grid của mình 8×4 tại `(15,274)`
- Nền tên đối phương `newui_Account_title.tga` tại `(11,37)` 170×28
- Hàng Zen trên `(11,150)` và dưới `(11,356)` dùng `newui_item_money.tga` 170×26
- Nút xác nhận đối phương `(146,186)` 36×29
- Nút xác nhận của mình `(144,390)` 36×29
- Nút đóng `(13,390)` 36×29
- Nút nhập Zen `(104,390)` 36×29
- Asset: `newui_Bt_accept.tga`, `newui_Account_title.tga`, `newui_Bt_money01.tga`, `newui_exit_00.tga`, `newui_item_box.tga`, `newui_item_money.tga`

Web:
- Trade panel nằm bên phải, kế bên Inventory (`right:190px * --mu-scale`) đúng `WinOpen2`.
- Mở trade tự mở Inventory.
- Kéo/nhấc item trong túi rồi click ô trade để đưa vào.
- Click item trong trade để trả về túi.
- Nút Zen mở hộp nhập số, gửi `C1:3A`.
- Nút xác nhận gửi `C3:3C`, đổi icon theo trạng thái.
- Đóng/thoát gửi `C3:3D`.
- Lời mời trade dùng hộp `name + GlobalText[419]`, OK gửi `C1:37` result 1, Cancel result 0.

## Kiểm tra

- `341/341` test pass.
- Test mới: packet trade, decoder response/item/money/confirm/result, session mở trade và cập nhật item/zen/confirm/close.
- Build debug 151 file.

## Giới hạn

- Chưa port danh sách item auto-move, cảnh báo item hiếm, backup item/coin đầy đủ như `NewUITrade`.
- Trade coin (JoB/JoS/JoC) chưa có vì server Emulator dùng `C1:3A` DWORD Zen đơn giản.
- Chưa port các msgbox cảnh báo nâng cao (GlobalText[2108] đã hiện khi server trả result 4).

## Sửa lỗi timeout lời mời trade

- `PacketCipher.decode` chuẩn hoá packet mã hoá `C3` thành `C1` sau khi giải mã. Vì vậy lời mời trade server gửi `C3:36` đến web dưới dạng `C1:36`.
- Decoder `decodeTradeRequest` ban đầu chỉ nhận `C3` nên người nhận không thấy hộp thoại, dẫn tới server timeout sau 5 giây và gửi result `3`.
- Đã sửa decoder nhận cả `C1` và `C3`.
- Đã sửa mapping result:
  - `2` = inventory full `GlobalText[495]`
  - `3` = trade request canceled `GlobalText[496]`
  - `4` = reinforced item `GlobalText[2108]`
- Thêm test decode trade request dạng `C1` sau giải mã.

## Sửa lỗi hộp thoại mời trade bị đóng ngay

- `renderNpc()` gọi `closeSale()` mỗi lần render khi không mở NPC. Điều này xoá luôn hộp thoại mời trade/party vừa tạo, nên người nhận không thấy gì và server timeout sau 5 giây.
- Đã sửa: chỉ gọi `closeSale()` khi NPC panel **đang mở và vừa đóng**, không xoá dialog trade/party đang chờ.
- Áp dụng chung cho hộp mời Party.

## Sửa lỗi không bỏ được đồ vào ô trade

- `pickItem` chỉ nhấc đồ nhưng không gọi `render()` lại, nên các ô drop của Trade (chỉ tạo khi `dragged !== undefined`) chưa xuất hiện → click không có ô để bỏ vào.
- Đã sửa: khi đang mở Trade và nhấc item, set `tradeSignature=undefined; render()` để tạo ngay 32 ô nhận đồ.
- Nút Zen gọi `tradeMoney(...).catch(...)` nhưng session `tradeMoney` không trả Promise → lỗi `Cannot read properties of undefined (reading 'catch')`.
- Đã sửa `tradeMoney` trả Promise, resolve khi server trả `C1:3A` result 1, reject khi bị từ chối/timeout.

## Sửa Zen giao dịch bị lệch số

- `PMSG_TRADE_MONEY_RECV` / `PMSG_TRADE_MONEY_SEND` có `PBMSG_HEAD` 3 byte, DWORD `money` bị canh 4 byte → offset **4**, sizeof **8**. Không phải offset 3/7.
- Web trước đó gửi `C1:3A` dài 7, tiền ở offset 3; server đọc offset 4 nên nhận sai số (vài chục nghìn).
- Đã sửa:
  - gửi `C1:3A` dài **8**, byte đệm ở offset 3, DWORD tại offset 4
  - decode `C1:3B` đọc DWORD tại offset **4**
- Test packet cập nhật: `[c1,8,3a,0, 40,e2,01,00]` cho 123456.

## Bổ sung text_eng.bmd trong khoảng trống Trade

Theo `NewUITrade::RenderText` cuối hàm, phần trống giữa hai grid hiển thị:
- `GlobalText[370]` "Warning!" màu đỏ tại `(20,185)`
- `GlobalText[365]` "Notice! Please check out" tại `(45,185)`
- `GlobalText[366]` "the level of the player" tại `(20,200)`
- `GlobalText[367]` "and the items before trading." tại `(20,215)`
- Màu cảnh báo `rgb(255,220,150)` như source.

## Sửa lỗi Escape/đóng Trade `reading 'catch'`

- `tradeExit()` trong session trước đó trả `undefined`, nhưng UI gọi `.catch(...)` khi bấm Esc/nút đóng → `TypeError: Cannot read properties of undefined (reading 'catch')`.
- Đã sửa `tradeExit()` trả Promise: resolve sau khi gửi `C3:3D`, reject nếu chưa mở trade hoặc gửi lỗi.
