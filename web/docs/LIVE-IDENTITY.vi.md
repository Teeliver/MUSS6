# W0057 — Identity macOS và đăng nhập thật (2026-10-25)

## Đã triển khai

- Helper native dùng gethostuuid + SHA256, định dạng 5 nhóm 8 hex uppercase, cùng cách tạo identity của native/client/client.cpp. UUID/digest không được log; helper trả identity qua stdout riêng tới Node, không ghi static file.
- HTTP helper bind 127.0.0.1:9001; chỉ GET /identity và /keys, Host chính xác, Origin http://127.0.0.1:8080. Không nhận đường dẫn file từ request, không cookie/credentials; no-store/Vary Origin, giới hạn 4 yêu cầu đồng thời và 60 yêu cầu/phút, provider timeout3s. Từ chối origin khác, query lạ, method khác; lỗi nội bộ trả503 không body.
- User đã đồng ý cấp HWID và Data/Enc1.dat, Data/Dec2.dat cho Debug local. /keys chỉ đọc đúng hai file client54byte; /identity chỉ chạy sau greeting theo identityProvider của game-handshake. Không đọc UUID hoặc HWID qua công cụ chat, không đưa vào log/file web.
- Client chọn server -> key/profile -> game channel -> greeting -> provider -> F3:09 -> ready-for-login. Không gọi identity-sent là verified: server không có ACK riêng. Form đăng nhập giữ một lần thử/phiên, không tự điền, lưu hoặc thử tài khoản, xóa ô password sau trả lời/lỗi. Sau login accepted gọi lobby.characters; roster/policy đều qua codec kiểm tra.
- Profile Debug thêm ClientVersion từ native/config/client.ini (bỏ dấu chấm, 5 ký tự), dùng đúng version client khi login, không phản chiếu version server để bỏ kiểm tra. Không đổi ClientVerify/blacklist/count hoặc gameplay server.
- 102/102 client tests và 14/14 gateway/helper tests qua. Helper tests dùng identity/keys giả, không đọc máy thật. Build Debug/Release và staging179asset qua. Release không chứa live-panel/helper; khóa/HWID không stage vào Release.

## Bằng chứng thực

Trong lượt browser này, sau người dùng thao tác đăng nhập, panel hiện đã nhận danh sách nhân vật thật (4). Code chỉ hiển thị trạng thái này sau login accepted và nhận đủ roster/policy. Không đọc hoặc ghi lại account/password người dùng nhập. Không tuyên bố đã chọn nhân vật hoặc vào world.

MySQL/CS/Join/Data/Game/gateway/static từ W0056 vẫn chạy local. Thêm native helper9001. Backup DB trước startup giữ nguyên. Không sửa Data hoặc account để test. Bằng chứng screenshot được lưu riêng tại /tmp/mu-w57-roster.png.

## Chạy lại helper

Từ root Emulator:

`mkdir -p web/.build/identity`

`clang++ -std=c++17 -Wno-deprecated-declarations web/gateway/native/identity.cpp -o web/.build/identity/mu-native-identity`

`node web/gateway/src/identity-main.mjs "$PWD/web/.build/identity/mu-native-identity" /Users/angelale/Downloads/MuClientEX603/Data`

Sau build Debug, stage profile theo sự đồng ý của user:

`python3 web/tools/stage_live_profile.py --client native/config/client.ini`

Dừng helper bằng Ctrl+C. Không bind helper ra mạng/public. Origin check bảo vệ browser khác origin, không chứng minh chống tiến trình local độc hại giả header. Thiết kế này chỉ là tích hợp local, chưa có trust/provisioning cho triển khai public. String JS và bản sao runtime không được bảo đảm zero toàn bộ; không tuyên bố không còn dữ liệu trong memory sau khi đóng.

## Chưa hoàn thiện / kế tiếp

- Màn hình hiện là panel tích hợp. Chưa scene3D chọn server/login/nhân vật, font/resource GUI gốc, model nhân vật từ roster thật, chọn nhân vật/join map hoặc world snapshot.
- Chưa test account rejection/timeout thực, nhiều tab/HWID limits và reconnect/roster server variants. Unit fixtures không thay thế live coverage.
- Chưa M03/M06 hoàn tất; full7class skills/Windows visual parity/FPS vẫn PARTIAL.
- Bước kế tiếp: đưa roster thật lên scene chọn nhân vật, hiển thị model/class/charset/trang bị tương ứng, sau đó nối enter-world và map. Không đánh dấu hoàn tất dựa trên panel text này.
