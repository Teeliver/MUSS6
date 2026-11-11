# W0097 — Áo choàng DL/RF từ Data và PhysicsManager

## Nguyên nhân

Bản cũ vẽ model item dùng trong inventory như attachment thường. Source RenderLinkObject bỏ qua helper30 (Cape of Lord) và wing49 (Cape of Fighter); phần vải được runtime_physical_cloth tạo riêng. Wing40/50 giữ khung trang trí, còn mặt vải và dải bên được thay bằng lưới động. Icon inventory vẫn giữ model nguyên bản.

## Tài nguyên

| Item | Class | Vải chính | Dải bên |
|---|---|---|---|
| Cape of Lord 13,30 | DL | Player/DarklordRobe.OZT | Không |
| Cape of Emperor 12,40 | DL | Item/dl_redwings02.OZT | Item/dl_redwings03.OZT |
| Cape of Fighter 12,49 | RF | Item/NCcape.OZT | Không |
| Cape of Overrule 12,50 | RF | Item/monke_manto.OZT | Item/monk_manto01.OZT |

Tên texture đối chiếu ZzzOpenData.cpp:676–679,1416–1418. Bộ Data cung cấp đặt các texture Item/Texture ở ngay Item nên dùng vị trí thực của file. Không vẽ lại hay thay ảnh.

## Hình học và chuyển động

ZzzCharacter.cpp:15017–15128: bone19, lưới chính10×10, width180, height180(DL)/170(RF), vai thu0.6; cấp3 thêm hai dải2×5, width12, height200(DL)/180(RF). Offset và cầu va chạm bone2/17 lấy riêng theo từng nhánh.

Port PhysicsManager.cpp:344–776: khởi tạo lưới; swizzle(z,-y,x) khi ghim hàng đầu; lò xo, damping.01, mass.0025/inverse400, trọng lực9.8×100 hoặc180, wind giới hạn[-.2,1], lực lệch hai dải RF, trung bình hiệu chỉnh va chạm/đường chéo và giữ chiều dài cạnh dọc. Chạy25tick/s, mỗi tick5 bước0.005 như Move2 của nguồn. Lưới ở tọa độ world để có quán tính khi owner di chuyển; ma trận bone lấy từ pose hiện tại trong WASM.

SetCharacterScale: DL.92, RF1.03. Dùng cùng tỷ lệ cho thân, attachment, tên và vải ở main/remote. Class khác giữ tỷ lệ hiện hành trong lượt này.

Tháo/đeo/đổi áo choàng theo actor swap. Khi teleport/time jump hoặc solver không giữ được chiều dài, khởi tạo lại lưới để tránh kéo vải xuyên bản đồ. Không sửa dữ liệu nhân vật online.

## Kiểm tra và giới hạn

265/265 test client qua sau phần tỷ lệ DL/RF. Test cấu hình đúng class, kích thước/offset, topology, điểm ghim,100tick hữu hạn và teleport reset. Test WASM/Data thật: icon vẫn đủ mesh; world bỏ sheet tĩnh, giữ khung; xuất bone2/17/19 cho cả bốn áo choàng. Đã xem riêng bốn item trong cảnh offline.

Chưa chứng nhận so sánh từng pixel với client native. Wind ngẫu nhiên mỗi actor và tái khởi tạo ổn định có thể khác trạng thái tức thời của native. Chưa port các lớp tóc/vạt giáp phụ của toàn bộ class hoặc trạng thái stun/sleep của cloth manager.

### QA trình duyệt cuối — 2026-10-30

Đã kiểm tra riêng Cape of Lord/DL, Cape of Emperor/DL, Cape of Fighter/RF, Cape of Overrule/RF ở bản đã publish: main và remote cùng hiện đúng texture, nhìn từ phía sau, vải thay đổi khi di chuyển bằng phím. Cảnh offline đo khoảng 60 FPS, frame p95 17.6–17.7 ms trên máy hiện tại; không phải benchmark mọi thiết bị hoặc xác nhận phiên online.

Ảnh kiểm chứng: `/tmp/mu-w97-dl2-peer.png`, `/tmp/mu-w97-dl3-peer.png`, `/tmp/mu-w97-dl3-walk.png`, `/tmp/mu-w97-rf2-peer.png`, `/tmp/mu-w97-rf2-walk.png`, `/tmp/mu-w97-rf3-peer.png`, `/tmp/mu-w97-rf3-walk.png`. Tab test giữ lựa chọn đúng class và có remote cape cùng loại. Cảnh Lorencia còn cảnh báo Object168 chưa có mapping, thuộc audit map riêng.
