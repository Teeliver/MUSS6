# W0080 — dựng người chơi khác trong viewport

Nguyên nhân lỗi hai account chỉ thấy tên: W0078 nhận C2:12 name/PK nhưng bỏ CharSet18 byte; npc-layer chỉ dựng actor có NPC catalog binding, nên player chưa có model.

## Đã sửa

- Giữ và clone CharSet cùng snapshot C2:12. Parse đúng record36+effectCount của GameServer/Viewport.h EX603.
- Player-layer riêng dựng BodyAnimation + model/armor/vũ khí từ cùng body pipeline của nhân vật chính. Trang bị lấy từ CharSet của chính player đó, không dùng inventory của account đang xem. Renderer có shadow pass và upgrade pass như body hiện có.
- Giữ instance theo server index, di chuyển nội suy theo snapshot D4/15; sample idle/walk/run/safezone, action attack; xóa khi destroy/dead/đổi scene. Tên bám vào pose model thay vì endpoint giật từng packet.
- F3:13 cập nhật đủ CharSet; C1:25 cập nhật/tháo từng ô. Byte ItemInfo[1] ở đây là slot ở high nibble và visualLevel ở low nibble, không phải cách decode inventory. LevelConvert lấy0/3/5/7/9/11/13/15 từ ZzzCharacter.cpp. Index8191 là tháo; override không ảnh hưởng các ô khác.
- Thay appearance giữ body hiện tại đến khi bản mới tải xong; bỏ kết quả async đã stale hoặc player đã rời. Giải phóng rig, renderer, ảnh khi hủy, không tạo socket hay ghi DB trong fixture.

## Kiểm tra

Full test client216 qua. Test player-layer: spawn/model, draw shadows+body, đổi CharSet giữ body cũ đến commit, kết quả async stale không hồi sinh player đã rời; packet equipment/tách visualLevel; tháo ô không làm mất armor khác; snapshot không bị sửa ngoài ý muốn.

Browser offline với hai player bên cạnh self đã thấy đủ3 model (không còn chỉ tên), chuyển vị trí vẫn giữ body; Các lần đo fixture ghi nhận khoảng34–60FPS khi có nhiều phiên browser; không lỗi console. Remote rig sample tối đa25Hz như nhịp animation gốc, camera/render vẫn theo RAF; không cam kết60FPS cho mọi số player. Ảnh `/tmp/mu-w80-remote-players.png`. Fixture `?scene=terrain-check&map=0&x=140&y=125&walk=1&players=1`, nút thay vị trí/trang bị player giả; không giao dịch tài khoản thật.

## Giới hạn

Bản deploy cần refresh cả hai phiên game và đăng nhập lại. Chưa quan sát lại trực tiếp hai phiên thật của người dùng sau khi refresh trong lượt này. Wing/pet/cloth/effect đặc thù, biến hình, buff trạng thái và chi tiết PvP không được coi đã đạt parity. Budget hiện32 remote bodies. Nội suy tốc độ hiện dùng safezone3/ngoài5 tile/s; chưa có packet tốc độ riêng của từng player. Tên model đã render mới xuất hiện; khi tải resource lần đầu có thể chờ model, không bịa model thay thế.
