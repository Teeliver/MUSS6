# EX603ENG C++ native macOS

Hướng dẫn chạy và test từng server, kèm kết quả mong đợi: [RUN-AND-TEST.vi.md](RUN-AND-TEST.vi.md).

Bản port dùng gameplay/protocol C++ gốc, CMake, mạng POSIX thay Winsock/IOCP, console/timer thay WinMain/GDI, Lua 5.2.4 và QueryManager dùng MySQL C API. Không cần Node.js. Profile: UPDATE603, LANGUAGE1, SHOP3, EXTRA1, ENCRYPT_STATE1.

## Build

Chạy trong `/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator`:

```bash
cmake -S native -B native/build -G Ninja -DBUILD_TESTING=ON
cmake --build native/build -j 4
ctest --test-dir native/build --output-on-failure
```

Máy này đã có Apple Clang, CMake, Ninja, pkg-config và libmysqlclient. Lua được build từ source chính thức trong `vendor`; không dùng thư viện Windows `.lib`. Test cipher dùng khóa thật ở MuServer và MuClientEX603; đổi `MU_TEST_SERVER_KEY_DIR`/`MU_TEST_CLIENT_KEY_DIR` khi cần.

| Server | Executable tính từ Emulator |
|---|---|
| ConnectServer | `native/build/connect/ConnectServer-native` |
| JoinServer | `native/build/support/JoinServer-native` |
| DataServer | `native/build/support/DataServer-native` |
| GameServer | `native/build/game/GameServer-native` |

## Chạy ConnectServer

Mở `native/Start-ConnectServer.command` hoặc chạy executable ConnectServer trực tiếp. Khi không có `--config`, nó tìm bộ ba file cấu hình trong thư mục hiện tại, thư mục `config` bên cạnh executable, rồi `native/config/connect` của source checkout. Log in rõ thư mục được dùng.

```bash
native/Start-ConnectServer.command
```

Bộ `native/config/connect` dùng cổng gốc TCP44405/UDP55557, giữ thông tin server trong ServerList.dat và chuyển SHOW/HIDE thành1/0, ánh xạ MaxIpConnection10 thành MaxConnectionPerIP10. MaxPacketPerSecond20 và MaxConnectionIdle30 giây là preset native vì INI cũ không có hai giá trị đó; lifetime tính từ lúc kết nối, không từ packet cuối. BlackList.txt bắt đầu không chặn địa chỉ nào. Các file MuServer gốc không bị sửa. Dừng bằng Ctrl+C. GameServer cần gửi heartbeat để xuất hiện trong danh sách server khả dụng.

## MySQL local trên Mac

MySQL 8 chạy trong container `mu-native-mysql`, publish `127.0.0.1:3306`, dữ liệu nằm trong `native/runtime/mysql/data`. Docker Desktop cần đang chạy. Mở `native/Start-MySQL.command` để bật database; launcher thoát khi database sẵn sàng, container tiếp tục chạy. Thêm `--check-status` để kiểm tra hoặc `--stop` để dừng và giữ dữ liệu. Hướng dẫn: [LOCAL-MYSQL.vi.md](LOCAL-MYSQL.vi.md).

JoinServer/DataServer cũng tự tìm config đúng role trong thư mục hiện tại, `config/<role>` cạnh executable hoặc `native/config/<role>` của source. Nếu không truyền `--mysql-config`, chúng tìm `mysql.ini` gắn với config/executable/source; không tự dùng file example hoặc file mysql.ini không liên quan trong HOME. Các flag explicit vẫn ưu tiên và báo lỗi nếu đường dẫn sai. Log chỉ in đường dẫn, không in credential.

Có thể mở `native/Start-JoinServer.command` và `native/Start-DataServer.command` để chạy. Kiểm tra trước:

```bash
native/Start-JoinServer.command --check-config
native/Start-JoinServer.command --check-db
native/Start-DataServer.command --check-config
native/Start-DataServer.command --check-db
```

`native/mysql.ini` hiện đã trỏ database local `MuOnline`, với tài khoản runtime riêng và `SslMode=required`; không copy template đè lên file này. Cấu hình VPS cũ được lưu riêng trong `native/runtime/mysql/mysql.previous.ini`. Các file credential/runtime được ignore. Biến môi trường `MU_MYSQL_*` có thể thay giá trị trong file.

```bash
native/build/support/JoinServer-native --config native/config/join --mysql-config native/mysql.ini --check-config
native/build/support/DataServer-native --config native/config/data --mysql-config native/mysql.ini --check-config
```

`--check-config` không mở listener/database. `--check-db` kiểm tra kết nối MySQL thật, chưa xác nhận bảng/procedure. Bật MySQL trước; sau đó chạy Connect → Join → Data → Game trong bốn terminal khi schema và Data đã sẵn sàng.

Backend chuyển các query đã biết sang MySQL; **MuOnline local đã nhập 47 bảng, 11 dòng seed và 9/63 procedure; CLI client 0.2 đã pass login/list/create/select**. Bộ mới đã có `/Users/angelale/Downloads/MuServerEX603/DB/MuOnline.sql` dạng T-SQL với47 bảng và63 procedure để chuyển. Chi tiết: `db/README.md`, `db/query_catalog.json`. SQL tùy ý trong Lua chưa được chuyển và hiện bị từ chối.

## Bộ MuServer hiện có

```bash
native/build/game/GameServer-native --config /Users/angelale/Downloads/MuServerEX603/GameServer --check-config
native/build/game/GameServer-native --config /Users/angelale/Downloads/MuServerEX603/GameServer --audit-assets
```

Runtime đang dùng MuServerEX603: có đủ116 input paths được tham chiếu và61 file spawn. Item/Item.bmd là output, không bắt buộc startup. Đã test khởi tạo world và listener127.0.0.1:55900 thành công, Join/Data TCP đều kết nối. Các đối chiếu bộ MuServer cũ chỉ còn là tham khảo trong game/DATA-COMPATIBILITY.md.

Giữ Data chung là thư mục anh em của GameServer vì source dùng `../Data`. Build thành công chưa chứng minh parser Data cũ khớp, procedure MySQL đúng hoặc client đã đăng nhập/chơi/lưu được. Chưa chạy client thật với server native.

## Khác biệt và giới hạn

- Console thay UI GDI/menu Windows; loại crash dump DbgHelp và marker đóng gói Themida. Mã hóa packet dùng logic gốc, kiểm tra với khóa thật.
- Dispatch và timer gameplay dùng một vòng owner; query/Lua chậm có thể làm chậm tick. Giữ chu kỳ gốc, tránh dồn callback sau tick trễ.
- Sửa pointer 64-bit, callback qua token DWORD, packed records và endian/unaligned cipher; giữ độ rộng field giao thức.
- SIGINT/SIGTERM đóng client và cố gửi hết hàng đợi Join/Data trong thời hạn. Lỗi/timeout báo thất bại; gửi TCP xong chưa xác nhận database commit.
- Framing, buffer và cipher được kiểm tra; toàn bộ variable-length packet schema và hành vi runtime còn cần kiểm chứng.

Test offline kiểm tra ABI, cipher, weighted pointer, callback lifetime, timer, INI/file/time, framing và SQL dialect/result. Chúng không thay thế kiểm tra login, tạo nhân vật, đánh quái, sự kiện, giao dịch và lưu/khôi phục trên MySQL thật.

## Client kiểm thử native macOS

Mở `native/Start-Client.command` để chạy client Cocoa ARM64 0.14: lấy danh sách server, đăng nhập, tạo/chọn DW và vào Lorencia online. Account test muverify, password ở dòng hai native/runtime/client/credentials.txt. Hướng dẫn chi tiết: [client/README.vi.md](client/README.vi.md).

Có nhà/cây/static objects từ Data gốc, camera khóa kiểu MU, tìm đường bằng chuột, idle/walk/run theo điều kiện trang bị, bóng Shadow01.bmd và phiên online liên tục. Terrain dùng texture gốc/UV toàn map; height reader tương thích offset legacy để sửa cầu X132/Y99. Bốn test cuối pass; người dùng xác nhận bản 0.14 di chuyển mượt. Chưa có viewport NPC/quái/người chơi khác, hội thoại, UI inventory/equipment hay combat. Start-WorldViewer.command mở preview offline.
