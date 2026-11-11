# Client EX603 native macOS 0.14

## Luồng online đã triển khai

Chạy MySQL → Connect → Join → Data → Game rồi mở `native/Start-Client.command`.

1. Lấy danh sách server, chọn Server 0.
2. Đăng nhập bằng account test muverify (password ở dòng 2 của native/runtime/client/credentials.txt).
3. Chọn MacGuiDW hoặc nhân vật DW ở Lorencia, bấm **Xem nhân vật trong map**.
4. Cửa sổ cảnh giữ phiên network. **Click terrain** tìm đường A*, đóng gói tối đa 14 bước trong C1:D4; phím mũi tên dùng một bước. Chỉ dựng đường đã được server xác nhận, xác nhận điểm cuối không gây teleport tới điểm cuối. Ô có attribute 4/8 bị chặn ở phía client; server tiếp tục kiểm tra độc lập.
5. Camera khóa góc nhìn MU, FOV 35°, theo nhân vật; không orbit/zoom. Đóng cửa sổ map hoặc bấm Ngắt kết nối để logout; vị trí được server lưu.

F3:09 được gửi ngay sau greeting theo handler Main_EX603 gốc. Identifier là SHA-256 rút gọn của host UUID macOS, đóng gói 5 nhóm hex vào trường 45 byte. Không giả Windows hardware ID, không tắt ClientVerify trên server, không lưu UUID raw hoặc password trong profile.

TCP được nhận theo buffer không chặn, giữ các frame bị chia nhỏ, giới hạn kích thước/buffer và deadline packet chưa đủ. Queue network tuần tự, pump tối đa 60 Hz, coalesce tick để không xếp hàng vô hạn. Timer chạy trong NSRunLoopCommonModes để vẫn xử lý khi kéo camera hoặc mở menu. Heartbeat 5 giây dùng tốc độ nhận từ packet F3:E1 của server.

## Tối ưu và animation

- Terrain/model/pose dùng cache immutable trong tiến trình; mỗi cảnh có focus riêng. Bỏ atlas 64 MiB: terrain dùng texture gốc trên GPU với UV liên tục (64 texel/ô), light/alpha từng đỉnh và repeat/mipmap.
- Texture da dùng chung giữa các phần model.
- Idle dùng action 1 (6 keyframe), đi bộ dùng action 15 (7 keyframe) trong Player.bmd, nội suy quaternion và GPU skinning (mỗi vertex gắn một xương như BMD). Một skeleton dùng chung cho năm phần trang phục; không trộn mesh, đọc BMD hay dựng lại geometry mỗi frame. Quaternion được chuẩn hóa và nội suy trực tiếp ở renderer callback, không dùng CAAnimation để blend khớp. Mỗi frame chỉ cập nhật joint transform và tính điểm thấp nhất của boot để căn chân trên mặt đất.
- Bản sửa ngày 14/10/2026 thay morpher additive gây phồng/co nhân vật. Chu kỳ idle đặt 1,2 giây. Trong 0.11, walk/run lấy (số keyframe−1)/(0,33 hoặc 0,34 × 25) theo SetPlayerWalk tham chiếu; chưa đối chiếu với main.exe EX603 gốc. Bản 0.5 sửa action đứng từ action 0 (PLAYER_SET) sang action 1 (PLAYER_STOP_MALE).
- Renderer mục tiêu 60 FPS, MSAA 2x, dừng animation khi cửa sổ bị che. Terrain không được dựng lại khi di chuyển; chỉ actor/camera cập nhật.
- Asset reader và bộ tạo batch geometry build với -O2.
- Đo một lần trên Mac hiện tại: tải asset lần đầu trong tiến trình khoảng 144 ms, mở lại cache khoảng 44 micro giây. Đây là đo cache ứng dụng, không phải benchmark disk lạnh hoặc cam kết mọi máy.

## Chuyển động, môi trường và bóng hiện tại

- Queue tối đa 16 vị trí đã được server xác nhận, bỏ chờ 80 ms khi bắt đầu. Gửi cả đường thay vì chờ ACK từng ô. Nội suy Catmull-Rom qua bốn điểm từ MovePath tham chiếu; 0.14 lấy quãng đường thực theo bảng arc-length 64 đoạn; speed walk 3 ô/giây, run 3,75 ô/giây (CharacterMoveSpeed 12/15 ×25 Hz, tile 100). Không gán cùng thời gian cho bước chéo/cardinal. Camera theo vị trí render. Click mới tìm đường từ vị trí render hiện tại và thay đường cũ sau ACK; server correction hủy đường và đồng bộ vị trí.
- Hướng mặt theo tiếp tuyến đường cong, cung ngắn nhất qua 360 độ, hệ số giảm nửa ở 25 Hz chuyển sang thời gian thực. Đây là chuyển thể render liên tục; MovePath gốc snap góc ≥45 độ và giảm nửa góc nhỏ, chưa phải parity từng tick.
- Run cho DW không vũ khí: đọc slot giày 6 từ C4:F3:10, level từ ItemInfo; giữ bộ đếm 40 tick ở nhịp mô phỏng 25 Hz khi giày +5 và ngoài safe zone, reset khi dừng hoặc vào safe zone. Đây là phần được đối chiếu với [SetPlayerWalk/SetPlayerStop](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzCharacter.cpp); absolute timing chưa đối chiếu executable EX603. Hiện chỉ inventory snapshot lúc vào game được xử lý, chưa có UI đổi trang bị hoặc mount/wing/effect movement.
- Bóng dùng mesh Shadow01.bmd gốc, FK của cùng pose hiện tại, chiếu lên độ cao terrain theo hướng sáng cố định. Ba Metal vertex buffer và geometry được cấp phát một lần; không tạo geometry mỗi frame. Bóng tròn thử nghiệm đã bỏ. Hướng sáng và opacity chưa đối chiếu frame với Windows.
- GUI 0.9 đã xác nhận bóng hình người và click di chuyển ở 60 FPS. 0.10 nạp 2.864 vật thể gốc từ EncTerrain1.obj, gộp theo texture/chunk và cull ở xa. 0.11 tách HouseWall05/06 (type 125/126) khỏi vật liệu khác; HeroTile lấy layer1, khi bằng 4 giảm alpha 0,05 mỗi tick 25 Hz, ra ngoài tăng lại. Texture vật thể dùng repeat thay clamp. Một type 168 chưa có mapping; chưa hoàn thiện các hiệu ứng/material đặc biệt.

## Kiểm chứng

Gói protocol được kiểm chứng trong 0.11 ngày 14/10/2026: bốn test offline pass; live test gói ba bước và heartbeat 75 giây pass, 144,131 → 143,130, nhận 131 packet.

Bốn test offline: protocol, asset thật, geometry CGFloat và cache/pose sharing. Bản sửa skeleton thêm kiểm tra 21 góc nội suy quaternion không scale/shear, cùng khoảng cách giữa các vertex thuộc cùng xương trong tất cả keyframe idle/walk; các kiểm tra này đã pass. GUI preview đã kiểm tra trực tiếp sau đổi sang GPU skinning. Live test kiểm tra đăng nhập, nhận chỉ số server, bước đi được xác nhận, heartbeat và lưu tọa độ sau logout. Chi tiết kết quả live/GUI được cập nhật sau mỗi lần test; không coi cổng mở là chứng minh gameplay.

```bash
cmake --build native/build --target MuClient-native MuClient-viewer MuClient-assets mu_client_contract mu_client_render_contract MuClient-world-test -j4
ctest --test-dir native/build -R '^mu_client_(assets|contract|render_contract|cache_contract)$' --output-on-failure
native/build/client/MuClient-world-test native/runtime/client/credentials.txt
```

Live test chỉ dùng muverify, có nhân vật MacGuiDW, gửi đường ba bước (đi–quay lại–đi) và giữ phiên 75 giây. Không chạy cùng lúc bạn đang dùng account muverify trong GUI.

Kết quả ngày 14/10/2026: cả bốn test offline và live authentication test pass. World test giữ phiên 75 giây, nhận 124 packet và xác nhận bước đi 135,126 → 134,125. GUI bản mới đã đăng nhập, vào MacGuiDW, dùng Right/Down đi 134,125 → 135,126, hiển thị model/terrain và đóng map để logout. Truy vấn MySQL sau GUI xác nhận ConnectStat=0, MapNumber=0, MapPosX=135, MapPosY=126. Ảnh kiểm chứng nằm tại runtime/client/lorencia-online-04.png.

Kiểm thử GUI bản sửa di chuyển: giữ Right 900 ms, server xác nhận X=137 → 140, Y=129; renderer hiển thị nhân vật không còn pose vặn tay/chân như ảnh báo lỗi. Vẫn cần đối chiếu cảm giác/tốc độ với executable gốc; chưa tuyên bố giống 100%.

## Phần chưa triển khai

Hiện chỉ Lorencia và model DW mặc định, có nhà/cây/static objects gốc, tìm đường bằng chuột, idle/walk/run và bóng. Chuyển clip dùng quaternion slerp trong 0,12 giây. Run tự động nếu inventory thật có giày DW +5 và ngoài safe zone. Chưa có renderer viewport NPC/monster/người chơi khác, hội thoại NPC, UI inventory/equipment và mesh trang bị thật, combat, skill, sound hoặc map/class khác. Packet không được renderer dùng vẫn được nhận và kiểm tra framing, không tạo entity giả.

Bản này chưa phải client chính thức hoặc parity 100%. Bộ Main_EX603 là DLL hook; phần engine macOS đang được viết riêng. Cấu hình/source format: [ASSET-FORMATS.vi.md](ASSET-FORMATS.vi.md).

## Lịch sử bản 0.3 (không mô tả hành vi hiện tại)

Mở `native/Start-Client.command`, bấm **Xem Lorencia (offline)** để xem terrain và nhân vật DW gốc. Không cần chạy server cho preview offline. Có thể mở riêng bằng `native/Start-WorldViewer.command`.

Lịch sử: kéo chuột/scroll để orbit và zoom; **Về nhân vật** đưa camera về vị trí actor. Năm phần model Helm/Armor/Pant/Glove/BootClass01 được đọc trực tiếp từ BMD, liên kết với các bone được dùng trong Player.bmd, lấy frame đầu của action đầu. Không có model nhân vật giả.

Sau đăng nhập, chọn một nhân vật DW ở Lorencia và bấm **Xem nhân vật trong map**. App đọc world-entry thật, lấy map/X/Y/direction/class rồi mở cảnh tại vị trí đó. Phiên network được đóng sau khi nhận world-entry; cảnh chưa chạy gameplay hoặc movement.

Đã hiển thị và kiểm tra trực tiếp: terrain, texture và model đủ hình người. GUI 0.3 đã pass cả preview offline và login muverify → chọn MacGuiDW → world-entry → mở renderer ở X=135 Y=124. Lỗi vertex CGFloat 64-bit của macOS được sửa bằng API geometrySourceWithVertices; có render contract test kiểm tra tọa độ sau chuyển đổi SceneKit.

Giới hạn bản 0.3: chỉ map 0/Lorencia và mesh DW mặc định, pose tĩnh. Chưa nạp nhà/cây/static map objects, equipment theo inventory, animation theo thời gian, NPC/monster/player viewport, movement/combat/audio. Chưa có parity 100% với engine Windows. Terrain atlas 4096² trộn tile/alpha/light trên CPU; camera và renderer dùng SceneKit/Metal mới, không phải engine gốc.

Build renderer và kiểm tra:

```bash
cmake --build native/build --target MuClient-native MuClient-viewer MuClient-assets mu_client_contract mu_client_render_contract -j4
ctest --test-dir native/build -R '^mu_client_(assets|contract|render_contract)$' --output-on-failure
```

Các format hỗ trợ và nguồn đối chiếu: [ASSET-FORMATS.vi.md](ASSET-FORMATS.vi.md).

Mở `native/Start-Client.command`. App: `native/build/client/MuClient-native.app`.

## Test trên giao diện

1. Chạy MySQL, ConnectServer, JoinServer, DataServer và GameServer local.
2. Bấm **Lấy danh sách server**, chọn **Server 0**.
3. Account test **muverify**. Mật khẩu ở dòng thứ hai của `native/runtime/client/credentials.txt` (file riêng quyền 600). Không phải user MySQL.
4. Bấm **Đăng nhập**: app nhận kết quả F1:01 và danh sách nhân vật F3:00 từ server thật.
5. Tạo nhân vật với tên 1–10 chữ/số ASCII. Các class nâng cao vẫn chịu điều kiện mở class của server.
6. Chọn nhân vật rồi bấm **Xem nhân vật trong map**. App nhận packet F3:03 rồi mở preview 3D cho DW ở Lorencia. Phiên network đóng trước khi mở preview; chưa có thao tác chơi.
7. **Ngắt kết nối** đóng socket; server xử lý logout và cập nhật trạng thái account.

Client không lưu password tự động. Heartbeat C3:0E được gửi trong lobby bằng clock của phiên thật. Các thao tác network chạy trên một serial queue, không block giao diện.

## Kết quả đã xác nhận

- CLI login → list → tạo MacTestDW (DW) → list → select: PASS; map 0, vị trí 140,132, world-entry 136 byte.
- MySQL có nhân vật level 1, đúng account; ConnectStat trở về 0 sau đóng client.
- Integration test: sai password (0), thiếu account (2), serial sai (6), valid login/list (1), login trùng (3), encrypted heartbeat, reconnect: PASS.
- Contract test: credential layout/XOR, invalid input, character-list framing/slot, discovery/greeting, transport hai chiều đối chiếu hàm gốc có/không MHP: PASS.
- GUI 0.2 đã kiểm thử trực tiếp: đăng nhập muverify, tải MacTestDW, tạo MacGuiDW và chọn world-entry map 0, tọa độ 135,124: PASS.

## Build và kiểm tra

```bash
cmake --build native/build --target MuClient-native MuClient-probe MuClient-live-test mu_client_contract -j4
ctest --test-dir native/build -R '^mu_client_contract$' --output-on-failure
native/build/client/MuClient-live-test native/runtime/client/credentials.txt
native/build/client/MuClient-probe --connect-code 0 --credentials native/runtime/client/credentials.txt
native/build/client/MuClient-probe --connect-code 0 --credentials native/runtime/client/credentials.txt --select MacTestDW
```

Live test yêu cầu account muverify có nhân vật và chưa được người dùng đăng nhập ở ứng dụng khác. `--create NAME --class 0` ghi nhân vật thật; không lặp cùng tên nếu muốn test tạo mới.

## Cấu hình

GUI đọc `Contents/Resources/client.ini` trong app bundle; source cấu hình ở `native/config/client.ini` được copy khi build. CLI dùng source profile hoặc `--config FILE`.

ClientSerial/ClientVersion và key MHP nằm trong profile client, không cần đọc folder server khi chạy app. GameRuntime là fallback cho profile cũ. AssetDirectory trỏ MuClientEX603 để đọc Enc1.dat/Dec2.dat. Profile hiện dùng đường dẫn local; chưa đóng gói asset cho máy khác.

## Nguồn và phần còn lại

Protocol lấy từ ConnectServerProtocol, GameServer/Protocol.h/.cpp, HackCheck, PacketManager và Main_EX603/Main/Protocol.cpp. Dùng key thật theo chiều Enc1/Dec2; C3/C4 kiểm tra checksum từng block và serial. Không có mock nhân vật hoặc thông báo login thành công giả.

Bộ Main_EX603 là DLL hook Win32, không có engine client độc lập. Bản 0.3 có terrain/model renderer bước đầu; chưa có animation, inventory UI, viewport/entity processing, movement, combat, âm thanh hoặc kiểm thử gameplay/save đầy đủ. World-entry là bước kiểm tra network; không phải chứng nhận client chính thức hoặc tương đương 100%.

## Source tham chiếu cho 0.11

- [MovePath / Alpha](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzAI.cpp): Catmull-Rom ở t=0,25/0,5/0,75/1; native nội suy liên tục cùng đa thức, giữ nhịp server thay vì bê nguyên vòng cập nhật Windows.
- [Roof alpha](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzObject.cpp): Lorencia HouseWall05/06, AlphaTarget=0 khi HeroTile=4.
- [HeroTile](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzInterface.cpp): TerrainMappingLayer1 tại vị trí hero.

Đây là source engine tham chiếu công khai, không có bằng chứng nó đồng nhất 100% với executable EX603 của bộ Data này. Giao thức giới hạn 14 bước đối chiếu trực tiếp GameServer/Protocol.cpp và User.h trong bộ source đang dùng.

## Bản sửa 0.14 — ngày 14/10/2026

- Camera cố định, góc 45° và độ nghiêng khoảng 48,5°, FOV 35° theo tham chiếu. Renderer vẫn dùng hình học 3D như MU, nhưng góc chơi khóa kiểu 2.5D.
- Retarget ở vị trí phân số không bị nhánh phát hiện teleport nhảy thẳng tới bước chéo mới. Test giữ vị trí ngay trước/sau thay đường, tốc độ đều qua góc cong, không đi ngược, và endpoint chính xác.
- Giữ cờ LockPositions của BMD để xác định đúng số keyframe cho vòng clip; nội suy vòng cuối về đầu.
- Vật liệu Object1 dùng alpha từ OZT, repeat UV, thêm additive cho BlendMesh theo textureIndex trong BMD. Light01..03/PoseBox là mesh hiệu ứng bị HiddenMesh=-2 trong source: không vẽ chúng thành khối đen. Hiện 2.787 vật thể render, 1.556 batch; chưa dựng particle tương ứng.
- Heightmap: bfOffBits+wrapper = 1.082 nhưng OpenTerrainHeight tham chiếu đọc ở 1.084. Bộ Data cầu đặt khoảng Z=167, khớp height pixel 110 (Z=165) tại X132/Y99 sau offset tương thích; đọc BMP chuẩn cho pixel15 (Z=22,5), làm mặt cầu lõm. Đọc đúng offset legacy; hai byte cuối file thiếu được điền xác định bằng pixel cuối, không đọc bộ nhớ ngoài file như code cũ. Có regression test riêng X132/Y99.
- GPU prepare trước khi chạy cảnh; environment cull theo 256 chunk thay 1.556 node vật liệu. Khóa camera để giữ frustum ổn định.
- Log chẩn đoán không chứa account/password: /tmp/mu-client-network-performance.log (steps/ACK ms), /tmp/mu-client-render-performance.log (120 frame/max ms/over25ms). Ghi log render qua queue riêng. Phiên 0.13 cho ACK thường khoảng 0,25–0,72 ms nhưng có frame 49–96 ms; đây là bằng chứng hitch phía render trong phiên đó, không chứng minh mọi nguyên nhân đều đã hết.
- Build cuối 0.14 và cả bốn test offline pass (2,04 giây), gồm regression cao độ cầu X132/Y99. GUI online đã mở bản 0.14; người dùng xác nhận di chuyển mượt và các phần hiện tại hoạt động tốt. Không thay lời xác nhận này bằng tuyên bố parity 100%.

## Thứ tự triển khai tiếp

1. Viewport thật: spawn/despawn/update NPC, quái và người chơi theo packet GameServer EX603; map ID → model/action/texture theo source và Data. Không tạo entity mô phỏng thay server.
2. NPC tương tác: chọn đúng entity/range, gửi talk packet, mở shop/warehouse theo server reply; kiểm tra với NPC Lorencia thật.
3. Inventory/equipment: parse toàn bộ item bytes/slot/option, cập nhật mesh đúng class và item, nhận thay đổi trang bị để đồng bộ tốc độ/animation.
4. Combat thường: target/attack, animation và damage/HP/death/drop theo server, rồi skill/effect riêng.
5. UI/audio và mở rộng class/map sau khi luồng chơi cơ bản có regression test và kiểm tra online.

Mỗi mốc ghi source handler, cấu trúc packet, asset path, điều kiện hành vi và trường hợp chưa hỗ trợ. Độ tương đương 100% cần đối chiếu client EX603 gốc thực thi được; source engine công khai dùng tham chiếu chưa đủ chứng minh cùng phiên bản.
