# Baseline W0001 — 2026-10-23

## Bằng chứng đã tạo

- Data: 13.660 file thường, 911.910.018 byte (869,67 MiB); 0 lỗi đọc, 0 symlink và 0 collision khi casefold.
- Có 5.183 `.bmd`, 5.618 `.ozj`, 1.668 `.ozt`, 911 `.wav`, 66 `.att`, 57 `.map`, 57 `.obj`, 47 `.ozb`; không có MP3/TTF/TTC/WOFF trong lần quét Data này.
- Không coi toàn bộ `.bmd` là model; có cả bảng dữ liệu dùng chung đuôi file. Chưa decode toàn bộ.
- Source archive: 649 file, 909.754 byte gzip. Các root: GameServer484, Main_EX603 extension70, native/client88, native/src1, native/include4, tools1, tests1.
- Config: hash-only 22 file (native/config14, GameServer/DATA8); không lưu plaintext config vào source archive và chưa có backup config để phục hồi.
- Data manifest fingerprint: `d9e9688e669ba976e91017a26f08e1dc6e4408e103912951894de2418fbd1c74`.
- Source archive SHA-256: `633f0dddc4b0a4830dd0b4964bce4fd03c6aa8c79a542a3ae64335cf4092dbd0`.
- Native client version đọc từ Info.plist: 0.29.7. Có bốn contract native chạy lại, không clean-rebuild hoặc test GUI/online trong đợt này.

Archive chỉ bao gồm file source được allowlist theo extension/CMakeLists, không gồm toàn bộ repository, vendor, executable, native/mysql.ini, database, credential hay tài nguyên Data. Đây là snapshot M00 trước khi triển khai web; không gọi là backup toàn hệ thống. Không chia sẻ thư mục baseline cục bộ ra ngoài tự động.

Artifact: `web/.local-baselines/2026-10-23-m00-001/{data.manifest.json,*.manifest.json,source.tar.gz,summary.json}`. Metadata có thể tái lập nếu bytes không đổi; gzip/tar không được hứa cho cùng archive hash giữa các lần tạo.

## Tái lập

Từ thư mục Emulator, chọn output mới vì tool từ chối ghi đè baseline:

```sh
python3 -m unittest discover -s web/tests -v
python3 -u web/tools/baseline.py \
  --data /Users/angelale/Downloads/MuClientEX603/Data \
  --source server-ex603=GameServer \
  --source main-ex603-extension=../Main_EX603/Main \
  --source native-client=native/client \
  --source native-platform=native/src \
  --source native-platform-headers=native/include \
  --source audit-tools=web/tools \
  --source audit-tests=web/tests \
  --config native-config=native/config \
  --config game-config=/Users/angelale/Downloads/MuServerEX603/GameServer/DATA \
  --output web/.local-baselines/NEW-BASELINE-ID
```

Tool chỉ đọc nguồn, không theo symlink, không chạy decoder/server/DB. Output là thư mục cục bộ mới có quyền0700, file manifest/archive tạo riêng. Không sửa tool sau khi tạo baseline rồi vẫn gọi snapshot cũ là code mới; đợt sau tạo baseline mới.

## Ma trận kiểm tra đã chốt cho bản thử

Máy đã xác minh: Apple M2 / RAM8GiB, macOS26.3, Safari26.3. Đây là phiên bản đọc từ máy, không phải đề nghị nâng/hạ hệ điều hành. Chưa chạy WebGL/browser benchmark; chưa xác nhận Chrome đã cài.

| Ca | Browser/resolution | Cảnh/tải | Tiêu chí dự kiến | Đã đo? |
|---|---|---|---|---|
| P01 | Safari1280x720, DPR1 | Lorencia/DW idle60s, chạy tuyến cố định120s | Mục tiêu60FPS; p95 frame<=16,67ms, p99<=33,3ms | Chưa |
| P02 | Safari1280x720, DPR2 | Cùng P01 | Ghi riêng chi phí DPR, không gộp số trung bình với DPR1 | Chưa |
| P03 | Safari1280x720 | Scene fixture20 quái di chuyển, 10 item, skill đồng thời có số lượng cố định trong test | Ghi CPU/GPU/frame/hitch/draw calls/bộ nhớ; giới hạn FX không tăng vô hạn | Chưa |
| P04 | Chrome, version phải ghi khi test | Lặp P01–P03 | So sánh cùng Data hash, không so các cấu hình khác nhau | Chưa |
| P05 | Safari và Chrome | RTT0/50/150/300ms, jitter/loss/disconnect | Không nhân đồ, chạy bù hoặc queue vô hạn; không dùng ngưỡng ACK local để thay benchmark | Chưa |
| P06 | Safari và Chrome | 10 vòng vào/ra map, tabẩn/hiện và context lost | Thu hồi GPU/cache/connection, không leak tăng đều; sau warmup so cùng điểm đo | Chưa |

P03 là fixture render để đo chi phí, không phải bằng chứng gameplay online. Chưa có số đo để cam kết tất cả trường hợp đạt60FPS. Mốc M05 phải đo cold/warm riêng và ghi p50/p95/p99, >33ms, thời gian tải, decoded texture/GPU/JS/WASM memory. Tải theo cảnh; không nạp 870MiB Data một lần vào máy8GiB RAM.

Chưa có screenshot Windows chuẩn/DPR thực trên browser, chưa có mức tải production đã thống nhất và chưa có mục tiêu mobile. Những phần này vẫn mở, không giả số liệu.
