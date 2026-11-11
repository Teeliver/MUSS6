# W0090 — Quái, animation và thanh máu

Cập nhật 2026-10-29. Đang kiểm tra tích hợp, chưa đạt parity toàn bộ quái/hiệu ứng.

## Phần đã triển khai

- Đọc binding CreateCharacter và OpenMonsterModel trong Main5.2/source, giữ model, scale, tốc độ/action theo nguồn. Catalog bao gồm Skeleton dạng Player, Chaos Castle biến thể và các bẫy có binding rõ ràng.
- Kiểm kê 61 file spawn của server: 6.739 dòng cấu hình, tổng count cấu hình 7.726. Đây không phải số quái sống đồng thời; event/section5 có controller riêng. Không tạo spawn giả trên client.
- Có binding cho 311/313 loại trong cấu hình. Báo cáo chi tiết: MONSTER-SPAWN-W0090.json và MONSTER-COVERAGE-W0090.json. Hai loại chưa đủ dữ liệu: 106 Laser Trap và 523 Imperial Guardian Trap; người dùng xác nhận chưa có config/source bổ sung.
- Client giữ clip idle/walk/attack/death hiện hữu, chạy death một lần rồi fade theo logic nguồn. Skeleton ẩn thân khi chết theo nguồn; hiệu ứng mảnh xương còn thiếu.
- Nhận thanh máu C2:F3:E2 theo record 4 byte của server hiện tại; hỗ trợ record 12 byte của client nguồn khác. Chỉ hiện phần trăm khi server không gửi HP/maxHP, không tự suy ra số máu.
- Hover quái hiện tên/thanh máu, chọn vùng đánh theo bounding box nguồn và dùng CursorAttack gốc. Sửa clock âm nhỏ giữa event và animation frame làm mất actor.
- Texture dùng lại theo basename chỉ khi các bản tìm được có cùng SHA256. Actor Maya 364 được nguồn chủ động ẩn, không tính là thiếu model cần thay thế.

## Kiểm tra và giới hạn

250 test client đã qua trong đợt triển khai; test mục tiêu monster/navigation/cursor cũng đã qua sau sửa chọn vùng đánh và clock. WASM đã build và kiểm tra. Fixture offline xác nhận tên, thanh máu và death; số HP 3K/4K trong fixture là dữ liệu test.

Trước reboot đã quan sát Cursed Lich và phần trăm máu trên phiên Elbeland thật. Chưa chốt vòng đánh → chết trên phiên thật. Vũ khí gắn riêng của một số quái, particle/special death và toàn bộ hiệu ứng map vẫn còn phần chưa port; không coi catalog đầy đủ là render đạt 100%.

## Phục hồi sau reboot

Ngày 2026-10-29 đã mở lại Docker Desktop, MySQL và Connect/Join/Data/GameServer, gateway9000, helper9001, web8080. Kiểm tra listener đầy đủ; GameServer báo Join=1/Data=1, Join/Data kết nối MySQL. Giữ nguyên bản web đã stage và dữ liệu nhân vật. Đang chờ đăng nhập lại để tiếp tục kiểm tra combat thật.

## Evil Spirit: quái Elbeland (2026-10-29)

world-session.cast từng lọc multi-skill bằng type < 200 nên bỏ sót Cursed Lich422. Đã thay bằng attackableMonsterTypes sinh từ actor-catalog nguồn; bộ sinh catalog đồng bộ danh sách. Giữ loại NPC/player/quái chết và ngoài phạm vi, không đổi damage hay quyền đánh trên server.

Test hồi quy dùng quái422/423, NPC240, quái chết và quái xa để xác nhận index C3:DB. Full suite251/251 qua trong thư mục test tách biệt; build Debug125/Release32, revision59bf1fdb9e86462f đã lên server.

Phiên thật tải revision mới: Evil Spirit trừ mana, EXP tăng trong lượt chơi Elbeland, quái ngã/chết và có đồ rơi, không disconnect. Người dùng đồng thời thao tác/cộng điểm/nhặt đồ nên chưa coi đây là phép đo damage đơn mục tiêu độc lập. Bằng chứng /tmp/mu-w90-evil-spirit-fixed.png. FPS quan sát54–60, chưa benchmark toàn map.
