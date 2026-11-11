# Kế hoạch hành trình — MU Online S6 (EX603): server + client web (ưu tiên) → native → Windows/mobile

Cập nhật: 2026-11-11. Phạm vi: folder `Emulator/`.

## 0. Quyết định đã chốt

| # | Quyết định | Hệ quả cho kế hoạch |
|---|---|---|
| 1 | Mở cho người lạ, nhiều người chơi | Bảo mật, transaction, backup, chống gian lận, load test là **bắt buộc trước khi mở**, không để cuối |
| 2 | Client chính: **Web trước**; sau đó native macOS, Windows, mobile | Web là đường chính. Native macOS chỉ giữ để kiểm thử và ổn định server. Windows và mobile vào giai đoạn sau |
| 3 | Làm tất cả class, map, tính năng | Có khối lượng lớn; phải chia đợt, đánh dấu tiến độ theo bảng coverage |
| 4 | Giữ đúng EX603 theo source | Mọi hành vi lấy từ source GameServer/Main_EX603; không tự chế cân bằng |
| 5 | Dùng font MU gốc; sau này có tiếng Việt → kiểm tra lại | Xem mục 4 bên dưới |

## 1. Phạm vi đã đọc và giới hạn

- Đã đọc: tài liệu hướng dẫn native/web, `SOURCE-MAPPING`, `PROJECT-LAYOUT`, `STATUS` (đến W0122), `KNOWN-GAPS`, `HANDOFF` 03/11 và 05/11, `MIGRATION-STATUS`, `LOGIN-FLOW`, `LIVE-SERVER`, audit kiến trúc 23/10, audit OpenMU 23/10, cấu trúc thư mục GameServer/DataServer/JoinServer/ConnectServer.
- **Chưa đọc từng dòng**: ~166.000 dòng C++ GameServer, toàn bộ `web/` (gồm ~1.000 file JSON baseline sinh tự động), `native/build/`. Các phần này được đưa vào từng giai đoạn để đọc sâu khi làm.
- Tài liệu trong repo ghi nhiều mục PARTIAL; kế hoạch chưa tự kiểm chứng lại các mục đó.

## 2. Hiện trạng

| Thành phần | Trạng thái (theo tài liệu) |
|---|---|
| ConnectServer / JoinServer / DataServer / GameServer (native macOS) | Build được. Login, list, tạo, chọn, vào world đã pass trên client native 0.2. Gameplay và save đầy đủ **chưa xác nhận**. |
| MySQL local | 47 bảng, **10/63 procedure**. Còn **53 procedure**. |
| Client web (WebGL + WASM + gateway) | M00–M03 PARTIAL, M04–M12 TODO. Đã có: đăng nhập thật, 4 nhân vật, vào map, đi bộ từng ô, NPC/shop cơ bản, inventory, HUD, skill HUD, pet/cánh/cape, Duel, Duel Watch, party/chat một phần. |
| Client native (Cocoa, macOS) | Login → chọn server → Lorencia, di chuyển, inventory, tooltip. Chưa combat, NPC dialog đầy đủ. |
| Test | Web 501/501, native game 8/8 (05/11). |

## 3. Rủi ro phải xử lý trước khi mở cho người lạ

Đây là nhóm **chặn mở công khai** (P0). Phải xong trước khi có người chơi ngoài nhóm.

1. **Credential trong repo/file**: `native/runtime/client/credentials.txt`, `tnlabtest-credentials.txt`, `native/mysql.ini`, `web/gateway/config.local.json`, `native/runtime/backups/*.sql`. Kiểm tra git; nếu đã commit → đổi mật khẩu và xóa khỏi lịch sử.
2. **Trade/Warehouse/Shop**: audit OpenMU ghi lỗi (Zen âm khi vượt giới hạn; item và Zen không commit cùng transaction; lỗi lưu khi thoát bị nuốt). GameServer EX603 **phải kiểm tra riêng** trong `Trade.cpp`, `Warehouse.cpp`, `Shop.cpp`, `ItemManager.cpp`, `DataServer/`.
3. **Gateway public**: hiện chỉ loopback, Origin allowlist. Mở ra internet cần WSS, rate limit, giới hạn số phiên theo IP, chặn flood packet.
4. **ClientVerify/HWID (G03)**: không tắt để cho qua. Phải có cơ chế chống nhiều tài khoản/bot ở mức server (theo IP/serial/limit) trước khi mở.
5. **Lưu dữ liệu**: backup tự động, khôi phục đã thử, cảnh báo khi lưu thất bại. Procedure thiếu (53) gây lỗi lúc chơi.
6. **Đường dẫn sai trong README** (`SOURCE/Source/Emulator` → đúng là `Source/Source/Emulator`).

## 4. Font (quyết định 5)

- Bộ Data MU không có font TTF; client EX603 dùng **font bitmap** (`Data/Interface/Font.OZT`, `font_*.OZT`) và font hệ thống Gulim cho text (`Text_eng.bmd[0]`).
- Nguồn gốc đã thấy trong máy: `Font.OZT`, `FontSC.OZT`, `FontInput.OZT`, `Font_Damage.OZT` (trong bộ TitanMu/MuClientEX603), `Mu-Season17/Client/Data/Interface/GFx/fonts_*.swf` (font GFx Flash, không dùng cho EX603). `ARIALUNI.TTF` nằm trong TitanMu, không phải font MU.
- **Vấn đề tiếng Việt**: font bitmap gốc chỉ có Latin cơ bản; Gulim không có đủ dấu tiếng Việt. Khi làm tiếng Việt sẽ phải tạo atlas bitmap mới theo cùng kích thước ô/glyph, hoặc dùng font TTF tương đương có ghi rõ là thay thế.
- **Việc làm**: giai đoạn 3 (web M04) — dựng font atlas từ `Font.OZT` theo đúng `GlobalText`, kiểm tra ảnh so với native; ghi vào `KNOWN-GAPS` mục G14 là đã xử lý phần Latin. Tiếng Việt: để sang đợt riêng, kiểm tra lại từng bảng glyph.

## 5. Hành trình theo giai đoạn

Thứ tự: **server an toàn → web chơi được → web đủ tính năng → mở người chơi → native/Windows/mobile**. Native được giữ để kiểm thử song song, không làm trước web.

### Giai đoạn 0 — An toàn và mốc gốc (1–2 ngày)

- [ ] `git status`/`.gitignore`: credential, `mysql.ini`, `config.local.json`, `runtime/`, `backups/`, `dist/`, `node_modules/` không được commit.
- [ ] Sửa đường dẫn sai trong README/RUN-AND-TEST.
- [ ] Backup `MuOnline` có ngày; ghi quy trình khôi phục và thử một lần.
- [ ] Chạy test lấy mốc: native `ctest`, web `npm --prefix web test`, Python `unittest`.

**Đạt khi**: mốc test ghi vào file này; không credential trong git; có backup đã khôi phục thử.

### Giai đoạn 1 — Server đúng dữ liệu (2–3 tuần)

- [ ] Hoàn thiện 53 procedure còn lại theo `native/db/query_catalog.json`. Thứ tự: login/logout → character → item/warehouse → guild → quest → event → cash shop → reset.
- [ ] Mỗi procedure có test trên DB thử (như `test_character_procedures.py`).
- [ ] Trade/Warehouse/Shop: item và Zen trong **cùng transaction**; kiểm giới hạn Zen trước khi cộng; không rollback RAM sau khi đã commit DB.
- [ ] Lưu khi thoát: không nuốt lỗi; có retry và log cảnh báo.
- [ ] Kịch bản tay (`RUN-AND-TEST.vi.md` mục 7): tạo → vào map → đi → đánh → nhặt → đổi đồ/kho/trade → thoát → đăng nhập lại → kiểm level/Zen/inventory/warehouse.

**Đạt khi**: kịch bản pass 2 lần liên tiếp; dữ liệu đúng sau restart; test transaction trade có lỗi giả lập ở từng mốc commit.

### Giai đoạn 2 — Web: vào game và đi lại đúng (M04–M07, 4–6 tuần)

1. **M04 — Resource**: manifest + loader có hash; audit toàn bộ Data (G04); font atlas Latin từ `Font.OZT` (mục 4).
2. **M05 — Scene**: loading → server → login → chọn nhân vật → world tự động, bỏ màn debug `?scene=servers`. Dùng camera/world 74/78 theo source.
3. **M06 — World state**: HUD cập nhật liên tục HP/SD/Zen/MP/AG/EXP (hiện chỉ snapshot — W0058).
4. **M07 — Di chuyển**: click tìm đường giống Windows, chạy, đổi map qua cổng, không giới hạn 12 ô (hiện 12 ô — W0060).
5. Đo FPS trên cảnh thật (CPU/GPU p95) trước khi ghi "60 FPS".

**Đạt khi**: nhân vật đi bộ/chạy/click giữa Lorencia, Noria, Devias; đổi map đúng; không mất kết nối trong 30 phút; FPS đo và ghi vào `STATUS.vi.md`.

### Giai đoạn 3 — Web: combat và vật phẩm (M08–M09, 6–10 tuần)

1. **Item**: tooltip đầy đủ (socket, harmony, 380, set, ancient, wing/pet); đồ rơi/nhặt (G07–G09); swap/stack theo PC.
2. **Quái**: binding 311/313 loại, drop, EXP, chết/hồi sinh.
3. **Skill**: 7 class đầy đủ; 51 skill "derived" chưa port đủ (W0115) — port theo source `CreateEffect`/`CreateJoint`.
4. **NPC/shop/kho/chaos box** theo GameServer tương ứng.
5. Evil Spirit (G10): kiểm miss/range/HP trước khi kết luận lỗi server.

**Đạt khi**: combat + drop + EXP + shop chạy trên web với 2 nhân vật; mỗi class có bảng coverage skill/animation/effect/sound.

### Giai đoạn 4 — Web: đủ class, map, âm thanh (M10–M11, 8–12 tuần)

- Thứ tự class: DW → Elf → MG → DL → Summoner → RF → GL (mỗi class đạt Done với bằng chứng test + kiểm tay).
- Thứ tự map: Lorencia → Noria → Devias → Dungeon → Atlans → Tarkan → Icarus → Kanturu/Raklion và event (Blood Castle, Devil Square, Illusion Temple, Duel, Chaos Castle, Castle Siege).
- Âm thanh: mapping đầy đủ, timing (G13).
- Party, guild, friend, chat/whisper, quest.

**Đạt khi**: mỗi class và map có trạng thái Done trong `KNOWN-GAPS` (G15 được đóng dần).

### Giai đoạn 5 — Hardening và mở người chơi (song song từ giai đoạn 1, chốt trước khi mở)

- [ ] Gateway: WSS, Origin allowlist, rate limit per IP, giới hạn packet/giây, timeout bắt tay.
- [ ] Chống multi-account/bot: giới hạn theo IP/serial; log hành vi đáng ngờ (không tắt ClientVerify).
- [ ] Backup DB tự động hằng ngày; thử khôi phục hằng tuần.
- [ ] Soak test 4 giờ với 50–100 client giả; ghi RTT, RAM, số phiên đóng bất thường, deadlock.
- [ ] Runbook: khởi động/dừng đúng thứ tự, xử lý sự cố DB, rollback phiên bản.
- [ ] Trang đăng ký/quản trị tối thiểu (tạo tài khoản, ban, xem log).

**Đạt khi**: soak test không crash, không mất dữ liệu sau restart, có runbook và backup đã khôi phục thử.

### Giai đoạn 6 — Mở beta cho người lạ (nhóm nhỏ trước)

- Mở theo đợt: 20 → 100 → 500 người chơi, theo dõi từng đợt.
- Thu lỗi qua form/issue; dùng skill `report` cho báo cáo lỗi.

### Giai đoạn 7 — Client native macOS, Windows, mobile

- **Native macOS**: giữ đồng bộ với server; dùng để kiểm thử hồi quy server.
- **Windows**: build từ `native/` với backend POSIX→Win32 (adapter đã có `MU_NATIVE_MACOS` để tham chiếu); hoặc đóng gói client web bằng Electron/Tauri (cần quyết định).
- **Mobile**: client web tối ưu touch (đề xuất trước), native sau nếu cần. Quyết định khi tới giai đoạn này.

## 6. Thứ tự 4 tuần đầu

| Tuần | Việc chính |
|---|---|
| 1 | Giai đoạn 0 xong. Đọc sâu `Trade.cpp`, `Warehouse.cpp`, `ItemManager.cpp`, `DataServer/QueryManager.cpp`. Liệt kê lỗi trade. |
| 2 | Giai đoạn 1: procedure login/character/item. Test trên DB thử. |
| 3 | Giai đoạn 1: transaction trade/warehouse, kịch bản tay. |
| 4 | Giai đoạn 2 bắt đầu: M04 resource audit + font atlas Latin. |

## 7. Cách theo dõi tiến độ

- Mỗi đợt: cập nhật `web/docs/STATUS.vi.md` (web), `native/RUN-AND-TEST.vi.md` (native), và đánh dấu ô ở file này.
- Chỉ đánh dấu Done khi có bằng chứng test hoặc kiểm tay; ghi rõ phần còn thiếu.
- Cuối mỗi giai đoạn: chạy lại toàn bộ test, cập nhật `KNOWN-GAPS.vi.md`.

## 8. Việc cần bạn xác nhận sớm

1. Ngày dự kiến mở beta (ảnh hưởng thứ tự giai đoạn 5).
2. Máy chủ public: VPS nào, có domain và chứng chỉ TLS chưa? (cần cho WSS).
3. Windows: ưu tiên build native hay đóng gói web (Electron/Tauri)?
4. Tiếng Việt: khi nào bắt đầu đợt font tiếng Việt (sau beta hay trước)?

---

## 9. Quy tắc làm việc (cập nhật 2026-11-11)

1. **Không sửa gì khi chưa được hỏi.** Chỉ đọc, kiểm tra và ghi chú.
2. Nếu cần sửa: làm trên **bản sao** (thư mục tạm), test trên bản sao, báo kết quả và chờ bạn duyệt trước khi áp vào client/server thật.
3. Mọi việc, quyết định, lỗi phát hiện đều ghi vào file này (mục 10 - Nhật ký) để sau này phân tích lỗi.
4. Các câu hỏi mục 8 để lại, sẽ làm khi các hạng mục cơ bản hoàn tất.
5. Khi bạn xác nhận xong một hạng mục, mình đánh dấu **Done** trong danh sách.

## 10. Kiểm tra hiện trạng (chỉ đọc, 2026-11-11)

| # | Hạng mục | Kết quả kiểm tra | Cần làm? |
|---|---|---|---|
| A | Credential, mysql.ini, config.local.json, backup SQL có bị commit không | Đều nằm trong `.gitignore` (`native/.gitignore` dòng `mysql.ini`, `runtime/`; `web/.gitignore` dòng `gateway/config.local.json`). Không có file nhạy cảm nào trong danh sách tracked. | Không. Nhưng xem B. |
| B | Git root | `git rev-parse` trả về `/Users/angelale` (home), không phải `Emulator/`. `git status` liệt kê toàn bộ home. | **Cần bạn quyết định**: có nên tạo repo riêng cho Emulator không. Chưa làm. |
| C | Procedure MySQL | Bản gốc `~/Downloads/MuServerEX603/DB/MuOnline.sql` (UTF-16) có khoảng 60 procedure `WZ_*`/`WZ_CS_*`. `native/db/query_catalog.json` liệt kê **57** procedure. Tài liệu ghi "9/63" rồi "10/63". Số liệu chưa khớp nhau. | **Cần đối chiếu** trước khi viết thêm. |
| D | Đường dẫn sai trong README | Chưa sửa. Đường dẫn đúng là `Downloads/SOURCE-2.0.7/Source/Source/Emulator`. | Có, nhưng đợi bạn duyệt. |
| E | Test native / web | Chưa chạy lại trong lượt này (lần gần nhất 05/11: web 501/501, native 8/8). | Chạy lại khi bạn đồng ý. |
| F | Backup DB | Có file `native/runtime/backups/MuOnline-before-web-20261026-100511.sql`. Chưa thử khôi phục. | Có, cần thử khôi phục trên DB thử. |
| G | Trade/Warehouse/Shop có lỗi như audit OpenMU không | Chưa đọc sâu. | Có, đọc sau khi bạn duyệt. |
| H | Font MU | Có `Font.OZT`, `font_*.OZT` trong Data. Chưa dựng atlas. | Có, để sau. |
| I | Gateway public (WSS, rate limit) | Chưa làm, hiện loopback. | Để sau khi mở beta. |
| J | Tiếng Việt, Windows, mobile | Chưa làm. | Để sau (xem mục 8). |

**Kết luận lượt kiểm tra**: không có file nhạy cảm bị commit. Hai việc cần bạn quyết định trước là B (tạo repo riêng) và C (đối chiếu số procedure). Mình chưa sửa file nào.

## 11. Nhật ký

| Ngày | Việc | Kết quả | Ghi chú |
|---|---|---|---|
| 2026-11-11 | Lập kế hoạch, cập nhật theo quyết định | Đã ghi file PLAN | Không sửa code |
| 2026-11-11 | Kiểm tra hiện trạng (mục 10) | Đã ghi | Chỉ đọc |

## 12. Lỗi/vấn đề ghi nhận (để phân tích sau)

- **L01** — Git root là home, không phải Emulator (mục 10-B).
- **L02** — Số procedure không khớp: tài liệu ghi 63, catalog có 57, bản gốc có khoảng 60 (mục 10-C).
- **L03** — Đường dẫn sai trong `native/README.md` và `RUN-AND-TEST.vi.md`.
- **L04** — Trade/Zen theo audit OpenMU (23/10): chưa xác nhận trong EX603.

## 13. Câu hỏi: Data client khi lên online sẽ đọc và load thế nào (2026-11-11, chỉ đọc)

Trả lời ngắn: hiện **chưa có đường tải Data online**. Mọi thứ đang đọc từ đĩa local và chỉ dùng cho kiểm thử.

- **Native (macOS)**: đọc thẳng từ `MuClientEX603/Data` trên máy, qua chuỗi Boot 10 tác vụ (catalog item, panel, actor+terrain, tile/light, geometry, skeleton, body, shadow, placement, environment). Cache chỉ trong tiến trình, khởi động lại là đọc lại. Server không gửi Data; GameServer đọc Data riêng của nó (`MuServerEX603/Data`).
- **Web**: `web/package.json` có `stage:preview` chạy `tools/stage_preview.py`. Script này copy **chỉ** các file cần thiết (theo danh sách model/texture/title/skill/audio) vào `dist/debug/preview-assets/Data/`, kèm manifest (path, size, sha256). Trình duyệt tải `/preview-assets/Data/<path>`, kiểm size và SHA-256 rồi mới giải mã trong WASM. Script ghi rõ: không expose toàn bộ Data, không expose key/config.
- **Loader có kiểm hash** (`platform/assets/load-resource.js`): giới hạn 64 MiB mỗi model, 2 request đồng thời, deadline 15 giây, không tự tải lúc mở app, không cache ngầm, không fallback model khác khi hash sai.
- **Chưa có** (theo `RESOURCE-LOADER`, `RESOURCE-MANIFEST`): HTTP staging đúng cho production, scheduler/cache/version, trust/chữ ký manifest, progress nhiều asset.

### Ý nghĩa khi đưa lên online

1. Phải có một **kho asset** (static server/CDN) để phục vụ đúng các file đã staging, không phải thư mục Data nguyên bản.
2. **Manifest phải được ký** hoặc lấy từ nguồn tin cậy. Hiện runtime chỉ kiểm format và hash, chưa kiểm chữ ký.
3. **Version phải khớp** giữa client Data, manifest và server Data (`MuServerEX603/Data`). Lệch version gây sai item/monster/skill mà không báo lỗi rõ.
4. Lần vào đầu tiên phải tải một lượng lớn (Data ~911 MB, 13.660 file). Cần tải theo map/nhân vật, có cache trình duyệt theo hash.
5. **Vấn đề pháp lý**: Data MU gốc có bản quyền. Đưa công khai cho người lạ cần cân nhắc rủi ro; đây là câu hỏi cho bạn, không phải kỹ thuật.

### Đề xuất (chưa làm, cần bạn duyệt)

- Thiết kế đường tải online: manifest có chữ ký, kho asset theo version, tải theo map, cache theo hash.
- Thêm một bước kiểm version giữa client và server trước khi vào game.

### Ghi nhận vào nhật ký

- **L05**: Chưa có đường tải Data online; web chỉ có staging local (`stage_preview.py`).
- **L06**: Manifest chưa có chữ ký/trust.
- **L07**: Chưa có kiểm version Data giữa client và server.
- **L08**: Vấn đề bản quyền khi phân phối Data MU công khai (cần bạn quyết định).

## 14. Kế hoạch tải asset online và tối ưu màn loading (đề xuất, chưa làm)

### 14.1 Số đo thực (chỉ đọc, 2026-11-11)

| Nhóm | Kích thước thô | Nén gzip (ước tính) | Ghi chú |
|---|---|---|---|
| Toàn bộ Data | ~900 MB, 13.664 file | — | Không tải hết |
| `Interface/` | 41 MB | ~15 MB | Nhiều OZJ/OZT đã nén sẵn nên gzip ít hiệu quả |
| UI màn login (LSBg, lo_*, login_*, server_*, New_lo_*...) | ~4,5 MB | ~1,5–2 MB (ước tính) | Cần để hiện form login |
| `World74` (nền chọn server) | 2,5 MB | 1,5 MB | |
| `Object74` (vật thể nền) | 13 MB | 9,2 MB | **Nặng nhất** màn chọn server |
| `World78` / `Object78` | 2,7 MB / 4,2 MB | — | Biến thể nền khác (theo source) |
| `Player/` | 37 MB | 26,5 MB | Model nhân vật, tải theo class đã chọn |
| `Enc1.dat`, `Dec2.dat` | — | — | **Khóa mã hóa gói tin**. Không được đưa lên web/CDN |

Kết luận từ số đo: màn chọn server phải tải ~15–20 MB nếu tải hết. Phải tách thành phần cần ngay và phần có thể tải nền.

### 14.2 Có nên nén zip rồi đọc không?

**Không nên dùng zip cho trình duyệt.** Lý do:
- Zip phải tải xong toàn bộ file mới giải nén được; không hiển thị được từng phần khi đang tải.
- Trình duyệt không có API đọc zip gốc. Phải dùng thư viện JS (fflate) thêm vào bundle.
- Không tận dụng được cache theo từng file.

**Nên dùng** kết hợp 3 lớp:

1. **Nén ở tầng HTTP**: server gửi `Content-Encoding: br` (Brotli) hoặc `gzip` cho từng file. Trình duyệt tự giải nén, code không cần đổi. Lưu ý: hash kiểm tra phải tính trên dữ liệu đã giải nén (loader hiện đã làm vậy).
2. **Gom theo màn (pack)** để giảm số request, không gom toàn bộ: `login.pack`, `charselect.pack`, `world-lorencia.pack`... Mỗi pack có **chỉ mục** (tên, offset, kích thước, sha256) ở đầu. Trình duyệt tải bằng HTTP Range để lấy từng file trong pack khi cần. Đây là cách thay cho zip.
3. **Tên file theo hash** (ví dụ `LSBg01.<sha8>.ozj`): cho cache vĩnh viễn (`Cache-Control: immutable`). Đổi nội dung thì đổi tên, không lo cache cũ.

Khuyến nghị: **pack có chỉ mục + Brotli + Range + tên theo hash**. Zip chỉ dùng khi làm bản tải thủ công.

### 14.3 Thứ tự tải và tối ưu màn loading

Mục tiêu: người chơi thấy form login trong vài giây; phần 3D nền tải tiếp.

| Bước | Tải gì | Chặn UI? | Ghi chú |
|---|---|---|---|
| 0 | `index.html`, JS/WASM core, `manifest.json` nhỏ | Có | WASM nên có cache và tải song song |
| 1 | UI login (~4,5 MB, nén ~2 MB) | Có | Hiện form login ngay khi đủ |
| 2 | Chọn server: `World74` + `Object74` | Không | Tải nền sau khi hiện form; có thanh tiến trình |
| 3 | Đăng nhập xong: danh sách nhân vật, `Player` class đã chọn | Có (chỉ phần cần) | Chỉ tải model class của nhân vật |
| 4 | Vào map: terrain + object của map đó | Có | Có màn loading theo map |
| 5 | Âm thanh, hiệu ứng, model phụ | Không | Tải nền, không chặn chơi |

Các kỹ thuật cụ thể:
- **Hiện form trước, 3D sau**: màn chọn server dùng ảnh nền tĩnh (đã có `LSBg*.OZJ`) trong lúc `World74/Object74` tải nền. Chỉ bật camera tour khi đủ dữ liệu.
- **Prefetch**: khi người chơi đang gõ tài khoản, tải sẵn `World75/79` và model `Player` class đã biết từ lần trước.
- **Cache trình duyệt**: Cache API hoặc IndexedDB theo sha256; lần sau chỉ kiểm manifest.
- **Service Worker**: cache offline cho UI và pack đã dùng; chỉ cache sau khi hash khớp.
- **Song song có giới hạn**: 4–6 request đồng thời (hiện loader mặc định 2 — cần cân nhắc tăng khi làm thật).
- **Giảm kích thước gốc**: không đổi file gốc. Nếu cần, chỉ tạo bản nén khác có hash riêng, không sửa Data gốc.

### 14.4 Mục tiêu đo lường (đề xuất, cần bạn duyệt)

- Form login hiện trong **≤ 3 giây** trên mạng 20 Mbps, máy desktop thường.
- Lần thứ hai (có cache): form login ≤ 1 giây.
- Vào map đầu tiên ≤ 10 giây (không tính mạng chậm).
- Không kèm `Enc1.dat`, `Dec2.dat`, config, credential trong bất kỳ pack nào. Có test tự động kiểm.

### 14.5 Các bước làm (trên bản sao, chờ bạn duyệt)

1. Tạo script đóng gói: đọc manifest, sinh pack theo màn, tính sha256, nén Brotli. Chạy trên thư mục tạm.
2. Đo kích thước và thời gian tải của các pack (Chrome, mạng giả lập 20 Mbps và 5 Mbps).
3. Thêm test: pack không chứa file ngoài danh sách; hash khớp; `Enc1/Dec2` bị chặn.
4. Chỉ sau khi bạn duyệt số đo mới áp vào client thật.

### 14.6 Câu hỏi cần bạn trả lời (sẽ làm khi các việc cơ bản xong)

1. Mục tiêu thời gian chờ chấp nhận được? (đề xuất mục 14.4)
2. Có cho phép dùng CDN bên thứ ba không, hay chỉ server riêng?
3. Chấp nhận tải Object74 nền (9 MB) sau khi đã vào form login không?

### 14.7 Nhật ký

- **L09**: Data chứa `Enc1.dat` và `Dec2.dat` là khóa gói tin. Không được đưa ra ngoài, kể cả trong pack.
- **L10**: Loader web hiện mặc định 2 request đồng thời; chưa tối ưu cho tải nhiều file nhỏ.
- **L11**: Chưa đo thời gian tải thực trên mạng thật; số ở mục 14.1 là kích thước, không phải thời gian.

## 15. Kiến trúc đọc asset: local và online phải dùng chung một đường (đề xuất, chưa làm)

### 15.1 Hiện trạng (đã đọc code, chỉ đọc)

- **Native**: đường dẫn thư mục Data được truyền thẳng vào mọi nơi. Ví dụ `WorldAssets::dataDirectory` là `std::filesystem::path`; `mu::assets::load_terrain(data, map)`, `load_actor_catalog(data)`, `playerMeshes(..., dataDirectory)`, và `NSSound initWithContentsOfFile:` dùng đường dẫn trực tiếp. Nghĩa là code **gắn với đĩa**: muốn đọc từ mạng phải sửa khắp nơi.
- **Web**: đã đi theo hướng đúng hơn: `fetch(url)` theo đường dẫn logic, kiểm size + SHA-256, rồi mới giải mã trong WASM. Nhưng nguồn chỉ là `dist/debug/preview-assets/` do script staging copy vào, nên chỉ dùng được ở máy dev.
- Hai client đang đi **hai đường khác nhau**, và đường native không thể dùng trực tiếp cho online.

### 15.2 Nguyên tắc đúng

1. **Bộ giải mã chỉ nhận bytes, không nhận đường dẫn.** Hàm như `load_terrain` nên nhận một nguồn asset (`AssetSource`), không nhận `std::filesystem::path`. Ví dụ: `load_terrain(source, "World1/EncTerrain1.map")`.
2. **Tên logic giống nhau ở mọi nơi.** Ví dụ `World74/Object74/...` là cùng một tên trên máy, trên web và trên CDN. Chỉ nguồn đọc thay đổi.
3. **Mỗi file có sha256 và kích thước trong manifest.** Client chỉ dùng file đã khớp manifest. Không có fallback, không dùng file thay thế khi thiếu.
4. **Manifest có version.** Client gửi version lên server khi đăng nhập; lệch version thì từ chối vào game, báo rõ cần cập nhật.
5. **Nguồn tin cậy.** Manifest phải được ký (chưa có). Enc1/Dec2 không bao giờ nằm trong manifest.

### 15.3 Thiết kế đề xuất

```text
Logic path:  World74/EncTerrain74.map        (cùng tên ở mọi nơi)
                     │
                AssetSource (interface, client-core)
           ┌─────────┼──────────────────────┐
  LocalDirectory   HttpCache (web)     HttpCache (native online)
  đọc đĩa,         fetch + hash +      cùng logic, lưu cache
  kiểm manifest    cache Cache API /   trên đĩa theo sha256
  (dev, native)    OPFS theo sha256
```

- **Máy dev / native local**: `LocalDirectory` đọc file gốc, nhưng vẫn kiểm sha256 với manifest khi khởi động. Nếu Data trên máy khác manifest thì báo lỗi, không im lặng chạy.
- **Online**: `HttpCache` lấy file qua URL có hash trong tên (`/a/<sha256>`), `Cache-Control: immutable`. Lần sau lấy từ cache trình duyệt/đĩa, không cần kiểm lại từ server.
- **Nén**: để HTTP làm (Brotli/gzip). Không dùng zip ở đường chính. Gom pack chỉ là tối ưu sau, không đổi kiến trúc.

### 15.4 Lộ trình chuyển (từng bước, trên bản sao, chờ duyệt)

1. **Kiểm kê**: đếm mọi chỗ native dùng đường dẫn Data (grep `dataDirectory`, `initWithContentsOfFile`, `fopen`, `ifstream`, `contentsOfFile`). Lập bảng file → hàm.
2. **Định nghĩa `AssetSource`** trong `client-core` (C++ thuần, không phụ thuộc macOS) với các hàm: `read(path) -> bytes`, `size(path)`, `has(path)`. Có fixture test.
3. **Chuyển bộ giải mã** từ path sang `AssetSource`, từng module, mỗi module có test so với kết quả cũ (phải giống hệt byte-by-byte).
4. **Thêm `LocalDirectory`** và xác nhận native vẫn chạy y như cũ (test hồi quy `ctest` + kịch bản tay).
5. **Tạo bộ build manifest + object store**: quét danh sách file cần, tính sha256, sinh `manifest.json` (có version, có chữ ký) và thư mục `objects/<sha256>`.
6. **Thêm `HttpCache` cho web** dùng manifest đó (web đã có phần fetch + hash, chỉ cần đổi nguồn và thêm cache).
7. **Kiểm version giữa client và server**: GameServer phải trả manifest version khi bắt tay; client so sánh trước khi vào world.
8. **Chỉ sau khi test trên bản sao đạt**, mới áp vào client thật.

### 15.5 Điều kiện nghiệm thu

- Native với `LocalDirectory` cho kết quả giống hệt trước khi đổi (so sánh hash của terrain, mesh, texture đã giải mã).
- Web với `HttpCache` tải đúng file, từ chối file sai hash, không tải file ngoài manifest.
- Lệch version manifest: client báo lỗi rõ, không vào game.
- Không có Enc1/Dec2 hoặc file config trong object store (có test tự động).

### 15.6 Ghi nhận

- **L12**: Native đang gắn đường dẫn Data vào nhiều chỗ (world_view.mm, audio, cursor, equipment). Muốn online phải có lớp `AssetSource`.
- **L13**: Web đang đọc từ `preview-assets` do script debug tạo, chưa có nguồn online thật.
- **L14**: Manifest chưa có chữ ký và chưa được server dùng để kiểm version.

### 15.7 Câu hỏi cần bạn trả lời (khi tới bước này)

1. Native online có cần tải asset qua mạng không, hay vẫn dùng Data trên máy người chơi (kèm kiểm manifest)?
2. Ai là người giữ kho asset và ký manifest?
