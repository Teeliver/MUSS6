# W0089 — Ngồi ghế, dựa tường và con trỏ riêng

## Nguồn đối chiếu

- Main5.2/source/ZzzObject.cpp: CreateOperate/CreateObject; register loại vật thể, BoundingBoxMin/Max.
- GMNewTown.cpp type 103; GMUnitedMarketPlace.cpp type 67; GM_Kanturu_2nd.cpp type 3; GMBattleCastle.cpp type 77/84.
- ZzzInterface.cpp: ActionMovementOperate (3298), SelectOperate (7641), RenderCursor (8673); ZzzOpenData.cpp (5450–5451) ánh xạ CursorLeanAgainst/CursorSitDown.
- ZzzBMD.cpp BMD::Transform: chế độ game thường dùng hộp trục song song thế giới, cộng BodyOrigin, không xoay hộp theo góc model.
- _enum.h: PLAYER_SIT1/PLAYER_SIT_FEMALE1 233/235, POSE1/POSE_FEMALE1 239/240, HEALING1/HEALING_FEMALE1 237/238; ZzzOpenData.cpp tốc độ 0,4 và healing 0,2 mỗi tick 25 Hz.
- GameServer/Protocol.h PMSG_ACTION_RECV và Protocol.cpp CGActionRecv: C1:18 direction/action. Gửi đủ struct 7 byte, hai byte target bằng 0; wsclientinline.h bản bổ sung chỉ phát 5 byte, nhưng server hiện tại đọc đủ struct.

## Đã ghép vào web

429 placement được thống kê trong OPERATE-POINTS-W0089.json. Lorencia 109; Dungeon 39; Devias 129; Noria 69; Atlans 4; Tarkan 13; Castle 38; Kanturu2nd 9; Elbeland 19. Loren Market có nhánh type67 trong code nhưng Data hiện không có placement phù hợp.

Click vùng tương tác: đi tới ô gốc, đổi hướng với các loại có TargetAngle trong source, lấy clip nam/nữ tương ứng, gửi action lên server. Đi tiếp, click chỗ khác hoặc server sửa vị trí sẽ hủy ý định/tư thế. Noria type38 dùng healing, không dùng sit. Atlans và Loren Market dùng pose nhưng con trỏ sit theo đúng nhánh RenderCursor gốc.

Con trỏ lấy trực tiếp CursorLeanAgainst.ozt/CursorSitDown.ozt, kích thước 24 logical pixel, hotspot 2,2 như source. Helper tài nguyên đã khởi động lại để nhận allowlist hai file mới; không đổi truy cập localhost hoặc GameServer.

Viewport đọc ViewState trong CharSet khi spawn, nhận broadcast action 128/129/130 và xóa rest khi nhận di chuyển. Người chơi khác dùng cùng bộ clip nam/nữ. Không phát tiếng đánh cho action ngồi/dựa.

## Kiểm tra và giới hạn

247 client test qua; sau chỉnh điều kiện Castle/thông báo cuối, 8 test tương tác/cursor/remote liên quan đều qua. Build WASM, debug/release qua. Test có đường chọn ghế qua ray/hộp, cursor sit/lean riêng, gửi đúng action một lần, giữ tư thế đến khi đi và khôi phục/xóa rest của người chơi khác. Trình duyệt offline đã quan sát được ngồi trong bar và pose dựa tường Lorencia; chưa xác nhận thao tác bằng hai phiên server thật trong đợt này.

Castle map30 hiện không kích hoạt thao tác vì web chưa nhận đủ trạng thái trận công thành/quyền chủ thành. Source chặn toàn bộ operate lúc trận bắt đầu và chỉ cho guild master ngồi throne77; không tự bỏ qua điều kiện này. Các vị trí thường vẫn phải tìm được đường hợp lệ trên terrain. Chưa kiểm hình ảnh từng placement một và chưa khẳng định tương đương 100% mọi vị trí.
