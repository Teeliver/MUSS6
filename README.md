# T-NLabs · MU Online EX603 Web

Web client và bộ ConnectServer, JoinServer, DataServer, GameServer native cho MU EX603. README này hướng dẫn đưa bản hiện tại lên **máy Mac chạy từ xa để test kín**, rồi chuẩn bị chuyển sang **web công khai qua HTTPS**.

## 1. Chọn hình thức triển khai

| Mục tiêu | Hạ tầng | Trạng thái |
|---|---|---|
| Test từ xa với nhóm nhỏ | Mac Apple Silicon chạy game, Docker Desktop chạy MySQL, người test kết nối qua SSH | Giữ nguyên các URL loopback của bản hiện tại |
| Người chơi vào bằng tên miền | Máy chủ, tên miền, HTTPS/WSS, bản client đã chuyển khỏi chế độ local | Cần hoàn thành các thay đổi ở mục 7 |
| VPS Linux | Native server và helper đã được port, build và test trên Linux | CMake native hiện chặn nền tảng ngoài Apple |

GitHub dùng để lưu source và tài liệu. GameServer, MySQL, Gateway và Identity cần tiến trình chạy trên máy chủ.

## 2. Chuẩn bị máy Mac chạy server

Máy Mac hiện tại có thể làm host test. Với Mac mới, cài Xcode Command Line Tools, CMake, Ninja, pkg-config, MySQL client library, Docker Desktop và Node **26.11.1** kèm npm **11.20.0**. Các lệnh build dưới đây chạy từ thư mục gốc của repo.

```bash
xcode-select -p
cmake --version
ninja --version
pkg-config --modversion mysqlclient
docker info --format '{{.ServerVersion}}'
node --version
npm --version
```

Chép riêng bộ runtime MuServer EX603 đã dùng thành công, thư mục Data của client và bản dump MySQL đã kiểm tra. Giữ cấu trúc thư mục GameServer và Data dùng chung. Launcher hiện tìm Node ở `/opt/homebrew/bin/node`, client Data ở `~/Downloads/MuClientEX603/Data` theo đường dẫn tuyệt đối của máy phát triển. Trên Mac mới hoặc tài khoản khác, cập nhật `NODE` và `DATA` trong `native/tools/mu_web_stack.py` theo kết quả `command -v node` và đường dẫn Data thực tế.

Điền đường dẫn tuyệt đối tới thư mục **GameServer** của bộ runtime trong `native/config/game/runtime.ini`:

```ini
[GameServer]
RuntimeDirectory=/absolute/path/to/MuServerEX603/GameServer
```

Dùng [hướng dẫn MySQL](native/LOCAL-MYSQL.vi.md) để cấu hình `native/mysql.ini` từ [file mẫu](native/config/mysql.ini.example) và tạo user runtime có quyền phù hợp trên `MuOnline`. Khi chuyển máy, dùng dump **MySQL** của bộ đang chạy. Bộ `MuOnline.sql` gốc dạng T-SQL cần qua quy trình [chuyển schema](native/db/README.md).

## 3. Build và cấu hình

```bash
cmake -S native -B native/build -G Ninja -DBUILD_TESTING=ON
cmake --build native/build -j 4
ctest --test-dir native/build --output-on-failure
npm --prefix web/gateway ci --ignore-scripts --omit=optional
npm --prefix web run build:wasm
npm --prefix web run build:debug
npm --prefix web test
npm --prefix web run test:gateway
```

Build native cần đường dẫn khóa kiểm thử đúng máy theo [native/README.md](native/README.md). Build Identity helper và tạo profile từ cấu hình client riêng đã điền đúng trên host:

```bash
mkdir -p web/.build/identity
clang++ -std=c++17 -Wno-deprecated-declarations \
  web/gateway/native/identity.cpp -o web/.build/identity/mu-native-identity
python3 web/tools/stage_live_profile.py --client native/config/client.ini
```

Chi tiết cấu hình ở [LIVE-IDENTITY.vi.md](web/docs/LIVE-IDENTITY.vi.md). Chép `runtime`, `preview-assets`, `audio-assets`, `ui-assets` và `map-title-assets` của bản Debug hoạt động nếu bộ deploy đang dùng có các thư mục đó. Đặt profile tại `web/dist/debug/runtime/live-profile.json`. Dùng profile đúng version/serial với GameServer và khóa đúng bộ EX603. [Quickstart hiện tại](native/MU-WEB-QUICKSTART.vi.md).

Tạo `web/gateway/config.local.json`:

```json
{
  "port": 9000,
  "origins": ["http://127.0.0.1:8080"],
  "routes": {
    "connect": {"host": "127.0.0.1", "port": 44405},
    "game-0": {"host": "127.0.0.1", "port": 55900}
  }
}
```

Giữ địa chỉ GameServer được quảng bá trong `native/config/connect/ServerList.dat` là `127.0.0.1:55900` cho sơ đồ này. Browser nối WebSocket vào Gateway, còn Gateway nối TCP vào các route nội bộ.

## 4. Khởi động và kiểm tra tại máy host

Mở Docker Desktop rồi chạy:

```bash
./native/Start-GameServer.command --audit-assets
./native/MU-Web.command check
./native/MU-Web.command
./native/MU-Web.command status
curl -fsS -o /dev/null http://127.0.0.1:8080/
tail -n 50 native/runtime/background/GameServer.log
```

Launcher chạy theo thứ tự MySQL → Connect → Join → Data → Game → Identity → Gateway → Web. Mở [màn chọn server](http://127.0.0.1:8080/?scene=servers) trên host. Dùng tài khoản test để đăng nhập, vào nhân vật, Move qua map và đi tiếp, sau đó thoát/vào lại để xác nhận lưu dữ liệu. Dùng hai phiên để kiểm tra nhìn thấy nhau, shop, trade và Duel. Kiểm tra thời gian tải map và RTT khi nhiều người cùng chơi. `status` xác nhận listener, còn các bước trong game xác nhận luồng thực tế.

Nếu gặp `ServerPort must be 1..65535`, kiểm tra `Data/GameServerInfo - Common.dat` trong runtime có nội dung, có mục `[GameServerInfo]` và cổng `55900`. Nếu báo lỗi khởi tạo phiên, xem cả `Identity.log`, `Gateway.log` và profile. [Chẩn đoán từng dịch vụ](native/RUN-AND-TEST.vi.md).

## 5. Cho nhóm test truy cập từ xa

Bật Remote Login trên Mac cho user test được phép, dùng khóa SSH. Dùng địa chỉ VPN riêng hoặc endpoint SSH đã cấu hình của host. Trên máy **người chơi**, mở terminal và giữ lệnh này chạy:

```bash
ssh -N -o ExitOnForwardFailure=yes -o ServerAliveInterval=30 \
  -L 127.0.0.1:8080:127.0.0.1:8080 \
  -L 127.0.0.1:9000:127.0.0.1:9000 \
  -L 127.0.0.1:9001:127.0.0.1:9001 \
  user@mac-server.example
```

Mở đúng [http://127.0.0.1:8080/?scene=servers](http://127.0.0.1:8080/?scene=servers) trên máy người chơi. Địa chỉ này giữ đúng Origin và Host mà helper hiện yêu cầu. Các cổng 8080, 9000 và 9001 phải còn trống trên máy người chơi. Chỉ cấp tunnel cho nhóm tin cậy vì luồng hiện tại lấy identity và khóa qua helper chạy trên host. Kiểm tra hai tài khoản từ hai máy và xem server xử lý identity/IP dùng chung thế nào trước khi mở rộng nhóm test.

| Cổng mặc định | Dịch vụ |
|---|---|
| 3306 TCP | MySQL |
| 44405 TCP / 55557 UDP | ConnectServer / heartbeat |
| 55970 TCP / 55960 TCP / 55900 TCP | Join / Data / Game |
| 8080 TCP / 9000 TCP / 9001 TCP | Web / Gateway / Identity |

Giữ các cổng trong bảng ở mạng nội bộ hoặc chặn bằng firewall. SSH hoặc VPN là đường vào cho nhóm test.

## 6. Đưa source lên GitHub

Tạo repo private trước. Chỉ đưa lên source, README, cấu hình mẫu và tài nguyên có quyền phân phối. Rà staged diff để loại dump người chơi, log, backup, file profile riêng, credential, khóa và Data thương mại. File đã từng commit cần xử lý cả lịch sử và thay credential đã lộ.

```bash
git rev-parse --show-toplevel
git status --short -- .
git add -- README.md
git diff --cached --stat
git diff --cached -- README.md
git remote -v
```

Nếu chưa có remote, thay URL mẫu bằng URL repo private. Trước khi push, kiểm tra toàn bộ commit sẽ được gửi.

```bash
git remote add origin git@github.com:YOUR_ACCOUNT/YOUR_REPOSITORY.git
git commit -m "Document MU Web deployment"
git push -u origin HEAD
```

## 7. Quy trình đưa lên tên miền công khai

1. **Tách cấu hình client theo môi trường.** Hiện `web/client/src/app/live-servers.js` gắn cứng Gateway và helper vào loopback. Rà toàn bộ call site bằng lệnh dưới. Đổi luồng production sang URL HTTPS/WSS cùng origin, giữ đường dẫn Gateway `/mu/<route>`, và kiểm thử server list, chuyển map, reconnect.
2. **Tách identity, khóa và asset.** Helper hiện kiểm tra Origin/Host local và trả khóa qua `/keys`. Thiết kế luồng phiên production trước khi mở public. Giữ khóa bí mật ở backend, xác thực phiên ở WebSocket upgrade và định nghĩa identity theo phiên hoặc thiết bị theo yêu cầu của GameServer. Chỉ phục vụ asset client được phép phát hành qua manifest.
3. **Dựng staging HTTPS.** Trỏ DNS tới host staging. Dùng reverse proxy có TLS để phục vụ web tĩnh và chuyển WebSocket `/mu/` tới Gateway loopback. Allowlist đúng origin HTTPS và kiểm tra phiên trước khi chuyển request. Chỉ mở 443 và 80 nếu cần chuyển hướng hoặc cấp chứng chỉ. Giữ cổng SSH giới hạn cho quản trị. Dùng trình quản lý dịch vụ của host để tự khởi động lại tiến trình và quản lý log.
4. **Tối ưu sau khi đo.** Build release rồi kiểm tra luồng đăng nhập và tải asset vì bản đã chạy hiện dùng Debug. Phục vụ JS/CSS/WASM có MIME đúng và bật nén HTTP. Cache dài cho asset có hash trong tên, cho HTML/profile tái xác thực để phát hành bản mới mà người chơi không phải xóa cache. Đo tải map lạnh/ấm, RTT, RAM và giới hạn kết nối trước khi chỉnh Gateway.
5. **Sao lưu và diễn tập nâng cấp.** Dump MySQL bằng tài khoản backup riêng với thông tin kết nối lưu ngoài Git. Thử phục hồi vào database staging. Giữ bản deploy trước đã kiểm chứng để rollback. Khi thay bản chạy, bảo trì có thông báo, deploy artifact theo commit, kiểm tra sức khỏe từng dịch vụ rồi test hai tài khoản từ mạng ngoài trước khi mở lại.

```bash
rg -n '127\.0\.0\.1:(8080|9000|9001)|/keys|/identity|live-profile' \
  web/client web/gateway/src
npm --prefix web run build:release
```

Các bước public ở trên là phần cần triển khai và kiểm thử. Bản hiện tại vẫn dùng client local và native macOS, nên cấu hình reverse proxy đơn thuần chưa đủ để mở đăng nhập qua tên miền.
