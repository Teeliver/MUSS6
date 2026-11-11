# Menu Esc — native client 0.25

## Đối chiếu

- Menu đăng nhập/chọn nhân vật: `SysMenuWin.cpp`, `WinEx.cpp` trong engine tham chiếu.
  Resource: Interface/op1_stone, op1_back1..4, op1_b_all.
  GlobalText: 381 (Exit), 382 (Server), 385 (Option), 388 (Close).
- Menu trong game: `NewUICustomMessageBox.cpp` / CSystemMenuMsgBox.
  Khung 230x192; 5 nút 108x29, cách nhau 30, bắt đầu y=23.
  Resource: newui_msgbox_top/middle/bottom/back, newui_btn_empty.
  GlobalText: 381, 382, 383, 385, 384.
- Font dùng chung `source_font`, lấy cấu hình/ngôn ngữ của client.
- Không sửa hoặc chuyển tên thư mục Data gốc.

## Hành vi

Esc ưu tiên trả item đang cầm, đóng Kho/Inventory, đóng hộp tạo nhân vật/
hộp thoại/Credit; khi không còn cửa sổ con thì mở menu. Esc lần nữa hoặc
Cancel/Close chỉ đóng menu, phục hồi focus. Menu chặn thao tác chuột xuyên
xuống map nhưng tiếp tục nhận gói tin/heartbeat và render ở mục tiêu 60 FPS.
Nút Menu tại chọn server và chọn nhân vật cũng mở menu này.

Trong game, Exit/Select Server/Switch Character gửi F1:02 tới GameServer:
0/2/1 tương ứng. Chỉ chuyển màn hình sau ACK; Switch Character giữ socket
và tải lại danh sách từ server. GameServer giữ quyền quyết định countdown,
trạng thái cấm logout và lưu nhân vật. Không tự sửa database để đổi nhân vật.

## Phần còn thiếu

- Option được hiển thị nhưng vô hiệu hóa: chưa port đủ cửa sổ tùy chọn,
  âm thanh, hiệu ứng và lưu thiết lập. Không gán một chức năng khác vào nút này.
- Ảnh/định nghĩa engine tham chiếu không đủ để tuyên bố khớp 100% với mọi
  custom patch của EX603. Cần đối chiếu tiếp trên executable Windows tương ứng.
- Kiểm thử AppKit kiểm tra menu 3 màn hình, chữ Data, giới hạn viewport,
  Cancel và callback Select Server; không thay thế kiểm thử logout online.

## Sửa lỗi 0.25.1

`SendRequestLogOut` của engine gốc gọi `spe.Send(TRUE)`. Do đó F1:02 phải
đi qua mã hóa block/serial (header C3 trước khi transport gửi), dù cấu trúc
PMSG_CLOSE_CLIENT_RECV ghi C1 ở phần chú thích. Bản 0.25 dùng C1 không mã
hóa; HackPacketCheck runtime yêu cầu Encrypt=1 và đóng socket ngay.
0.25.1 dùng header.setE, không sửa/nới policy server. Contract kiểm tra C3;
`MuClient-switch-test PRIVATE_CREDENTIALS CHARACTER` kiểm thử thật world ->
character ACK -> danh sách -> world trên cùng socket, rồi server-selection ACK.
Chỉ dùng tài khoản muverify và không tạo/xóa/di chuyển nhân vật.
