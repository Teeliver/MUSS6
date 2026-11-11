# TODO client MU EX603 chạy WebGL

Trạng thái 2026-10-23: đã bắt đầu M00, đang PARTIAL; chưa có client chạy trong browser. Nhật ký từng đợt và bằng chứng: [web/docs/STATUS.vi.md](../../web/docs/STATUS.vi.md). Đợt W0001 không bật server hoặc thay cấu hình runtime.

## Nguyên tắc và phạm vi

- Server EX603 hiện tại tiếp tục quyết định nhân vật, HP/MP, damage, vật phẩm, Zen, giao dịch và lưu dữ liệu.
- Client web là frontend mới. SceneKit/Cocoa trong các file .mm phải có implementation web; không thể biên dịch nguyên ứng dụng Mac sang trình duyệt.
- SOURCE-AUDIT.vi.md đã ghi nhận Main_EX603 là extension DLL/hook, chưa có đầy đủ engine Windows. Không lấy bản Mac hoặc source tham chiếu khác phiên bản làm bằng chứng “gốc 100%”.
- Mỗi tính năng phải ghi: nguồn gốc/file/hàm, Data sử dụng, packet liên quan, hành vi mong đợi, kiểm thử, phần chưa xác nhận.
- Không sửa, đổi tên hoặc dời các thư mục Server/Data gốc. Chuyển asset bằng công cụ đọc nguồn; output là file sinh tự động và có manifest truy ngược.
- MVP đầu tiên: desktop Safari/Chrome, một DW, Lorencia. Mobile và toàn bộ class/map là các mốc mở rộng.

## Kiến trúc dự kiến

Browser TypeScript + WebGL2/Three.js + lõi C++/WASM
  -> WSS gateway (chuyển byte, giới hạn đích kết nối)
  -> ConnectServer / GameServer EX603
  -> JoinServer / DataServer / MySQL hiện có.

- WASM: codec, state, giải mã tài nguyên hoặc logic portable có lợi khi dùng chung. Quyết định sau bản thử; không ép mọi UI vào WASM.
- TypeScript: lifecycle trình duyệt, transport bất đồng bộ, input, render và audio.
- Renderer dùng material/shader tái hiện blend, alpha-test và skinning của MU; không bật mặc định kiểu ánh sáng PBR rồi coi là giống MU.
- Gateway không giả lập gameplay, không sửa damage hoặc cộng đồ, không mở quyền truy cập database cho browser.

Thư mục dự kiến, chỉ tạo khi bắt đầu triển khai:

```text
Emulator/
  native/                  # bản Mac và server hiện có
  client-core/             # lõi portable được tách dần, dùng chung native/WASM
  web/
    client/                # TypeScript, renderer, UI, audio/input adapters
    gateway/               # WSS -> TCP, cấu hình đích cố định
    tools/                 # công cụ xuất asset/manifest
    tests/                 # packet, replay, hình ảnh, online, performance
    docs/                  # mapping source, ADR, checklist nghiệm thu
```

## M00 — Chốt nguồn chuẩn và baseline

- [x] M00.1a–b: archive source và hash cấu hình đã chọn; xem BASELINE về phạm vi, không gọi đây là backup toàn bộ hệ thống.
- [ ] M00.1c: sao lưu DB trước các lần test có ghi dữ liệu; đợt này chỉ đọc file, không bật database.
- [x] M00.2: lập SOURCE-MAP — confirmed EX603 / reference khác phiên bản / implementation mới / chưa biết.
- [x] M00.3a: inventory file Data với đường dẫn, chữ hoa/thường, hash và kích thước: 13.660 file, 0 lỗi đọc.
- [ ] M00.3b: kiểm tra dependency và decode model/texture/animation/terrain/table toàn Data; không coi inventory là decoder coverage.
  - W0002 PARTIAL: đã audit 5.034 model và 7.286 OZJ/OZT bằng parser native; chi tiết `web/docs/ASSET-AUDIT.vi.md`. Terrain/table/toàn bộ frame và fallback gốc chưa xác minh.
  - W0003 PARTIAL: terrain/object 5 world hỗ trợ và 9 bảng Text/Item/Skill đọc được; 42 world, 140 BMD khác và visual/fallback vẫn chưa kiểm tra. Xem `web/docs/WORLD-AUDIT.vi.md`.
  - W0004 PARTIAL: audit cấu trúc 47 world, 176/188 file qua schema; 12 trường hợp cần đối chiếu. Không đồng nghĩa hỗ trợ/render đúng 47 map; xem `web/docs/STRUCTURE-AUDIT.vi.md`.
  - W0005 PARTIAL: điều tra 12 ngoại lệ; audit decode 4 BMP, định vị 2 record NaN + 4 scale âm. Quy tắc height/runtime chưa xác nhận; xem `web/docs/EXCEPTIONS-AUDIT.vi.md`.
  - W0006 PARTIAL: numeric animation mọi frame/locked-position qua ở 5.028/5.034 model; 6 ngoại lệ được ghi. Timing/render chưa xác minh, xem `web/docs/ANIMATION-AUDIT.vi.md`.
- [x] M00.4: ghi danh sách lỗi/giới hạn Mac vào KNOWN-GAPS, chưa tự kết luận đã sửa hết.
- [x] M00.5a: ghi máy thực và ma trận resolution/DPR/cảnh/tải, tiêu chí dự kiến trong BASELINE.
- [ ] M00.5b: chạy browser thực, đo DPR/FPS/bộ nhớ và lưu hình đối chiếu; Chrome và Windows visual baseline chưa xác minh.

Đạt khi: có baseline tái lập được, danh sách phần thiếu và ma trận tính năng. Chưa cần làm UI web đẹp.

## M01 — Bộ khung và quy ước quản lý

W0010: cây thư mục theo trách nhiệm và output Debug/Release riêng; browser in-app đã chạy WebGL2/WASM. Xem `web/docs/PROJECT-LAYOUT.vi.md`. Full native và baseline browser/performance khác vẫn chưa đóng.

- [x] Tạo web/client, gateway, tools, tests, docs; ghi ADR-001 chọn stack tối thiểu M01 và pin Node/npm (W0007). Gateway mới là thư mục, chưa có dịch vụ.
- [x] Đặt quy tắc ID item/class/map, đơn vị tọa độ, trục, endian, độ dài packet và ownership dữ liệu trong web/docs/CONVENTIONS.vi.md (W0007); chưa phải codec đã test.
- [ ] Tách build web/WASM khỏi yêu cầu Cocoa/Objective-C++ của native/client/CMakeLists.txt.
- [x] Có lệnh build/test, output allowlist 4 file trong web/dist bị ignore; chưa bundle Data/credential (W0007).

Đạt khi: build web trống được và bản native vẫn build/test được.

## M02 — Tách lõi portable

W0049 (2026-10-25): DONE menu Debug chọn class/chuyển chức/skill theo snapshot SkillList.txt, freeze Fire01; 92/92 tests qua, browser kiểm DW/DK và skill chưa hỗ trợ. PARTIAL: chỉ Fire Ball có prototype effect, chưa full skill 7 class/eligibility/Language/hit/server; M02/M03 chưa hoàn tất. Chi tiết: web/docs/PREVIEW-MENU.vi.md.


W0048: Fire01 visual prototype +7class geometry/browser audit, SkillList386mục/class-filter matrix. PARTIAL đầy đủskill/effect7class, DLspike/RFidle/Firevisual/skin/wing/GUI/network. Chi tiết `web/docs/SEVEN-CLASS-EFFECTS.vi.md`.


W0047: fixture cast147/146 đồng bộ thân/trang bị, native86.400pose samples; audit8effect assets nguồn. PARTIAL skill selector/effect/material/trajectory/sound/hit/server và lỗi skin/wing. Chi tiết `web/docs/CAST-ANIMATION.vi.md`.


W0046: attack selector theo item/hand/twoHand và clip blend đồng bộ body/attachment; fixture visual, chưa projectile/hit/damage/cast/server. Chi tiết `web/docs/ATTACK-ANIMATION.vi.md`.


W0045: fixture cung/tên Elf, staff/khiên DW qua core resolver và multi-attachment; native geometry24case + browser cycle. PARTIAL equip online/attack/projectile/effect/skin/wing và120FPS. Chi tiết `web/docs/WEAPON-LOADOUTS.vi.md`.


W0044: motion idle/walk/run native rates/rootXY/blend0.12s và giữgaitphase; demo cycle tại chỗ, interval profiling. PARTIAL movement/runpolicy/input/network/attack và120FPS display. Chi tiết `web/docs/MOTION-ANIMATION.vi.md`.


W0043: idle quaternion animation time-based body+weapon, cached source/mesh + bufferSubData; native57.600bone sample120Hz. PARTIAL GPUskin/pooledcopy/walk-run-attack/GUI và120FPS display. Chi tiết `web/docs/BODY-ANIMATION.vi.md`.


W0042: DONE action policy class/weapon, direct Item.bmd twoHand checksum, body/attachment cùng pose; numeric573.510native cases. PARTIAL frame0/animation/gameplay/skin/wing/FPS. Chi tiết `web/docs/EQUIPMENT-ACTIONS.vi.md`.


W0041: DONE geometry Sword01 gắn pose/bone/grip, native realData12trường hợp khớp. PARTIAL stance/animation/equip online/bow/shield/wing và camera game. Chi tiết `web/docs/ATTACHMENT-GEOMETRY.vi.md`.


W0040: DONE bone/grip portable, numeric reference49.152trường hợp với native SIMD. PARTIAL geometry attachment/stance chưa render; skin/wing/animation/FPS còn thiếu. Chi tiết `web/docs/EQUIPMENT-GRIP.vi.md`.


W0039: DONE mixed5slot/core resolver thống nhất, audit31fixture, 73test; browser giữ đầu class khi slotmũ511. PARTIAL skin dưới mũ/wing/weapon/animation; screenshot55FPS cần profile. Chi tiết `web/docs/MIXED-ARMOR.vi.md`.


W0038: DONE core/WASM armor resolver53.760native combinations, audit26fixture/staging162asset, 71test + test:wasm. PARTIAL skin/visibility (Summoner thiếu đầu), weapon/animation/effects/GUI. Chi tiết `web/docs/ARMOR-RESOLVER.vi.md`.


W0037: DONE base-body105path/native reference, 21class-tier asset binding audit, Debug123asset, 67test. PARTIAL equipped resolver/animation/material/Windows parity. Chi tiết: `web/docs/BODY-CLASSES.vi.md`. M02/M03 vẫn PARTIAL.


W0036 PARTIAL: full5part DW và OZJ row fix so native pixels,65/65 npm tests; browser base/helmet có ảnh. Chưa wing material/class tiers/animation/game UI/MU online. Xem `web/docs/BODY-PREVIEW.vi.md`.

W0035 PARTIAL: Player binding numeric so native,63/63 test +WASM; browser8asset. Mũ visual/wing nền đen chưa nghiệm thu, cần body/material/UV. Xem `web/docs/PLAYER-BINDING.vi.md`.

W0034 PARTIAL: browser WebGL textured3model preview,62/62 test; screenshot60FPS chỉ preview. Cánh alpha/mũ Player binding còn sai; chưa GUI game/MU online. Xem `web/docs/MODEL-PREVIEW.vi.md`.

W0033 PARTIAL: OZT pixel/OZJ wrapper + JPEG browser adapter,61/61 npm +WASM+2Python manifest tests;4texture thật wrapper-checked. Chưa browser pixel/WebGL preview/GUI. Xem `web/docs/TEXTURES.vi.md`.

W0032 PARTIAL: BMD initial pose positions/normals/UV typed buffers,57/57 test + WASM; ba model thật qua numeric checks. Chưa texture/WebGL preview/animation/GUI. Xem `web/docs/BMD-GEOMETRY.vi.md`.

W0031 PARTIAL: BMD v10/v12 byte parser + metadata bridge,56/56 test + WASM; ba model gốc qua NodeWASM/native. Chưa geometry JS/texture/render/GUI/browser. Xem `web/docs/BMD-MODEL.vi.md`.

W0030 PARTIAL: stream asset loader size/hash/abort/deadline/budget,54/54 npm tests. Chưa HTTP/browser real asset/model decoder/render/GUI game. Xem `web/docs/RESOURCE-LOADER.vi.md`.

W0029 PARTIAL: exact-case/hash manifest285 models,2/2 Python +50/50 npm tests. Chưa asset fetch/decoder/render; web chỉ canvas Debug, CHƯA GUI game. Xem `web/docs/RESOURCE-MANIFEST.vi.md`.

W0028 PARTIAL: portable native item path mapping,48/48 test + WASM; Data285 paths/11 case mismatches. Chưa manifest/body armor/render/UI hoặc MU network. Xem `web/docs/ITEM-RESOURCES.vi.md`.

W0027 PARTIAL: audit custom text/catalog-resource rules; 3/3 audit tests, ba catalog active0. Chưa builtin mapping/compiled catalog/render/UI. Xem `web/docs/CUSTOM-CATALOG.vi.md`.

W0026 PARTIAL: wing/pet decoder và custom ambiguity so source packing, 46/46 test + WASM. Chưa catalog/resource/render/UI hoặc MU network. Xem `web/docs/COMPANIONS.vi.md`.

W0025 PARTIAL: class/tier và bảy slot appearance decoder so source packer, 44/44 test + WASM. Chưa wing/pet/custom, model mapping/render/UI hoặc MU network. Xem `web/docs/APPEARANCE.vi.md`.

W0024 PARTIAL: lobby chờ list+policy sau login có deadline/cancel, 42/42 test + WASM. Chưa charset/render/UI hoặc MU network/provisioning. Xem `web/docs/LOBBY-SESSION.vi.md`.

W0023 PARTIAL: list/policy character codec qua ABI source native/WASM, 38/38 test; chưa lobby session/UI/charset rendering/MU network. Xem `web/docs/CHARACTER-CODEC.vi.md`.

W0022 PARTIAL: account session có timeout/cancel/chặn gửi trùng, 36/36 test + WASM. Chưa UI/MU network, provider/provisioning thật hoặc danh sách nhân vật. Xem `web/docs/ACCOUNT-SESSION.vi.md`.

W0021 PARTIAL: login ABI/credential codec core-WASM qua source-reference, 32/32 test + WASM. Chưa login state machine/UI/network MU hoặc provider/provisioning thật. Xem `web/docs/ACCOUNT-CODEC.vi.md`.

W0020 PARTIAL: greeting → identity-sent có phase deadline/abort, 30/30 test + WASM; chưa provider/provisioning thật, login hoặc MU network. Không coi identity-sent là verified. Xem `web/docs/GAME-HANDSHAKE.vi.md`.

W0019 PARTIAL: pipeline packet GameServer ghép WASM crypto/transport/framing; 25/25 test + WASM. Chưa phase login/identity provider/provisioning/network MU. Xem `web/docs/GAME-CHANNEL.vi.md`.

W0018 PARTIAL: SimpleModulus/key-file/serial core và adapter WASM qua source-reference; 21/21 test + WASM. Chưa key provisioning, GameServer session crypto runtime, identity provider/login. Xem `web/docs/SIMPLE-MODULUS.vi.md`.

W0017 PARTIAL: identity wire/XOR C1-C2 so Main/server source trên native/WASM; provider browser còn thiếu, không fallback HWID giả. Chưa C3-C4/serial/login. Xem `web/docs/CLIENT-IDENTITY.vi.md`.

W0015 PARTIAL: MuOnline transport derivation so source InitHackCheck12 cấu hình, parser EXTRA1 greeting có native/WASM fixture; chưa provisioning/game session/ClientVerify/login. Xem `web/docs/GAME-PROFILE.vi.md`.

W0013 PARTIAL: ConnectServer codec + transport byte transform/MHP có native/WASM fixture và reference từ hàm server; chưa SimpleModulus/key derivation/ClientVerify/login. Xem `web/docs/CONNECT-CODEC.vi.md`.

W0011 PARTIAL: session transport adapter + WASM checked-result reader qua socket/clock mô phỏng. Chưa cipher/identity/state hoặc network thật; xem `web/docs/TRANSPORT.vi.md`.

W0009 PARTIAL: toolchain dự án đã build core/bridge WASM; cùng C++ contract qua native và WASM trong Node. Browser/crypto/state còn mở; xem `web/docs/WASM-TOOLCHAIN.vi.md`.

W0008 PARTIAL: đã có core framing C++17 standalone và fixture tổng hợp; chưa WASM, crypto/state hoặc transport adapter. Xem `web/docs/CORE-FRAMING.vi.md`. Các checkbox tổng thể dưới đây còn mở.

- [ ] Tách codec/crypto/state khỏi Socket POSIX, CommonCrypto, UUID/HID, timer, file I/O và UI callbacks trong client.cpp.
- [ ] Đánh giá tái sử dụng viewport.cpp, inventory.cpp, item_stats.cpp, item_placement.cpp, motion.hpp và các parser Data.
- [ ] Tạo transport bất đồng bộ: feed bytes -> decode/events -> send bytes. Không giữ các vòng await/receive blocking của native trên main thread browser.
- [ ] Dùng fixture packet thật đã loại credential, đối chiếu output C++ native và WASM; kiểm tra packing/endian tường minh.

Đạt khi: cùng fixture cho cùng state/bytes ở native và WASM; không còn import Cocoa/SceneKit trong core.

## M03 — Gateway WSS/TCP

W0016 PARTIAL: probe greeting mã hóa qua fixture GameServer WS/TCP, frontend20/20 và gateway12/12; chưa MU/browser network, ClientVerify/authentication/login. Xem `web/docs/GAME-PROBE.vi.md`.

W0014 PARTIAL: discovery/list/chọn endpoint qua gateway + CS fixture thực; frontend/core16/16, gateway9/9. Chưa UI/runtime MU thật hoặc game/login. Xem `web/docs/DISCOVERY.vi.md`.

W0012 PARTIAL: gateway loopback WS/TCP + fixture network 7/7 qua; chưa discovery/login MU hoặc WSS deployment. Xem `web/docs/GATEWAY.vi.md`. Checkbox toàn bộ yêu cầu dưới đây còn mở.

- [ ] Endpoint Connect cố định; endpoint game chọn theo server code trong allowlist cấu hình, không nhận IP/port tùy ý từ browser.
- [ ] Mỗi kết nối browser có upstream và crypto/serial state riêng; không gộp nhiều account vào một TCP stream.
- [ ] Chuyển byte nhị phân đúng thứ tự; không giả định một WebSocket message bằng một MU packet.
- [ ] Test packet bị cắt, ghép nhiều packet, giới hạn kích thước/hàng đợi, client chậm, timeout, heartbeat và đóng hai đầu.
- [ ] Dùng HTTPS/WSS khi triển khai ngoài local; kiểm tra Origin, giới hạn kết nối/tốc độ và không log password.
- [ ] Kiểm tra địa chỉ trả về từ ConnectServer được client ánh xạ sang route gateway, không cố kết nối TCP từ browser.

Đạt khi: discovery/login/list nhân vật qua gateway hoạt động; không cần render 3D và không bypass HackCheck.

## M04 — Pipeline Data và cache

- [ ] Xuất terrain, thuộc tính ô, object placements, mesh, skeleton, clip, texture và bảng ngôn ngữ từ Data thật.
- [ ] Dùng định dạng trung gian có schema/version và giữ ID/path gốc; không chỉnh asset thủ công để che lỗi decoder.
- [ ] So sánh tọa độ, bone, UV, alpha, số frame với parser native; log rõ file lỗi hoặc chưa hỗ trợ.
- [ ] Manifest có hash, byte size và dependency; tải theo cảnh, không tải toàn bộ Data ngay lần mở đầu.
- [ ] Cache theo hash, chống trộn phiên bản và xóa cache hỏng; tiến trình tải theo byte thực, parse/GPU preparation là trạng thái riêng.
- [ ] Xác minh font gốc thực sự có thể dùng trên web và tài nguyên audio có thật; file thiếu ghi rõ, không tự thay font/nhạc.

Đạt khi: xóa cache/tải lại, mất mạng, asset hỏng hoặc update một file đều cho kết quả rõ ràng và tái lập được.

## M05 — Bản thử đồ họa và hiệu năng

- [ ] Render Lorencia với camera, heightmap, texture layers, alpha-cutout và object transform đúng dữ liệu.
- [ ] Render một DW đứng/đi/chạy; nội suy skeleton theo thời gian, không theo số khung hình.
- [ ] Kiểm tra đổi kích thước, DPR, tab ẩn/hiện và WebGL context lost/restored.
- [ ] Đo CPU/GPU frame time, draw calls, tải dữ liệu, bộ nhớ và hitch lúc tạo shader/texture.

Đạt khi: cảnh cố định và tuyến chạy cố định đạt ngân sách trên máy/browser đã chọn. Mục tiêu 60 FPS = 16,67 ms/frame; ghi p50/p95/p99 và frame >33 ms, không chỉ nhìn FPS trung bình.

Đây là cổng quyết định khả thi. Nếu không đạt, xử lý batch/culling/texture/GPU skinning trước khi mở rộng game.

## M06 — Luồng mở game đến vào map

- [ ] Loading Data thực -> chọn server -> đăng nhập -> chọn nhân vật -> vào game.
- [ ] Server chính đứng cố định; click bật/tắt sub-server bên phải; background, cursor và UI lấy từ Data đã xác minh.
- [ ] Tạo/xóa nhân vật theo packet và điều kiện server, thông báo lỗi đúng; không coi UI đổi là DB đã đổi.
- [ ] Escape/menu/đổi nhân vật/đổi server/logout, countdown và reconnect có state machine riêng.

Đạt khi: thực hiện cả luồng thành công và thất bại nhiều lần, không trùng session, không mất input hay nhảy sai màn hình.

## M07 — Di chuyển và đồng bộ thế giới

- [ ] Pathfinding theo terrain attributes, hướng và tốc độ/tick source; tách simulation khỏi render.
- [ ] Theo dõi authoritative position, đường đang hiển thị, ACK/correction và hủy input cũ.
- [ ] NPC, quái và player khác spawn/move/action/death/despawn đúng vòng đời.
- [ ] Kiểm tra RTT0/50/150/300 ms, jitter, mất kết nối và tab bị throttle; tránh dồn packet hoặc chạy bù khi quay lại tab.

Đạt khi: tuyến đi/va chạm không sai ô; số packet và độ trễ không tăng vô hạn; không giật camera khi mở UI.

## M08 — HUD, trang bị, inventory và kho

- [ ] HUD/font/language/cursor theo Data; định nghĩa một hệ tọa độ UI và scale chung.
- [ ] Inventory và Warehouse cùng mở; click nhấc/click đặt, footprint, equipment slot và ACK server.
- [ ] Model mặc/cầm, bone gắn, safe/combat stance, +level và hiệu ứng cập nhật cả lobby lẫn trong game.
- [ ] Tooltip tên/chỉ số/yêu cầu/màu đúng nguồn; số và chữ không overlap khi thay resolution/DPR.
- [ ] Shop mua/bán/sửa, drop/pick/Space, tên ground item và latch key; màu theo metadata server.

Đạt khi: không mất/nhân đôi đồ khi reject/timeout/disconnect; DB, inventory và model sau đăng nhập lại khớp nhau.

## M09 — Combat, skill, damage, loot và EXP

- [ ] Đánh thường, sau đó mỗi skill thành một task độc lập: điều kiện học, MP, tầm, input, packet, action, projectile/particle, sound và hit/death.
- [ ] Bắt đầu Energy Ball/Fire Ball, rồi Evil Spirit; test không có mục tiêu, quái di chuyển, ngoài tầm, safezone, miss và không đủ MP.
- [ ] Render damage/EXP/level-up từ sự kiện server, không từ thời điểm hiệu ứng vừa chạm bằng mắt.
- [ ] Quái đánh có animation/sound; damage vào chính player không bị bỏ qua.
- [ ] Drop theo cấu hình server, không thêm RNG tạo loot phía browser; test ownership, timeout nhặt và túi đầy.

Đạt khi: replay packet và test online cho đúng mục tiêu/HP/MP/EXP/item; không coi một lần có damage là toàn bộ skill đã đúng.

## M10 — Âm thanh và điều khiển trình duyệt

- [ ] Lập SOUND-MAP nguồn file/hàm/sự kiện, volume, loop, cooldown và vùng nghe; không gán tiếng la vào mọi hit.
- [ ] AudioContext được mở sau thao tác người dùng; xử lý mute, tab ẩn/hiện và chuyển cảnh không chồng nhạc.
- [ ] Kiểm tra click phải/trackpad/Control, context menu, Option+Space, phím nhặt, focus và ngôn ngữ bàn phím trên Safari/Chrome.
- [ ] Giữ thao tác gốc khi browser cho phép; hotkey bị hệ điều hành chiếm phải có lựa chọn thay thế được ghi rõ.

Đạt khi: nghe/nhìn test trực tiếp trên browser và bàn phím thật, không chỉ test biến trạng thái.

## M11 — Mở rộng class/map theo ma trận

- [ ] Mỗi class: tạo/chọn/vào map/đi/chạy/mặc đồ/vũ khí/skill/chết/logout.
- [ ] Mỗi map: terrain, vật cản, alpha, object/NPC animation, ambience và cổng chuyển map.
- [ ] Ưu tiên Noria/Elf và Elbeland/Summoner sau MVP Lorencia/DW; lưu regression các lỗi từng gặp.
- [ ] Special/event/custom item và skill chưa hỗ trợ phải nằm trong backlog rõ ràng.

Đạt khi: ma trận ghi trạng thái và bằng chứng, không đánh dấu cả class/map chỉ vì tải được model đứng.

## M12 — Đưa lên host và nghiệm thu

- [ ] Tách staging khỏi dữ liệu thật; gateway/service chạy có log, health check, giới hạn và quy trình stop/restart.
- [ ] Deploy HTTPS/WSS; host asset theo version/hash, publish manifest sau asset và có rollback.
- [ ] Test phiên dài, nhiều phiên theo mức tải đã chốt, không leak memory/connection; kiểm tra database sau crash/disconnect.
- [ ] Chạy lại performance matrix và đối chiếu hình ảnh/video trên đúng nguồn tham chiếu đã ghi.

Đạt khi: có bản staging tái lập được, báo cáo phần chưa giống/chưa hỗ trợ và người dùng test các luồng chính.

## Thứ tự triển khai và định nghĩa xong

1. M00–M05: chứng minh protocol/Data/render chạy được và có ngân sách hiệu năng.
2. M06–M07: một DW đăng nhập và đi trong Lorencia online là mốc đầu có thể test.
3. M08–M10: một vòng chơi hoàn chỉnh inventory -> equip -> đánh -> loot -> lưu.
4. M11–M12: mở rộng, ổn định và triển khai.

Mỗi ticket chỉ Done khi có nguồn, implementation, test tự động phù hợp, test thực tế và ghi rõ sai khác. Tính năng chưa đo hoặc chỉ có mock không được đánh dấu Done. Không chốt lịch hoàn thiện toàn bộ trước khi M05 có số đo; ước lượng theo ticket sau bản thử.

Tài liệu kỹ thuật: https://emscripten.org/docs/porting/networking.html ; https://threejs.org/docs/ ; https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/WebGL_best_practices ; https://developer.mozilla.org/en-US/docs/Web/Media/Guides/Autoplay

## W0050 — 2026-10-25: preview một nhân vật

- DONE: bỏ hai mẫu Sword01/Wing01 hiển thị độc lập; không tải/decode/render chúng khi mở preview. Nhân vật được render ở một viewport duy nhất, không còn chia ba cột. Chọn class/skill qua menu hiện có.
- 92/92 npm tests và build Debug/Release qua; staging lại asset Debug.
- PARTIAL: menu vẫn ghi rõ skill chưa có effect; thay đổi này không bổ sung effect riêng hay xác nhận parity.

- W0050 bổ sung: bỏ góc quay phụ thuộc thời gian tuyệt đối của browser; khóa góc nhìn mặc định π + 0,4 rad khi reload/đổi class/skill, animation nhân vật vẫn chạy.

## W0051 — 2026-10-25: giảm cấp phát render preview

- DONE: Fire01 transform nhận output tái sử dụng, giữ cùng position/normal typed arrays qua các frame; topology thay đổi bị từ chối. Hai mesh không còn tạo thêm 4 typed arrays trong transform mỗi frame. WASM sampler vẫn cấp phát output, chưa phải render không cấp phát hoàn toàn.
- Bộ đo CPU/interval dùng ring buffer cố định 240/3600 mẫu, ghi O(1), không shift mảng. Percentile chỉ sao chép/sort khi cập nhật overlay mỗi giây. Clear CPU samples khi đổi visibility giữ hành vi cũ.
- 94/94 npm tests qua, Debug/Release build qua, staging lại 175 asset. Test so buffer tái sử dụng với kết quả không pooling tại mốc active/inactive/wrap, kiểm identity, source không đổi và topology mismatch. Test ring wrap/reset/percentile numeric và dữ liệu lỗi.
- PARTIAL: chưa pooling output WASM/body sampler, GPU skinning, profile DL spike hoặc bảo đảm 60/120 FPS. Các skill ngoài Fire Ball vẫn chưa có effect; M02/M03 chưa hoàn tất. Bước sau tiếp tục chuyển effect có nguồn đầy đủ và đối chiếu visual gốc.
- Không sửa Data/native production/config hoặc bật MU server/DB.

## W0052 — 2026-10-25: Energy Ball preview

- DONE: skill17 có prototype riêng chọn trong menu, không dùng Fire01 thay thế. Sprite gốc Effect/Thunder01.OZJ, additive/no depth write, billboard theo góc camera cố định. Kích thước0,6world unit, cao1unit, tốc độ15unit/s theo native/client/world_view.mm nhánh skill17. Đích giả3unit, lifetime0,2s, lặp2s chỉ là lịch kiểm thử. Body dùng cast146/147 adapter hiện có.
- Staging thêm đúng1texture, tổng176asset, hash/exact-case giữ nguyên. Manifest Python/JS cho phép filename an toàn trong Effect, vẫn chặn traversal/nested path. Release không stage asset này.
- 96/96 npm tests và build Debug/Release qua; 2 Python resource manifest tests qua. Kiểm numeric kích thước/height/speed/billboard/lifetime/buffer identity; menu kiểm classfilter và chọn energy thay vìfire.
- Browser DW tier2 chạy Energy Ball và bật freeze0,12s; không thấy lỗi render trong lượt kiểm.
- PARTIAL: nguồn đang chuyển là adapter native hiện có, chưa xác nhận visual parity Windows100%; genericcast, chưa sound/target thật/hit/damage/mana/cooldown/server. Các skill khác và đầy đủ7class vẫn TODO. M02/M03 chưa hoàn tất, chưa đảm bảo60/120FPS.
- Không bậtMUserver/DB hoặc sửaData/native production/config.

## W0053 — 2026-10-25: kiểm bao phủ toàn bộ danh sách skill

DONE: registry hỗ trợ và audit21class/tier, 97/97tests qua; chi tiết web/docs/SKILL-COVERAGE.vi.md. PARTIAL: toàn bộ effect7class chưa ghép; báo cáo phân biệt rõ prototype/missing, không coi danh sách config là skill hoàn chỉnh. Thiếu các nhánh source Windows gốc, đang yêu cầu đường dẫn nếu có.

## W0054 — 2026-10-25: Evil Spirit prototype

DONE: thuật toán25Hz đối chiếu4200scalarC++/JS, head/ribbon vàMultiply vật liệu lên preview;100/100testsqua,179asset. Chi tiết web/docs/EVIL-SPIRIT.vi.md. PARTIAL: headorientation/UV/scale/độ tối chưaWindowsparity, genericcast/nohit/server; chưa full7class và chưa đảm bảo60/120FPS.

## W0055 — 2026-10-25: chuyển sang tích hợp server thật

DONE: browser->gateway->ConnectServer-native thực, nhận list0 khiGSoffline; xử lý packetnamesF3:EA vừa phát hiện từtrace;102/102testsqua. PARTIAL: chưaGameServer/login/character/world và chưasceneGUIgốc. Chi tiết web/docs/LIVE-SERVER.vi.md. Đang chạyCS/gateway/web trênloopback, DB/GSchưabật.

## W0056 — GameServer thật (2026-10-25)

Browser đã nhận server0 và greeting0 qua gateway;102/102testsqua. BackupDB trước startup. MySQL/CS/Join/Data/Game/gateway/web đang chạy local. Chưa identity/login/character/world. Chi tiết web/docs/LIVE-GREETING.vi.md.

## W0057 — Identity và login thật (2026-10-25)

DONE: helper macOS origin-bound local9001, phiên identity và form login; browser sau thao tác người dùng đã nhận4nhânvật thật.102/102clienttests và14/14gatewaytestsqua. Chi tiết web/docs/LIVE-IDENTITY.vi.md. PARTIAL: chưascene3D roster/enterworld/map, chưaparityWindows; helper chỉlocal và chưaproductiontrust. Dịch vụ local vẫn chạy.


## W0058 — 2026-10-26: HUD dữ liệu vào map và vòng đời phiên

- Bổ sung HUD Debug hiển thị tên/cấp từ roster, map/tọa độ/HP 32-bit/Zen từ F3:03 đã decode. Dùng textContent, ẩn lúc khởi tạo và xóa khi đổi server, lỗi hoặc đóng phiên; dispose gỡ DOM.
- Đây là snapshot lúc vào map, nhãn hiển thị rõ chưa cập nhật trong trận; chưa có cập nhật HP/Zen, MP/SD/AG, HUD asset gốc hoặc combat. Các module roster/terrain/world-entry đã có trong workspace tiếp tục được sử dụng; không coi đây là xác nhận online/visual cho chúng.
- Sửa heartbeat để timer handle 0 vẫn chỉ đăng ký một interval; lấy lại danh sách server đóng cả world session.
- Kiểm thử: 109/109 Node tests qua, gồm HUD giá trị 32-bit/reset/dispose và timer handle 0; Debug/Release build qua (52/31 file), stage lại 179 asset và profile Debug local.
- Chưa kiểm thử browser/live login trong đợt này; chưa hoàn tất M05/M06. Không sửa Data/config gốc hoặc khởi động server/DB.


## W0059 — 2026-10-26: audit di chuyển và source bổ sung

Đã đối chiếu luồng input/path/packet/ACK/render: web chưa triển khai di chuyển; native có riêng, không dùng làm bằng chứng web. Inventory39file/3.648.890byte trong `file thieu`, hash/include/line anchors đã lưu; có nguồn Fire/Energy/Spirit cần review tiếp, versionEX603 chưa xác nhận. 109/109webtests qua; nativecontract/render pass, cachetest cần chạy ngoài sandbox mới pass. Chưa có click/di chuyển browser live thành công, chưa port effect mới. Chi tiết: web/docs/MOVEMENT-SOURCE-AUDIT.vi.md (tính từ Emulator). M07 vẫnTODO.


## W0060 — 2026-10-26: đi bộ từng ô trên web (PARTIAL M07)

Đã nối keyboard→D4→own-indexACK/correction→idle/walk và camera follow; chặn wall/corner/biên và giới hạn12ô quanh entry. Không click/A*/run/streaming hoặc fullsourceparity. 116/116Node tests pass; Debug/Release54/31file, stage179asset/profile. Livecheck JS→gateway→GameServer chưa pass do accounttest trả logincode3; chưabrowservisual/savedposition. Không sửaDB/config hoặc ngắt phiên đang đăng nhập. Chi tiết `web/docs/STEP-MOVEMENT.vi.md` từ Emulator; không đóngM07.


## W0061 — 2026-10-26: kiểm thử browser trên phiên người dùng mở sẵn

- Người dùng đã vào sẵn nhân vật DKTEST, map0. Kiểm thử trực tiếp tab `http://127.0.0.1:8080/?scene=servers`, không reload/logout hoặc đăng nhập thêm account.
- Bốn phím Right/Down/Left/Up lần lượt được UI báo **Server xác nhận**: (134,119) → (135,119) → (135,120) → (134,120) → (134,119). HUD tọa độ khớp mỗi ACK; kết thúc đúng ô ban đầu, phiên GameServer vẫn mở.
- Quan sát hình: nhân vật đổi hướng/pose đi bộ, mặt đất dịch khi camera theo; không thấy lỗi render/đóng phiên trong vòng thử ngắn. Overlay59.9–60.0FPS ở buffer1416×1798,DPR2. Đây là quan sát ngắn, không đo CPU/GPU percentile hoặc cam kết60FPS ổn định.
- Ctrl+Right không đổi tọa độ (134,119), đúng điều kiện bỏ modifier. Không thử wall/giới hạn12ô/tabẩn/disconnect/correction trên phiên này; các trường hợp đó vẫn chỉ có unit coverage.
- Ảnh bằng chứng cuối: `/tmp/mu-w61-movement.png`. Không sửa code gameplay hoặc rebuild trong lượt test; giữ nguyên phiên để người dùng tiếp tục.
- Phát hiện UI: HUD entry ở giữa phía trên chồng lên Debug overlay bên trái tại viewport khoảng708×899CSSpx. Chưa sửa trong lượt này vì rebuild/reload sẽ ảnh hưởng phiên đang dùng. Cần tách vị trí/responsive overlay.
- Chưa kiểm save sau logout/relogin, tốc độ/parity gốc, đi liên tục/chéo, clickA*, streaming, đủ7class hoặc chạy nhanh. M07 vẫnPARTIAL. Kết quả browser này bổ sung cho W0060, không biến lượt Node livecheck bị logincode3 thành pass.


## W0062 — 2026-10-26: menu M và audit47World (PARTIAL)

Đã thêm M/menu39moveđíchserver +47offlineWorldlink, loaderraw8/extended24theosource, online8E→1CsessionvàhelperallowlistWorld.47terrain giải mã,38texture/CPUpatchpass;browser38vòngrenderchạy,9mapbáolỗimapping.121clienttests/16gatewaytestsqua;Debug58/Release31. Chưaobject/NPC/environment/fullmapalias/onlinewarpverified hoặc100%parity. Chi tiết `web/docs/MAP-MENU.vi.md`, catalog và2báocáoJSON; giữ nguyênserver/DB/phiênonline.


## W0063 — checkpoint 2026-10-26, tạm dừng theo yêu cầu người dùng

Người dùng hẹn tiếp tục lúc13:30 giờ Việt Nam. Đợt resource/NPC **chưa hoàn tất**, không coi là full.

- Đã thêm resource catalog cho47World; scan thư viện104modelNpc và344modelMonster, cùng các Objectfolder. Kết quả3701/3819tham chiếu model qua kiểm tra;61NPCbinding,59rig qua,2rig còn texture lỗi.
- Đã thêm loader/render object placement/lightmap, generic multipart ActorAnimation trongWASM, NPC viewportspawn/update/destroy/death và layer render theo snapshot server. Offline có checkbox NPC cấu hình và chọn tọa độ NPC; không giả spawn online.
- Báo cáo mỗi map: `web/docs/world-resources-W0063.json`; catalog runtime: `web/client/src/world/resource-catalog.json`. CPUkiểm47patch có geometry hữu hạn;30classNPC trong spawnconfig còn thiếu binding/asset. Chưa full effect/animation/sound/talk/shop hoặc parityWindows.
- Browser: Lorencia146,110 hiện195object,3NPCfixture; Noria179,126 hiện482object và4NPCfixture, có CherryBlossom, giữ cảnh báo Object4/Object44.bmd vàNPC257. OnlineDKTEST đã thấy2→3NPCserver, không báo resourceerror ở khu vực quan sát; ảnh `/tmp/mu-w63-online-npc.png`, `/tmp/mu-w63-noria-resources.png`. Người dùng đang di chuyển; không tuyên bố agent tự chạy hết các tuyến.
- 128clienttests và16gatewaytests qua trước đợt chốt. Đang chạy lại fullclienttest trên bản copy tạm, tránh xóa outputDebug đang phục vụ: log `/tmp/mu-w63-isolated-tests-final.log`. Lượt stagingtest đầu thiếu siblingMain_EX603 (lỗi môi trường tạm), đã sửa layout cho lần chạy lại. Cần xác nhận log cuối trước chốt tài liệu.
- Debug64file/Release31 đã build, WASM đã rebuild;179previewasset và profileDebug đã stage. HelperPID lưu ở `native/runtime/background/Identity.pid`, đã restart riêng để hỗ trợ NPC/Object/subfolders. MUserver/gateway không restart.
- Testbuild trước đó đã xóa profileDebug tạm thời gây lỗi kết nối; đã stage lại và xác nhận mở lại form/login/roster/map thật trênbrowser. Tránh chạy buildtest trực tiếp vào dist đang phục vụ khi người dùng đang test.
- Việc tiếp: xác nhận test cuối; hoàn thiện docsW0063 và bảng resource/NPC từngmap; xử lý30classNPC thiếu (Player-based/event/delegated source),2rig thiếu texture và các mapping/model errors. Không ghi “full” khi còn thiếu.

W0063 chốt trước khi dừng: full client test trong bản copy tạm đã xác nhận **128/128 pass**, log `/tmp/mu-w63-isolated-tests-final.log`; output Debug đang chạy không bị thay đổi.


## W0064 — 2026-10-26: tiếp tục resource/NPC

Đã thêm helper dùng Player(action1/giápMale10), tách texture folder theo OpenTexture để sửa Quarrel, và adapter7sharedtexture có hash cho Raul cùng brightmesh4/14.63/63rig đã ánh xạ qua kiểm tra,3709/3825tham chiếu model qua;27classNPC cấu hình vẫn chưa đủ.130/130clienttests qua trong workspace tạm;Debug64/Release31 được chuẩn bị/stage trước khi publish, có backup output cũ. Browser đã kiểm helperDevias/Raul;Quarrel có occlusion và thiếuchrome được ghi. Chi tiết `web/docs/WORLD-RESOURCES.vi.md`, báo cáo `world-resources-W0064.json`. Không gọi là full resource/NPC hoặc100%parity.

W0066 (web): đã có giữ phím/click A* + route motion, safe/armed clips/attachment, foot run gate, marker gốc, normal attack4monster(2/3/11/12), damage/HP/death/reward/regen server. Online MuOptions click Spider nhận Hạ quái#61:78, HP65→64.143/143 clienttests +WASMcontracts pass. Full combat/skills/all monsters/mount/buff/corpse/fullHUD vẫn PARTIAL; xem web/docs/MOVEMENT-COMBAT-W0066.vi.md.
