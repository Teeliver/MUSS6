# T-NLabs · MU Online EX603 Web

Web client và bộ ConnectServer, JoinServer, DataServer, GameServer native cho MU EX603. Môi trường online đặt toàn bộ dịch vụ và bản web client đã build trên **VPS Linux**. MacBook dùng để phát triển và kiểm thử local. Người chơi tải client từ VPS và chạy game trong trình duyệt.

## Người chơi mở web là vào game

Đích triển khai là `https://play.example.com/?scene=servers`. Người chơi chỉ cần trình duyệt, tài khoản game và URL này. Kiến trúc khi mọi dịch vụ chạy trên cùng VPS:

```text
Browser ── HTTPS/WSS :443 ── Nginx
                               ├── Web release + tài nguyên được phép phát hành
                               └── /mu/ → Gateway 127.0.0.1:9000
                                             ├── Connect 127.0.0.1:44405
                                             └── Game    127.0.0.1:55900
Game ── Join/Data/Connect nội bộ ── MySQL 127.0.0.1:3306
```

### Đổi IP và cấu hình ở đâu

| Chỗ cấu hình | Giá trị cho VPS một máy |
|---|---|
| DNS của `play.example.com` | Bản ghi A trỏ tới IPv4 public của VPS. Thêm AAAA nếu VPS đã phục vụ IPv6 |
| Client discovery và kết nối game | Lấy host từ `window.location.host`. Discovery gọi `wss://play.example.com/mu/connect`. Game gọi route `/mu/game-0` trên cùng tên miền |
| `web/gateway/config.local.json` | `origins: ["https://play.example.com"]`. Giữ route TCP `connect` → `127.0.0.1:44405` và `game-0` → `127.0.0.1:55900` |
| `native/config/connect/ServerList.dat` và bảng route client | Với topology web này, giữ giá trị được quảng bá `127.0.0.1:55900` khớp bảng route client. Browser đi qua WSS và Gateway mở TCP |
| `GameServerInfo - Common.dat` | Giữ các địa chỉ Connect/Join/Data là `127.0.0.1` khi cùng VPS. Giữ cổng khớp cấu hình từng dịch vụ |
| `native/mysql.ini` | Dùng `127.0.0.1:3306` khi MySQL cùng VPS. Điền database, user và mật khẩu riêng của VPS |
| Firewall VPS | Mở TCP 443, TCP 80 nếu cần chuyển HTTPS/cấp chứng chỉ, và SSH cho quản trị. Chặn truy cập Internet tới các cổng game, MySQL, Gateway và helper |

### Thứ tự triển khai public

1. Hoàn thành build native Linux và kiểm tra MySQL theo mục **Triển khai toàn bộ hệ thống trên VPS Ubuntu**. Dùng DB staging để thử restore, đăng nhập và lưu nhân vật.
2. Sửa client và backend theo mục 6. Bản hiện tại gọi trực tiếp `127.0.0.1:9000/9001` trên máy người chơi. Public cần endpoint web cùng origin, luồng identity phù hợp và khóa bí mật được xử lý tại backend. Giữ helper local và endpoint `/keys` ngoài các route public.
3. Build và chuyển bản release đã kiểm tra tới `/srv/mu-web/current`. Trỏ DNS vào VPS, cài chứng chỉ cho tên miền, rồi cấu hình Nginx. Sau đây là phần đặt **trong server block HTTPS đã có chứng chỉ**, sau khi hoàn thành bước 2:

   ```nginx
   root /srv/mu-web/current;
   index index.html;

   location / {
       try_files $uri $uri/ =404;
   }

   location /mu/ {
       proxy_pass http://127.0.0.1:9000;
       proxy_http_version 1.1;
       proxy_set_header Upgrade $http_upgrade;
       proxy_set_header Connection "upgrade";
       proxy_set_header Host $host;
       proxy_set_header Origin $http_origin;
       proxy_read_timeout 120s;
       proxy_buffering off;
   }
   ```

   Cấu hình xác thực WebSocket upgrade theo luồng phiên ở bước 2 trước khi bật route này. Kiểm tra và nạp cấu hình bằng `sudo nginx -t && sudo systemctl reload nginx`.

4. Từ máy ngoài mạng VPS, mở URL game bằng cửa sổ riêng tư. Kiểm tra Network có HTTPS thành công và WSS trả `101`. Đăng nhập hai tài khoản, vào map, Move, reconnect và kiểm tra lưu nhân vật. Nếu WSS báo `403`, kiểm tra Origin và phiên. Nếu `502`, kiểm tra listener Gateway và log Nginx. Nếu mở web được nhưng vào game lỗi, kiểm tra route Game và log Connect/Game.
5. Đo trước khi chỉnh hiệu năng. Chọn VPS gần nhóm người chơi, đặt Game và MySQL cùng vùng mạng, dùng SSD. Đo RTT trong HUD và thời gian tải map lần đầu/lần sau bằng hai máy ngoài mạng VPS. Cache dài cho file có hash, cho HTML tái xác thực mỗi lần mở, bật nén cho JS/CSS/WASM và giữ asset tĩnh đi trực tiếp qua Nginx. Khi test đông người, đối chiếu CPU, RAM, I/O và log giới hạn Gateway trước khi tăng số phiên.

## Triển khai toàn bộ hệ thống trên VPS Ubuntu

GitHub lưu source và tài liệu. VPS chạy MySQL, ConnectServer, JoinServer, DataServer, GameServer, Gateway và Nginx. Nginx phục vụ bản web client đã build. VPS phải phục vụ được game khi MacBook đã tắt.

Để bỏ phụ thuộc Mac, cần port và xác nhận build Linux cho bốn executable C++, thay phần Identity phụ thuộc macOS, và chuyển client sang cấu hình HTTPS/WSS theo mục 6. Dùng VPS x86-64 Ubuntu với SSD, địa chỉ IP cố định và tên miền. Có thể lấy **4 vCPU, 8 GB RAM, 80 GB SSD** làm cấu hình staging ban đầu nếu chạy cả MySQL cùng máy. Đây là mức đề xuất để bắt đầu đo, dung lượng thực tế phụ thuộc Data, DB, log và số người chơi.

Chuẩn bị hệ điều hành và toolchain:

```bash
sudo apt update
sudo apt install -y git curl clang cmake ninja-build pkg-config \
  default-libmysqlclient-dev mysql-server nginx ca-certificates
sudo systemctl enable --now mysql
sudo mysqladmin ping
git clone YOUR_PRIVATE_REPO_URL mu-ex603
cd mu-ex603
```

Cài Node đúng phiên bản ở mục 2. Đưa server Data, MySQL dump đã chuyển đổi và asset được phép phân phối lên VPS qua SSH. Tạo cấu hình DB riêng, dùng user runtime và thử restore vào DB staging trước khi mở kết nối. Với repo hiện tại, lệnh sau sẽ dừng tại điều kiện `NOT APPLE` trong `native/CMakeLists.txt`. Dùng bước này làm mốc kiểm tra sau khi port:

```bash
CC=clang CXX=clang++ cmake -S native -B native/build-linux -G Ninja -DBUILD_TESTING=ON
cmake --build native/build-linux --parallel 4
ctest --test-dir native/build-linux --output-on-failure
```

Sau khi có bản build Linux đạt test, cấu hình các dịch vụ theo thứ tự **MySQL → Connect → Join → Data → Game → Gateway → Nginx**. Dùng đường dẫn executable Linux và file cấu hình tuyệt đối trong các unit systemd, với user dịch vụ thường và `Restart=on-failure`. Cấu hình Identity production theo kiến trúc đã chốt trước khi bật Gateway công khai. Chạy `--check-config`, `--check-db` cho Join/Data và `--audit-assets` cho Game theo [hướng dẫn native](native/RUN-AND-TEST.vi.md). Sau đó kiểm tra Nginx và URL từ máy ngoài:

```bash
sudo nginx -t
sudo systemctl reload nginx
curl -fsS -o /dev/null https://MU_DOMAIN/
```

Chỉ mở cổng quản trị SSH và HTTP/HTTPS theo mục 6. Giữ MySQL, TCP game và Gateway trong mạng nội bộ. Trước khi mở cho người chơi, kiểm tra browser nhận WebSocket `101 Switching Protocols`, đăng nhập hai tài khoản từ mạng ngoài, chuyển map, reconnect và xác nhận dữ liệu vẫn còn sau khi restart dịch vụ trên staging.

## 2. Dev local trên MacBook — chuẩn bị

Dùng máy Mac hiện tại để test local. Với Mac mới, cài Xcode Command Line Tools, CMake, Ninja, pkg-config, MySQL client library, Docker Desktop và Node **26.11.1** kèm npm **11.20.0**. Các lệnh build dưới đây chạy từ thư mục gốc của repo.

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

## 3. Dev local trên MacBook — build và cấu hình

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

## 4. Dev local trên MacBook — chạy và kiểm tra

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

## 5. Đưa source lên GitHub

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

## 6. Các thay đổi cần hoàn thành trước khi mở public

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
