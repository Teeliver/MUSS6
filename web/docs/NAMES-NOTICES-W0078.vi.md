# W0078 — tên đồ/nhân vật/NPC, phím Option, thông báo

## Phần đã làm

- Alt/Option + Space xử lý ở capture phase trước bộ lọc di chuyển: vẫn hoạt động khi nút UI có focus, event đã defaultPrevented, hoặc macOS trả ký tự NBSP. Không bắt phím khi đang nhập input/textarea/select/contenteditable. Alt/Option giữ tạm tên; tổ hợp Space chốt hiển thị; nhấn lại Alt/Option tắt. Space thường vẫn đi qua điều kiện nhặt đồ cũ.
- Tên nhân vật lấy từ roster thật; tên player khác đọc từ viewport C2:12, record EX603 36 byte + effects. PK đầu vào lấy từ F3:03 offset56. NPC lấy tên resource catalog và chỉ có label khi trỏ vào model được chọn, xóa ngay khi rời. Không dựng tên từ index giả. Màu lấy từ CGMHeadChat::SetPlayerColor: NPC150/255/240, PK1..6 theo bảng source. Vị trí lấy model đầu +40 đơn vị source, chiếu cùng camera terrain; không đặt theo tọa độ màn hình cố định.
- Đồ rơi: ghép tên/cấp cộng +Skill +Option +Luck theo ZzzObject.cpp CreateDropItemName. Text176/177/178 đọc từ Text_eng.bmd; Option là hậu tố có/không, không suy diễn điểm damage/defense. Zen giữ số phân cách hàng nghìn. Dùng chung cho hover và Alt/Option.
- ReceiveNotice type0/2 và10..15 ra vùng giữa màn hình; type1 vào log chat góc trái. Nhặt thành công chỉ ghi log sau ACK, dùng tên Item_eng và Text224/918; thông báo từ chối giữ nguyên server. Có log kết nối khi vào nhân vật.
- Dòng hiển thị khớp chính xác `Powered by SSeMU` đổi thành `Phát triển bởi "T-NLABS - Mu Webzen"`. Chỉ đổi ở client; không đổi copyright header, file Notice.txt hay restart GameServer.
- Label/render resource chỉ dùng RAM như trước, không thêm cache lưu lâu trên browser.

## Kiểm tra

211 test client qua: thêm hồi quy Option+Space khi button focus/defaultPrevented/NBSP; parser player có effect/bad length; bảng màu PK; NPC chỉ hiện khi hover; phân luồng notice/thương hiệu; hậu tố đồ theo bit thực. Build debug/release tách riêng, deploy nguyên tử lên8080.

Trình duyệt fixture offline: Option+Space giữ tên sau thả phím; Alt lần nữa tắt; nút đang focus không chặn. `Kris +9+Option+Luck`, `Small Shield+Option+Luck` hiển thị; event/thương hiệu ở giữa, kết nối/nhặt/từ chối ở log trái. FPS khoảng60/p95~17ms ở cảnh thử, console không có lỗi. Ảnh `/tmp/mu-w78-names-notices.png`. Fixture không mở socket, không đụng đồ thật.

## Còn giới hạn

Chưa kiểm chứng thao tác trên phiên nhân vật thật trong lượt này: cần refresh/đăng nhập lại nhận JS mới. Player khác đã đọc tên nhưng renderer body player khác chưa có; không coi multiplayer đã hoàn chỉnh. Guild/union/emblem, live đổi PK sau khi vào map, tên set Ancient/pet/event đặc biệt và chống đè label đồ chưa port đầy đủ. Notice10..15 hiện trung tâm nhưng chưa có slide theo tốc độ/màu packet; type2 chưa ghi vào cửa sổ guild. Log hiện5 dòng, chưa phải toàn bộ chức năng chat gốc. Không tuyên bố parity100%.
