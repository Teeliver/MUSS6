# Client MU EX603 WebGL — M00/M01/M02 đang triển khai

Đã có diagnostics WebGL2/WASM và hai build mode. Đã có gateway loopback kiểm thử bằng fixture; chưa có MU online, scene hoặc gameplay web.

- Kế hoạch gốc: [WEBGL-TODO](../native/client/WEBGL-TODO.vi.md).
- Mỗi đợt đã làm gì, test gì và còn gì: [STATUS](docs/STATUS.vi.md).
- Nguồn nào là EX603, reference hoặc implementation mới: [SOURCE-MAP](docs/SOURCE-MAP.vi.md).
- Hash/phạm vi snapshot, máy kiểm tra và cách tái lập: [BASELINE](docs/BASELINE.vi.md).
- Audit terrain/object/table: [WORLD-AUDIT](docs/WORLD-AUDIT.vi.md).
- Audit cấu trúc 47 world: [STRUCTURE-AUDIT](docs/STRUCTURE-AUDIT.vi.md).
- Điều tra ngoại lệ: [EXCEPTIONS-AUDIT](docs/EXCEPTIONS-AUDIT.vi.md).
- Audit mọi frame animation: [ANIMATION-AUDIT](docs/ANIMATION-AUDIT.vi.md).
- Core framing portable: [CORE-FRAMING](docs/CORE-FRAMING.vi.md).
- Build/test WASM: [WASM-TOOLCHAIN](docs/WASM-TOOLCHAIN.vi.md).
- Transport/session: [TRANSPORT](docs/TRANSPORT.vi.md).
- Gateway local: [GATEWAY](docs/GATEWAY.vi.md).
- ConnectServer/transport codec: [CONNECT-CODEC](docs/CONNECT-CODEC.vi.md).
- Discovery/session chọn server: [DISCOVERY](docs/DISCOVERY.vi.md).
- GameServer profile/greeting: [GAME-PROFILE](docs/GAME-PROFILE.vi.md).
- GameServer probe mã hóa: [GAME-PROBE](docs/GAME-PROBE.vi.md).
- ClientVerify/HWID: [CLIENT-IDENTITY](docs/CLIENT-IDENTITY.vi.md).
- SimpleModulus/key-file/serial: [SIMPLE-MODULUS](docs/SIMPLE-MODULUS.vi.md).
- Pipeline packet GameServer: [GAME-CHANNEL](docs/GAME-CHANNEL.vi.md).
- Greeting/ClientVerify phases: [GAME-HANDSHAKE](docs/GAME-HANDSHAKE.vi.md).
- Codec đăng nhập EX603: [ACCOUNT-CODEC](docs/ACCOUNT-CODEC.vi.md).
- Session đăng nhập: [ACCOUNT-SESSION](docs/ACCOUNT-SESSION.vi.md).
- Codec danh sách nhân vật: [CHARACTER-CODEC](docs/CHARACTER-CODEC.vi.md).
- Session danh sách nhân vật: [LOBBY-SESSION](docs/LOBBY-SESSION.vi.md).
- Appearance class/trang bị: [APPEARANCE](docs/APPEARANCE.vi.md).
- Appearance cánh/pet: [COMPANIONS](docs/COMPANIONS.vi.md).
- Audit catalog custom: [CUSTOM-CATALOG](docs/CUSTOM-CATALOG.vi.md).
- Item model path/Data audit: [ITEM-RESOURCES](docs/ITEM-RESOURCES.vi.md).
- Manifest resource: [RESOURCE-MANIFEST](docs/RESOURCE-MANIFEST.vi.md).
- Loader resource có kiểm hash: [RESOURCE-LOADER](docs/RESOURCE-LOADER.vi.md).
- BMD model đọc trực tiếp: [BMD-MODEL](docs/BMD-MODEL.vi.md).
- BMD geometry buffers: [BMD-GEOMETRY](docs/BMD-GEOMETRY.vi.md).
- Texture OZT/OZJ: [TEXTURES](docs/TEXTURES.vi.md).
- Browser textured model preview: [MODEL-PREVIEW](docs/MODEL-PREVIEW.vi.md).
- Player skeleton binding/material audit: [PLAYER-BINDING](docs/PLAYER-BINDING.vi.md).
- Fullbody/OZJ row fix: [BODY-PREVIEW](docs/BODY-PREVIEW.vi.md).
- Các lỗi/thiếu chưa được phép đánh dấu hoàn thiện: [KNOWN-GAPS](docs/KNOWN-GAPS.vi.md).

Chạy kiểm thử công cụ M00:

```sh
python3 -m unittest discover -s web/tests -v
```

Lệnh này chạy từ Emulator, không chạy server/DB. Baseline cục bộ ở `.local-baselines/` bị ignore; không đưa source archive, credential, cấu hình riêng hoặc Data gốc vào bundle web.

Mỗi mốc tiếp theo phải cập nhật STATUS, TODO và KNOWN-GAPS trong cùng đợt triển khai. Chỉ đóng mục có bằng chứng đủ cho phạm vi của nó.

## Bộ khung M01 — W0007

Cần Node **26.11.1**, npm **11.20.0**, Python 3. Không cần `npm install` ở đợt này vì chưa có dependency.

```sh
npm --prefix web run build:wasm
npm --prefix web run build:debug
npm --prefix web run build:release
npm --prefix web test
npm --prefix web run serve
```

Lệnh serve phục vụ diagnostics tại `http://127.0.0.1:8080`, dừng bằng Ctrl+C; không khởi động MU server. Trang chưa tải Data/login/render game. Output 4 file ở web/dist. Kiểm tra browser thực chưa thực hiện trong W0007.

[ADR stack](docs/ADR-001.vi.md) · [Quy ước dữ liệu](docs/CONVENTIONS.vi.md). M00 vẫn PARTIAL; gateway và WASM chưa triển khai.

## Quy ước hiện hành W0010

Xem [PROJECT-LAYOUT](docs/PROJECT-LAYOUT.vi.md) để biết cây thư mục, Debug/Release và lệnh hiện tại. Hướng dẫn output 4 file hoặc baseline làm dependency trong W0007–W0009 chỉ mô tả lịch sử; hiện dùng `.build/wasm` và `dist/debug`, `dist/release`.

- [W0037: thân cơ bản 7 class, audit Data và giới hạn](docs/BODY-CLASSES.vi.md). Preview Elf: `http://127.0.0.1:8080/debug/?preview=models&class=2`; Summoner dùng `class=5`; thêm `tier=1` hoặc `tier=2` để kiểm tra model cấp chuyển chức.

- [W0038: resolver trang bị class trong core/WASM](docs/ARMOR-RESOLVER.vi.md). Preview DW giáp: `http://127.0.0.1:8080/debug/?preview=models&class=0&armor=0`. Đây là fixture kỹ thuật; giới hạn skin/visibility ghi trong tài liệu.

- [W0039: trang bị độc lập từng slot, giới hạn skin và FPS](docs/MIXED-ARMOR.vi.md). Thử Summoner không đội mũ: `http://127.0.0.1:8080/debug/?preview=models&class=5&armor=511,10,10,10,10`.

- [W0040: bone và transform gắn trang bị portable](docs/EQUIPMENT-GRIP.vi.md). Hoàn thành policy/numeric reference; chưa ghép mesh vũ khí vào preview.

- [W0041: geometry vũ khí gắn Player pose](docs/ATTACHMENT-GEOMETRY.vi.md). Preview Sword01 tay phải thêm `weapon=0`; tay trái `weapon=1`; đeo lưng thêm `safe=1`. Hiện là fixture tĩnh, chưa stance/animation/equip online.

- [W0042: tư thế vũ khí từ Item.bmd và native action policy](docs/EQUIPMENT-ACTIONS.vi.md). `?preview=models&class=0&weapon=0` thử idle4; thêm `safe=1` giữ pose đứng cơ bản/đeo lưng. Chưa animation.

- [W0043: animation thân/vũ khí, tối ưu cập nhật và giới hạn số đo FPS](docs/BODY-ANIMATION.vi.md). Preview hiện chạy idle theo thời gian; overlay có CPU frame p95. Benchmark CPU: `node tools/benchmark_animation.mjs /đường/dẫn/Data`.

- [W0044: animation đi/chạy và blend tư thế](docs/MOTION-ANIMATION.vi.md). Thử `http://127.0.0.1:8080/debug/?preview=models&class=0&weapon=0&motion=cycle`; đây là demo animation tại chỗ, chưa di chuyển trên map.

- [W0045: cung Elf và staff/khiên DW](docs/WEAPON-LOADOUTS.vi.md). Fixture: `?preview=models&class=2&loadout=bow&motion=cycle` hoặc `?preview=models&class=0&loadout=staff-shield&motion=cycle`. Chưa chiến đấu online.

- [W0046: clip đánh theo vũ khí và chuyển tư thế](docs/ATTACK-ANIMATION.vi.md). Thử `?preview=models&class=2&loadout=bow&motion=attack` hoặc `class=0&loadout=staff-shield&motion=combat`. Chỉ fixture hình ảnh, chưa damage/projectile/server.

- [W0047: cast146/147 và audit nguồn effect](docs/CAST-ANIMATION.vi.md). Thử `?preview=models&class=0&loadout=staff-shield&motion=cast`. Chưa skill effect/damage/server. Audit nguồn: `node tools/skill_asset_audit.mjs /đường/dẫn/Data`.

- [W0048: Fire01 prototype, kiểm tra7class và ma trận skill/class server](docs/SEVEN-CLASS-EFFECTS.vi.md). `effect=fire&effect-time=0.12` là diagnostic render, không cấp skill; Fire Ball chỉ qua class filter củaDW/MG/Summoner. Full skill set7class vẫn chưa hoàn thiện.

- [W0049: menu chọn class và skill trong preview Debug](docs/PREVIEW-MENU.vi.md).

- [W0054: Evil Spirit preview và đối chiếu native](docs/EVIL-SPIRIT.vi.md).

- [W0060: đi bộ từng ô, ACK/correction và camera](docs/STEP-MOVEMENT.vi.md). Trang local: `http://127.0.0.1:8080/?scene=servers`; dùng phím mũi tên sau vào map, giới hạn12ô quanh entry. Chưa click tìm đường/run, livecheck chưapass.

- [W0062: menu M, catalog47World và kiểm tra map](docs/MAP-MENU.vi.md). Offline: `http://127.0.0.1:8080/?scene=terrain-check&map=0`.38terrain có thểnạp,9mappingtexturecònthiếu; chưafullscene/parity100%.

- [W0063/W0064: resource môi trường và NPC theo map](docs/WORLD-RESOURCES.vi.md). Có online viewport NPC và preview NPC cấu hình;63rig đã ánh xạ nạp được,27class NPC cấu hình còn thiếu.
