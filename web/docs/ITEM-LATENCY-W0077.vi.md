# W0077 — Kiểm tra và sửa độ trễ thay đồ / quăng rồi nhặt lại

## Nguyên nhân đã xác định

1. W0076 đã giữ map nhưng mỗi lần đổi trang bị vẫn decode toàn bộ body/animation/texture và tạo renderer nhân vật mới. Khi thả lại cùng item, ground-layer cũng dựng renderer và upload texture lại.
2. Nhặt thành công vẫn giữ hình đồ trên đất tới gói viewport C2:21, dù ACK22 đã xác nhận. Đây là chờ thừa ở client.
3. Native GameServer chạy QUEUE_TIMER_VIEWPORT mỗi1000ms (`native/game/main.cpp::timers`); đồ vừa thả chỉ được phát ra ở nhịp viewport kế tiếp. Vì vậy riêng đoạn ACK23→C2:20 có thể gần1giây.

## Thay đổi

- Prepared-store chỉ trong RAM của phiên/map: giữ tối đa4 cấu hình actor và24 model đồ rơi không còn dùng. Resource đang có lease không bị đẩy ra. Dùng chung yêu cầu đang tải, giải phóng khi đổi map/đóng view. Không có browser cache hoặc dữ liệu ghi bền; sửa resource rồi refresh vẫn nhận bản mới.
- Khi mặc lại bộ đã chuẩn bị hoặc thả lại model đã có, tái dùng cả GPU/rig thay vì dựng lại. Actor cache sử dụng timeline hiện hành của scene; đã bắt và sửa lỗi native rig không chấp nhận quay timeline về0.
- Nhặt ACK thành công xóa đúng key map ngay, thất bại giữ nguyên. Gói C2:21 sau đó vẫn xử lý idempotent. Packet chia Zen party không được coi là ACK cho yêu cầu đang nhặt đồ thường.
- Native-only `CViewport::RefreshViewportItems`: sau CGItemGetRecv/CGItemDropRecv gửi update item cho người thao tác ngay. Chỉ advance VpPlayerItem, giữ nguyên state player/NPC và nhịp viewport1000ms chung. Không sửa quyền nhặt, thời gian loot, item options hay protocol gốc. Nếu thao tác thất bại thì không tự tạo/xóa item.
- GameServer đã build và nạp lại bằng SIGTERM qua shutdown_world, đợi lưu/gửi dữ liệu xong; listener55900 hoạt động lại, Join=1 Data=1. Không thay tài khoản/config gameplay.

## Kiểm chứng

- 206 test client/contract qua, gồm source-extracted C++ test item viewport: destroy→dọn count→create→send, NPC state không bị đổi; resource store không evict actor đang dùng, coalesce và giải phóng build muộn.
- 8 native GameServer contract qua. Lần CTest đầu crypto trỏ thư mục MuServer cũ; chạy lại bằng đường dẫn MuServerEX603 và sửa hai CMake test-directory cache values, cả8 test xanh. Không thay key hay cấu hình runtime.
- Fixture trên8080, sáu lần tháo/mặc Kris: lần chuẩn bị mới138.5ms; các lần tái dùng0.5/0.5/0.6/0.9/0.7ms. Scene generation luôn1, actor revision tới6, không console error. Đây là thời gian xử lý model trong client, không bao gồm RTT server.
- Ba model rơi: lần đầu khoảng98.3ms; thả lại cùng ba model hit3/miss vẫn3, thời gian nhận prepared model0.1ms. Không tạo lại GPU renderer cho lần lặp đó.
- Ảnh `/tmp/mu-w77-equipment-delay-fixed.png`. Chưa agent thả/tiêu hao đồ thật; đo luồng renderer bằng fixture và tính đúng của giao dịch bằng packet test. Cần người dùng kiểm chuỗi thật sau refresh/đăng nhập lại vì GameServer đã được nạp mới.

## Giới hạn

Model/bộ trang bị chưa từng chuẩn bị vẫn cần tải/decode/chuẩn bị một lần; số liệu đầu khoảng100–140ms trong fixture, chưa phải0ms. RTT và thời gian xử lý server vẫn tồn tại. Những người chơi khác nhận thay đổi theo viewport thông thường; fast-path hiện gửi ngay cho người thao tác. Model còn thiếu theo audit resource trước đó không được thay bằng hình giả.

Mốc này thay thế ghi chú W0076 về việc chờ despawn sau pickup ACK và việc luôn giải phóng renderer actor sau mỗi lần đổi đồ.
