# W0055 — Bước đầu kết nối server thật (2026-10-25)

## Đã xác nhận

- Debug có `?scene=servers`: runtime discovery dùng WebSocket browser thật -> gateway loopback:9000 -> ConnectServer-native loopback:44405. Đây là màn kiểm tích hợp tạm, chưa GUI gốc/scene3D.
- Packet thực: greeting C1 04 00 01, tiếp theo C2 F3:EA danh sách tên server; reply list F4:06 có count0 khi chưa GameServer online. Không giả đăng ký GameServer hoặc bơm server vào danh sách.
- Sửa discovery đón F3:EA đúng schema nguồn ConnectServerProtocol.h/.cpp: header6byte và record34byte (WORD index + charName[32]), giới hạn2048byte, không trùngindex, một gói tên mỗi exchange. Vẫn từ chối packet lạ, sai size hoặc lặp. Tên render bằng textContent. Không reset deadline do packetmetadata.
- 102/102 npmtests qua, buildDebug/Release qua, staging179asset. Thêm hồi quy greeting+names chungchunk và malformed/repeatednames. Browser nhận thông báo ConnectServer đã trả lời/chưaGameServeronline.
- Đã bật local: ConnectServer TCP44405/UDP55557, gateway9000, staticweb8080; tất cả bind127.0.0.1. Không bật GameServer/JoinServer/DataServer/MySQL; không dùng tài khoản/identity và không ghiDB.

## Chạy lại

Từ rootEmulator:

`native/build/connect/ConnectServer-native --config native/config/connect --host 127.0.0.1`

`node web/gateway/src/main.mjs web/gateway/config.local.json`

`python3 -m http.server 8080 --bind 127.0.0.1 --directory web/dist`

Mở `http://127.0.0.1:8080/debug/?scene=servers`.

Gateway local config chứa routeconnect->44405 vàgame-0->55900; không credential. Không sửa gatewayconfig nếu máy đã cófile. Hiện tiến trình dùng bảnconfigtạm tương đương /tmp/mu-web-live-gateway.json. Dừng bằngCtrl+C ở từng tiến trình. Không tự bật dịch vụ qua việc mở webpage.

## Kế tiếp / chưa xong

1. Provision profile/Enc1/Dec2 và identity provider cho browser theo cơ chế server hiện có; không tự sinh identity để qua ClientVerify, không tắt kiểmtra.
2. ChạyGameServer và dependency thực, xác nhận tuyến0/55900 +greeting. Trước test có ghiDB cần backup. Đăng nhập thật và nhậnlistnhânvật chưa kiểm.
3. Nối scene lifecycle loadingData -> server -> login -> character -> world, dựa resource/camera/animation gốc; UI tạm này không được coi là hoàn thànhscene.
4. World snapshot/movement/vitals/equipment, rồi inventory/combat. 7classskill/Windowsvisualparity/FPS vẫnPARTIAL nhưW0054.

Luồngweb chỉ đạt discovery thật; M03/M06 chưa hoàn tất. Không gọi đây là đã vàoGameServer hoặc đã chơionline.
