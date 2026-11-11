# Chạy và test từng server EX603 native trên Mac

Mở các Terminal riêng cho Connect, Join, Data, Game. Trong mỗi Terminal chạy:

```bash
cd "/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator"
```

Thứ tự khởi động: MySQL → Connect → Join → Data → Game. Thứ tự dừng: Game → Data → Join → Connect, bằng Ctrl+C trong Terminal đang chạy từng chương trình; sau cùng dừng MySQL bằng launcher `--stop` nếu cần. Các lệnh bên dưới không thêm `; exit;`.

## 1. Build và test offline

```bash
cmake -S native -B native/build -G Ninja -DBUILD_TESTING=ON
cmake --build native/build -j 4
ctest --test-dir native/build --output-on-failure
```

Hiện có20 test. `100% tests passed` xác nhận các contract offline: cấu hình/discovery, framing, MySQL dialect/result, ABI, cipher với khóa thật, effect, weighted pointer, callback lifetime và timer. Không kết nối VPS hoặc chạy gameplay trong các test này.

## 2. ConnectServer — kiểm tra độc lập được ngay

ConnectServer đã được khởi động trong phiên làm việc này; dùng `lsof` để xác nhận trạng thái hiện tại trước khi mở thêm instance:

```bash
lsof -nP -iTCP:44405 -sTCP:LISTEN
lsof -nP -iUDP:55557
```

Nếu có `ConnectSe ... TCP *:44405 (LISTEN)` và UDP55557 thì dùng server đang chạy. Nếu cả hai cổng trống, khởi động:

```bash
native/Start-ConnectServer.command
```

Log cần thấy:

```text
Native Connect TCP listening at 0.0.0.0:44405
Native Connect UDP listening at 0.0.0.0:55557
```

Kiểm tra cấu hình mà không mở cổng:

```bash
native/Start-ConnectServer.command --check-config
```

Thành công có `Configuration loaded by original C++ loaders. No sockets were opened.`

Test protocol thật, chạy từ một Terminal khác:

```bash
python3 native/tools/probe-connect.py
```

Kết quả cần thấy `PASS: ConnectServer handshake C1:00 and server-list response F4:06.` Tool chỉ nhận handshake và gửi request danh sách server; không tạo heartbeat giả, không login hoặc ghi database. `Available servers: 0` vẫn có thể đúng khi Join/Game chưa chạy, Game chưa quảng bá hoặc entry đang HIDE. Test này không xác nhận UDP; `lsof` chỉ xác nhận UDP đã bind, còn luồng heartbeat được test khi Join/Game chạy thật.

Config native: `native/config/connect`. Cổng44405 TCP phục vụ client, cổng55557 UDP nhận heartbeat của Join/Game.

## 3. MySQL — chuẩn bị trước Join/Data

Hiện dùng MySQL local trên Mac, trong container `mu-native-mysql`. Docker Desktop cần đang chạy. Bật/kiểm tra bằng:

```bash
native/Start-MySQL.command
native/Start-MySQL.command --check-status
```

Endpoint là `127.0.0.1:3306`, database `MuOnline`, TLS required. Launcher thoát khi database sẵn sàng; MySQL tiếp tục chạy trong Docker. Dữ liệu được giữ tại `native/runtime/mysql/data`. Chi tiết và cách dừng: [LOCAL-MYSQL.vi.md](LOCAL-MYSQL.vi.md).

File `native/mysql.ini` đã có cấu hình local và credential được tạo tự động; không copy đè bằng template. Cấu hình trước đó được lưu riêng trong `native/runtime/mysql/mysql.previous.ini`. Biến môi trường `MU_MYSQL_*`, nếu đang đặt, sẽ ghi đè giá trị trong file. Các key ODBC/SQL Server trong INI runtime cũ không dùng để kết nối native MySQL.

### Tham khảo: kết nối qua tunnel SSH tới VPS

Nếu chuyển sang MySQL trên VPS, lệnh tunnel được gõ trong Terminal Mac:

```bash
ssh -L 3307:127.0.0.1:3306 bcons@103.70.13.189
```

Sau khi SSH đăng nhập thành công, chính cửa sổ Terminal đó chạy shell trên VPS. Giữ cửa sổ này mở để tunnel hoạt động. Chạy Join/Data trong một tab Terminal Mac khác. Nếu cổng3307 đã có tunnel đang nghe, dùng phiên SSH đó; mở thêm cùng lệnh có thể báo cổng đã được sử dụng.

Luồng kết nối là `Join/Data trên Mac → 127.0.0.1:3307 trên Mac → SSH → 127.0.0.1:3306 trên VPS`. Vì vậy `Host=127.0.0.1` và `Port=3307` trong `native/mysql.ini` là đúng khi dùng tunnel này.

SSH mã hóa đoạn Mac–VPS. `SslMode=required` còn yêu cầu MySQL hỗ trợ TLS riêng; tunnel không làm MySQL tự bật TLS. Giữ chế độ này theo lựa chọn hiện tại. Lỗi `TLS is required but the server does not support TLS` yêu cầu cấu hình TLS ở MySQL trên VPS trước khi thử lại.

`required` bắt buộc mã hóa kết nối; xác minh CA thuộc `verify_ca`, và xác minh thêm hostname thuộc `verify_identity`. `SslCa` để trống tự nó không gây lỗi với `required`. Giữ `SslMode=required` trong luồng hiện tại. [Các chế độ TLS của MySQL](https://dev.mysql.com/doc/refman/8.4/en/using-encrypted-connections.html).

Để xác định phiên bản và cách MySQL chạy, gõ trong cửa sổ đã đăng nhập VPS:

```bash
mysql --version
docker ps --format '{{.Names}} {{.Image}} {{.Ports}}'
```

Nếu lệnh báo `command not found` hoặc `permission denied`, giữ cả thông báo để chọn bước kiểm tra tiếp theo. Phiên bản từ `mysql --version` là phiên bản công cụ client; cần đối chiếu phiên bản server trước khi thay cấu hình TLS. Không cần gửi mật khẩu SSH hoặc MySQL.

Database cần có schema/procedure MU đã chuyển tương thích. Launcher local mới tạo database và tài khoản; MuOnline hiện chưa có bảng/procedure MU. Bộ mới có `MuServerEX603/DB/MuOnline.sql` dạng T-SQL với47 bảng và63 procedure; cần chuyển sang MySQL, backend không tự import nó.

Trong công cụ quản trị MySQL, chọn đúng database rồi có thể kiểm tra read-only:

```sql
SHOW TABLES;
SHOW PROCEDURE STATUS WHERE Db = DATABASE();
```

Đây chỉ là danh sách object, chưa xác nhận contract và dữ liệu. Join cần MEMB_INFO và các procedure account; Data cần AccountCharacter, Character, OptionData, Guild, GuildMember, GameServerInfo cùng các WZ_* procedure phù hợp. Không đủ object hoặc sai procedure vẫn có thể mở listener nhưng login/save sẽ lỗi.

## 4. JoinServer — account và xác thực

Cách mở thuận tiện: `native/Start-JoinServer.command`. Executable trực tiếp cũng tự tìm config và file MySQL; các lệnh explicit bên dưới vẫn hợp lệ. Có thể dùng `native/Start-JoinServer.command --check-config` và `--check-db` cho hai bước kiểm tra.

Chạy kiểm tra INI/MySQL settings, không kết nối database:

```bash
native/build/support/JoinServer-native \
  --config native/config/join \
  --mysql-config native/mysql.ini \
  --check-config
```

Kết quả: `No database connection or listener opened.`

Kiểm tra kết nối MySQL thật:

```bash
native/build/support/JoinServer-native \
  --config native/config/join \
  --mysql-config native/mysql.ini \
  --check-db
```

Kết quả cần thấy `Connected to MySQL. Schema/procedure behavior has not been validated. No listener opened.` Lệnh này kết nối rồi thoát; chưa chạy server, chưa test đăng nhập account.

Sau khi bước trên pass, chạy thật trong Terminal Join:

```bash
native/build/support/JoinServer-native \
  --config native/config/join \
  --mysql-config native/mysql.ini
```

Terminal sẽ tiếp tục chạy. Kiểm tra ở Terminal khác:

```bash
lsof -nP -iTCP:55970 -sTCP:LISTEN
```

Phải có `JoinServe ... TCP 127.0.0.1:55970 (LISTEN)`. Log sau khi mở cổng là `Native JoinServer TCP listening at 127.0.0.1:55970; MySQL connected.` Các dòng config/MySQL settings phía trước chỉ là preflight. Join gửi heartbeat tới127.0.0.1:55557.

Fixture Join trong native/config/join hiện có `MD5Encryption=0`. Chọn0/1 theo representation account của database; MU digest16 byte không giống chuỗi MD5 hex32 ký tự. AllowableIpList cho phép GameServer tại127.0.0.1.

## 5. DataServer — nhân vật, đồ, kho, guild

Cách mở thuận tiện: `native/Start-DataServer.command`. Executable trực tiếp cũng tự tìm config và file MySQL. Có thể thêm `--check-config` hoặc `--check-db` vào launcher để kiểm tra trước khi chạy.

Kiểm tra offline:

```bash
native/build/support/DataServer-native \
  --config native/config/data \
  --mysql-config native/mysql.ini \
  --check-config
```

Kiểm tra MySQL:

```bash
native/build/support/DataServer-native \
  --config native/config/data \
  --mysql-config native/mysql.ini \
  --check-db
```

Hai bước có các thông báo tương ứng như JoinServer. Sau đó chạy ở Terminal Data:

```bash
native/build/support/DataServer-native \
  --config native/config/data \
  --mysql-config native/mysql.ini
```

Kiểm tra:

```bash
lsof -nP -iTCP:55960 -sTCP:LISTEN
```

Phải có `DataServe ... TCP 127.0.0.1:55960 (LISTEN)` và log `Native DataServer TCP listening at 127.0.0.1:55960; MySQL connected.` Data startup đọc Guild/GuildMember; source có thể tiếp tục sau lỗi query, nên mở cổng không chứng minh schema đúng. Đăng ký GameServer có thể tạo dòng GameServerInfo Number0 nếu chưa có. Client không kết nối trực tiếp Join/Data; test login/character/save phải đi qua GameServer.

## 6. GameServer — dùng bộ MuServerEX603

Có thể mở `native/Start-GameServer.command` hoặc executable trực tiếp. Khi không có `--config`, GameServer tìm runtime trong thư mục hiện tại rồi đọc `native/config/game/runtime.ini`; file này đang trỏ `/Users/angelale/Downloads/MuServerEX603/GameServer`. Launcher dùng `native/mysql.ini` local. Executable trực tiếp cũng tự tìm file MySQL bên cạnh runtime/executable/source, giống Join/Data.

Kiểm tra nhanh từ Terminal Mac:

```bash
native/Start-GameServer.command --check-config
```

Preflight chỉ xác nhận INI. Bộ MuServerEX603 đã nạp world thành công với Join/Data TCP kết nối; database MU đã nhập 47 bảng và 9 procedure; login/list/create/select đã pass qua CLI client 0.2. Gameplay/save đầy đủ chưa được xác nhận. Các lệnh explicit dưới đây vẫn dùng được.

Kiểm tra startup INI:

```bash
native/build/game/GameServer-native \
  --config /Users/angelale/Downloads/MuServerEX603/GameServer \
  --check-config
```

Kết quả đã xác nhận:

```text
EX603ENG native C++ GameServer: startup INI loaded; port=55900.
No gameplay/Lua initialization, database connection or listener opened.
```

Kiểm tra Data:

```bash
native/build/game/GameServer-native \
  --config /Users/angelale/Downloads/MuServerEX603/GameServer \
  --audit-assets
```

Bộ MuServerEX603 có đủ116 input paths được tham chiếu và61 file Monster/Spawn. Audit bỏ qua Item/Item.bmd vì đây là output của WriteItemInfo, không phải input startup. Kết quả là0 file input thiếu. Bộ MuServer cũ và các mapping trước đó chỉ còn là tham khảo trong game/DATA-COMPATIBILITY.md.

Khi Data đã được chuyển đúng và schema MySQL sẵn sàng, chạy ở Terminal Game:

```bash
native/build/game/GameServer-native \
  --config /Users/angelale/Downloads/MuServerEX603/GameServer \
  --mysql-config native/mysql.ini
```

Log cần đạt `Game world initialized. Listener=127.0.0.1:55900; Join=1 Data=1.` và listener:

```bash
lsof -nP -iTCP:55900 -sTCP:LISTEN
```

Join/Data cần có log `GameServer connected ...`; nhãn `[SocketUDP]` trong log gốc vẫn là liên kết TCP này. Game gửi A1 heartbeat tới Connect, Join gửi A2; Connect mới có điều kiện quảng bá Game trong F4:06. Join=1/Data=1 chỉ là kết nối TCP, còn login/save cần procedure đúng và test thực tế.

INI Game hiện đã trỏ Join127.0.0.1:55970, Data127.0.0.1:55960, Connect127.0.0.1:55557. Data chung nằm ở /Users/angelale/Downloads/MuServerEX603/Data, anh em với GameServer.

## 7. Test client — sau khi các lớp trên hoạt động

Client EX603 đã build/chạy được cần ConnectServer address của Mac + port44405, GameServer address/port55900 trong ServerList/MapServerInfo, cùng version/serial/cipher phù hợp source. Nếu client chạy máy Windows khác, dùng IP LAN của Mac cho client và địa chỉ Game được quảng bá, chạy Game với `--host 0.0.0.0`. Join/Data có thể giữ127.0.0.1 khi cùng máy Mac. Source Main_EX603 chưa build không đủ để chạy client.

Test tuần tự trên account thử:

1. Lấy danh sách server, chọn server.
2. Đăng nhập, nhận danh sách nhân vật.
3. Tạo nhân vật, vào map, di chuyển, đánh quái.
4. Đổi đồ/kho/giao dịch và kiểm tra từng chức năng cần dùng.
5. Thoát game, đăng nhập lại để kiểm tra level/Zen/inventory/warehouse đã lưu.
6. Dừng Game → Data → Join → Connect rồi bật lại, xác nhận dữ liệu vẫn đúng.

Client macOS 0.2 đã pass CLI login/list/create/select; hướng dẫn: [client/README.vi.md](client/README.vi.md). Gameplay còn chưa triển khai. Cổng mở/handshake/test offline không thay thế các bước gameplay và lưu/khôi phục.

## 8. Lỗi thường gặp

| Thông báo/mã | Cách kiểm tra |
|---|---|
| Address already in use | `lsof -nP -iTCP:PORT -sTCP:LISTEN` hoặc `lsof -nP -iUDP:PORT`; xác định đúng instance trước khi dừng. |
| Missing runtime file | --config phải là thư mục có INI/asset tương ứng, không trỏ vào executable hoặc Data chung. |
| MySQL2003 | Host/port và listener MySQL; với local, kiểm tra Docker và `Start-MySQL.command --check-status`; với VPS, kiểm tra đường mạng/tunnel. |
| MySQL2005 | Hostname không resolve. |
| MySQL1045 | User/password và quyền account theo nguồn kết nối. Qua tunnel, kết nối đến từ phía VPS: thường `localhost/127.0.0.1` khi MySQL chạy trực tiếp, có thể là địa chỉ bridge/proxy khi qua Docker. Đối chiếu nguồn MySQL thực thấy, không dùng IP Mac. |
| MySQL1049 | Tên database chưa tồn tại/sai tên. |
| MySQL2026 — TLS is required but the server does not support TLS | Cấu hình TLS trên đúng instance MySQL theo phiên bản thực tế; giữ `SslMode=required`. |
| MySQL2026 — TLS certificate verification failed | Kiểm tra certificate/CA và thời hạn theo chế độ xác minh đang dùng. |
| MySQL2026 — TLS protocol or cipher mismatch | Đối chiếu TLS version/cipher được client và server hỗ trợ. |
| MySQL2026 — TLS handshake failed (cause not classified) | Chưa xác định nguyên nhân; thu thập phiên bản server và metadata TLS trước khi sửa cấu hình. |
| MySQL1146 | Thiếu bảng hoặc sai casing/schema. |
| MySQL1305 | Thiếu procedure cần migrate. |
| GameServer không vào được | Kiểm tra Data load, Join/Data connection, allowlist, port55900 và endpoint client được quảng bá. |

Ctrl+C dừng foreground process; với instance chạy bởi Codex, tra PID bằng lsof rồi dùng `kill -INT PID`. Dừng Game trước để nó có cơ hội gửi dữ liệu trong khi Data/Join còn chạy.
