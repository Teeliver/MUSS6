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
