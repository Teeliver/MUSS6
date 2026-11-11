# W0072 — HUD căn giữa, FPS/RTT và nút đáy inventory

Ngày 2026-10-26. Người dùng xác nhận giao diện ổn sau kiểm tra.

## HUD và bộ đo

- `gameUiLayout` giữ tỷ lệ 640:51, rộng tối đa 960 CSS px và không vượt 90% viewport; căn giữa ở đáy. Không dùng width:100% kéo giãn canvas ngang. Nút HUD dùng chung khung nên hotspot khớp hình. Scale panel C/I được giữ riêng.
- Test kích thước: 708×899 → HUD637.2×50.78, lề35.4; 944×899 →849.6×67.70, lề47.2; 1280×720 →960×76.5, lề160; 1920×1080 →960×76.5, lề480; 3440×1440 →960×76.5, lề1240.
- Bảng nhỏ FPS/frame p95, RTT và buffer/DPR luôn hiện trong game. F9 bật phần debug mở rộng. FPS lấy từ vòng render thật; p95 dùng ring 240 mẫu frame interval, bỏ delta≥1s để tránh tính thời gian tab bị treo/ẩn; không phải GPU timer hoặc chứng nhận FPS ổn định. Buffer cập nhật theo canvas thực khi viewport đổi.
- RTT là thời gian request→ACK GameServer của entry/move/item/stat/repair/shop, gồm cả xử lý server. Giá trị đứng yên là mẫu ACK gần nhất, không phải ICMP ping liên tục. Không gửi packet ping giả hoặc dùng RTT helper thay GameServer.

## Khiên và nút gốc

- Small Shield mặc định quay mặt trước: hướng inventory 270/-90/0. Shield01 có mặt +X gồm12 tam giác, UV area~.7606; mặt −X6 tam giác UV area~.0154. Đưa +X về +Z cho thấy đúng phần tâm kim loại của texture du_shield1.OZJ32×32, không phải mặt sau nâu. Đây là fallback cho Data hiện tại khi chưa có ItemTRSData.
- Ba nút inventory tại x13/49/85,y391: exit/repair/personal store, dùng hai frame thật của newui_exit_00/newui_repair_00/newui_Bt_openshop. Resource và code trạng thái đã đối chiếu NewUIMyInventory.cpp/NewUIButton.cpp.
- Tự sửa từ cấp50, cửa hàng cá nhân từ cấp6, khớp source client và GameServer. Repair mode gửi C1:34 slot/type1, chỉ cập nhật độ bền/Zen qua packet server; không sửa đồ tự động trong test.

## Cửa hàng cá nhân cơ bản

- Nút mở màn hình chuẩn bị hàng 8×4, slot204..235. Chuyển item giữa túi/trang bị và shop dùng container flag0/4, giữ wire12byte, chỉ commit sau ACK. Không tự swap món đang chiếm ô.
- Đặt giá Zen qua C1:3F:01 (9byte, giá little-endian), mở qua 3F:02 (40byte, tên tối đa35 ký tự ASCII theo phạm vi đang hỗ trợ), đóng qua 3F:03. ACK giá/mở/đóng lần lượt6/5/7byte; đóng kiểm đúng index của mình, bỏ qua broadcast người khác.
- Cần có hàng và giá đã được server xác nhận mới mở bán. Khi đang bán không chỉnh/chuyển đồ; đóng giao diện không tự ngừng bán và có thông báo rõ. Timeout giữ khóa để tránh retry mù. Thông báo bán hàng3F:08 loại item đã bán khỏi snapshot.
- UI chuẩn bị là bản cơ bản với frame hiện có, chưa port mọi skin/tab của MyShop hoặc giao diện mua shop người khác, giá Jewel/tiền custom UPDATE802, tên đa mã hóa và custom store. Không tuyên bố full marketplace hoặc parity100%.
- ABI generator nay lấy19 struct từ Protocol.h/ItemManager.h/PersonalShop.h, tiền xử lý #if UPDATE603/EXTRA1 trước khi lấy offset; không lẫn trường giá Jewel của UPDATE802.

## Kiểm tra và bằng chứng

- 170/170 client test qua cho bản W0072; sau tinh chỉnh lề90%, RTT entry và input/Esc, 22/22 test state/item liên quan qua lại, cùng kiểm tra layout nhiều viewport. Debug82/release31 file, stage179asset; Data gốc không chỉnh.
- Backup đóng gói cuối: W0072-20261026-225153. Debug entry revision cuối d08a0b8aa61272fa. Phiên người dùng đã xác nhận đang mở revision b8b2b656257ac092 của W0072; tinh chỉnh cuối nhận ở lần refresh tiếp theo.
- Browser ngoại tuyến: viewport1280×720 đo HUD960×76.5 tại x160; nhìn rõ mặt trước Small Shield và ba nút footer, repair khóa ở level6; nút shop mở màn hình chuẩn bị đúng, không đăng bán thật.
- Ảnh: /tmp/mu-w72-hud-centered-wide.png; /tmp/mu-w72-store-editor-offline.png. Số FPS trong ảnh ngoại tuyến chịu ảnh hưởng automation, không dùng làm benchmark game online.
- Đọc trực tiếp DOM phiên người dùng sau xác nhận: FPS60.0, frame p9516.9ms, RTT GameServer11.3ms ACKmove, buffer1888×1798/DPR2. Đây là mẫu tại thời điểm đọc, không khẳng định luôn60FPS.
- Không cộng điểm, sửa đồ/tốn Zen, đặt giá hoặc mở bán món thật bằng thao tác agent trong W0072. Test packet shop/repair dùng ACK giả trong unit test, không chứng minh một giao dịch bán hoàn tất giữa hai client thật.
