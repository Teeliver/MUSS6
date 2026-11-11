# MySQL local cho EX603 native trên Mac

Database chạy bằng image MySQL 8 ARM64 đã có trong Docker trên Mac. MU Connect/Join/Data/Game vẫn là các executable C++ native. Docker Desktop cần đang chạy.

## Bật database

Trong Terminal Mac:

```bash
cd "/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator"
native/Start-MySQL.command
```

Launcher tạo hoặc khởi động container `mu-native-mysql`, chờ database sẵn sàng rồi thoát. MySQL tiếp tục chạy trong Docker; đóng Terminal launcher không dừng database. Chạy launcher lần nữa dùng lại dữ liệu hiện có.

Endpoint cho MU là `127.0.0.1:3306`, database `MuOnline`. MySQL chỉ publish cổng trên loopback Mac. Tài khoản runtime riêng yêu cầu TLS và có quyền SELECT/INSERT/UPDATE/DELETE/EXECUTE trong MuOnline. Charset bootstrap là latin1; collation cần đối chiếu khi chuyển schema gốc.

`native/mysql.ini` là cấu hình local đang dùng. Mật khẩu được tạo tự động, lưu ở các file riêng có quyền600; không cần đưa mật khẩu vào lệnh hoặc gửi trong chat. Cấu hình VPS trước đó được sao lưu trong `native/runtime/mysql/mysql.previous.ini` trước khi chuyển. Dữ liệu, cấu hình và credential local nằm trong `native/runtime/mysql/`, đã được gitignore.

## Kiểm tra

```bash
native/Start-MySQL.command --check-status
native/Start-JoinServer.command --check-db
native/Start-DataServer.command --check-db
```

Join/Data cần báo `Connected to MySQL. Schema/procedure behavior has not been validated. No listener opened.` Đây là kiểm tra kết nối, chưa xác nhận đăng nhập hoặc lưu nhân vật.

Đã kiểm tra trong phiên thiết lập: MySQL8.0.46, phiên TCP dùng TLSv1.3, cả hai executable C++ Join/Data trả exit0 với `--check-db`. MuOnline có0 bảng và0 routine tại thời điểm này.

Có thể kiểm tra phiên TCP đang dùng TLS bằng MySQL client trên Mac:

```bash
mysql --defaults-file="/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator/native/runtime/mysql/client.cnf" \
  --no-login-paths --batch \
  -e "SELECT VERSION(); SHOW SESSION STATUS WHERE Variable_name IN ('Ssl_version','Ssl_cipher');"
```

Ssl_version/Ssl_cipher cần có giá trị. File client.cnf chứa credential; chỉ dùng đường dẫn của file trong lệnh.

## Dừng và chạy lại

Dừng Game, Data, Join trước nếu đang chạy, rồi:

```bash
native/Start-MySQL.command --stop
```

Lệnh stop giữ nguyên dữ liệu. Bật lại bằng `native/Start-MySQL.command`. Không xóa container hoặc thư mục data để sửa lỗi kết nối.

## Phần MU còn thiếu

MuOnline đã nhập 47 bảng và 11 dòng seed từ MuServerEX603/DB/MuOnline.sql; đã chuyển 9/63 procedure phục vụ login và character world-entry. Client native 0.2 đã pass login/list/create/select qua CLI, account trở về offline sau đóng kết nối. Các tính năng khác vẫn cần 54 procedure còn lại và kiểm thử gameplay/save. Chi tiết: [db/MIGRATION-STATUS.vi.md](db/MIGRATION-STATUS.vi.md).

Hướng dẫn các server: [RUN-AND-TEST.vi.md](RUN-AND-TEST.vi.md).
