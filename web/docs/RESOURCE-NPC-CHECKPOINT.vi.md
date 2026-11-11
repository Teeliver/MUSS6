## W0114 — 2026-10-31

Đã port khói màu bộ trang bị theo source: texture cra_04, bone0, 25Hz, điều kiện bộ/class/cấp và màu riêng. Preview Dragon Knight +15 có khói, +9 tắt. Không phải mọi bộ +15 đều có khói. [Đối chiếu và giới hạn](EQUIPMENT-SMOKE-W0114.vi.md).

## W0113 — 2026-10-31

Đã thêm preview trang bị tùy chọn offline cho7class,12ô,+0..15,Excellent,mặc bộ,zoom,xoay và lưu URL.102test qua;DK Dragon/Kris/Storm,refresh và RF không găng đã kiểm tra trình duyệt. [Cách dùng](EQUIPMENT-PREVIEW-W0113.vi.md).

## W0112 — 2026-10-31

Đã xác định/sửa UV cánh TGA trong inventory, trả góc cũ gồm Y=-10, sửa matte additive gây nền tam giác đen. Rà18 cánh/áo choàng cấp1–3,30 mesh,9 nhóm ảnh trình duyệt;98 test qua. [Chi tiết và ảnh](WING-ICONS-W0112.vi.md). W0108 suy luận lỗi chỉ do góc đã được thay thế bằng phát hiện UV.

## W0108 — 2026-10-31

Đã sửa góc icon cánh để giữ mặt nhìn, kiểm tra cùng cánh trong ô trang bị/túi và hover trên trình duyệt. 97 test qua. Thử thu nhỏ nón bằng fallback W0107 không đạt nên đã rút lại; tỷ lệ native của nón vẫn chưa xác nhận. Xem [ITEM-ICON-POSE-W0108.vi.md](ITEM-ICON-POSE-W0108.vi.md).

## W0106 — 2026-10-31

Đã thêm nhãn/tooltip option set từ Data checksum, sửa luồng thả đồ theo source. 97 test mục tiêu và tooltip Warrior đã kiểm tra. Cánh Heaven trong túi và tỷ lệ Violent Wind Helm **chưa chốt**, không báo hoàn tất hai icon này. Xem [ITEM-SET-DROP-W0106.vi.md](ITEM-SET-DROP-W0106.vi.md).

## W0105 — 2026-10-31

Đã sửa icon cánh Heaven ngược, Apple auto-fit quá lớn và trang sức lệch tâm; 81 test qua, đã kiểm tra trình duyệt offline. Chi tiết: [ITEM-ICONS-W0105.vi.md](ITEM-ICONS-W0105.vi.md). Bảng ItemTRSData vẫn thiếu nên chưa xác nhận tỷ lệ tuyệt đối.

> W0104: [Tooltip/class/Orb/Life](ITEM-TOOLTIPS-CLASS-W0104.vi.md): đọc ba bảng tooltip Data gốc, sửa Energy/Vitality, class chữ trắng nền đỏ có khoảng cách và đứng trước options; Life giữa ô, số lượng đậm nhẹ. Orb giữ Gem*.bmd, sửa phần phóng theo panel; ngọc giữ kích thước W0103 theo phản hồi. Full 290 test qua; 80 test liên quan sau giới hạn chỉ Orb qua. Còn các status tooltip phụ thuộc hệ thống chưa port và thiếu bảng custom ItemTRSData.

> W0102: [Item inventory](ITEM-INVENTORY-W0102.vi.md): sửa tỷ lệ Orb, dùng bốn model inventory RF và mesh HideSkin theo source, thêm số lượng bình/ngọc nén bằng atlas gốc. Audit 924 item: 902 item/903 variant ready, 22 chưa ánh xạ. 283 test full qua; 59 test liên quan sau chỉnh mặt trước RF qua. Thiếu ItemTRSData.bmd nên chưa chứng nhận tỷ lệ/góc toàn bộ item 100%.

> W0101: [MU Helper](MU-HELPER-W0101.vi.md): hai nút mở cấu hình và bật/dừng theo ACK GameServer; tự đánh, potion HP, nhặt lọc, quay lại điểm và repair cơ bản. 280 test client qua. Party/buff/combo/lệnh pet/offline train và popup skill native chưa hoàn tất; không công bố parity 100%.

> W0100: [Thanh tên map/tọa độ](HERO-POSITION-W0100.vi.md): đúng CNewUIHeroPositionInfo và resource Minimap_position/MacroUI; tên từ Text gốc, cập nhật theo nhân vật. Hai nút là cấu hình và Start/Stop MU Helper; người dùng chọn để hệ thống Helper ở bước sau.39test liên quan qua.

> W0099: [Sửa HP lệch và Unicorn](PET-HP-UNICORN-W0099.vi.md): inset/crop atlas HP đúng NewUICommon; Unicorn dùng đồng hồ riêng, controller25Hz và nội suy pose/frame.37test liên quan qua; chưa chứng nhận mọi tình huống trên phiên thật.

> W0098: [Máu pet và thú cưỡi](PET-MOUNTS-W0098.vi.md): HP helper từ độ bền/255, sửa follow/UV và cập nhật animation mỗi frame; thêm Uniria, Dinorant, Dark Horse, bốn màu Fenrir từ Skill. Audit7model/210mẫu,43cặp class hợp lệ;269test hồi quy qua. Skill/particle/controller đặc thù chưa đầy đủ.

> W0097: [Áo choàng DL/RF](CAPES-DL-RF-W0097.vi.md): sửa bốn cape bằng texture Data gốc và physics cloth theo source, bone19, va chạm bone2/17, tỷ lệ DL .92/RF 1.03. 265/265 test qua; xem main/remote và chạy offline riêng từng cape. Chưa chứng nhận pixel parity hoặc toàn bộ lớp cloth phụ.

> W0093: [Item, cánh, pet và remote player](ITEM-WING-PET-PLAYERS-W0093.vi.md); audit924 item, sửa wing Link=false, nối companion bits. Pet cưỡi/controller đặc thù và tỷ lệ ItemTRSData còn thiếu.

> W0091: [Mái Devias và animation/render quái](ROOFS-MONSTER-RENDER-W0091.vi.md); 252 test qua, audit207 model. Particle gắn bone/boss đặc thù còn thiếu, chưa parity100%.

> W0090: [Quái, animation, tên/thanh máu](MONSTER-COMBAT-W0090.vi.md): binding 311/313 loại spawn; combat thật đang kiểm tra, hai bẫy 106/523 và hiệu ứng đặc thù còn thiếu. Đã phục hồi server sau reboot 2026-10-29.

> W0089: [Ngồi ghế, dựa tường và con trỏ](OPERATE-CHAIR-LEAN-W0089.vi.md); kiểm kê 429 điểm. Castle còn phụ thuộc trạng thái công thành.

> W0088: xem [kiểm kê map và hiệu ứng](MAP-RESOURCE-EFFECT-W0088.vi.md). Đang thực hiện; chưa đạt đầy đủ hiệu ứng gốc.

> W0087: Move thêm C1:F3:12 FinishLoading sau nạp cảnh để server gỡ RegenOk, hủy route cũ/giữ socket khi ACK trễ; cánh clip0 animation và DW fly actions11/34. 236 test qua, user xác nhận đã hết lỗi trước khi yêu cầu audit resource/effect toàn map. Chi tiết [MOVE-FINISH-LOADING-W0087.vi.md](MOVE-FINISH-LOADING-W0087.vi.md).

> W0085: GUI Move dựa NewUIMoveCommandWindow, MuOptions Lv300 + Wings of Heaven cấp1; gỡ phím S thử nghiệm vì chưa có đường tương ứng trong source và người dùng yêu cầu bỏ qua. Chi tiết [MOVE-GUI-TEST-CHARACTER-W0085.vi.md](MOVE-GUI-TEST-CHARACTER-W0085.vi.md).

> W0084: lưu skill/potion hotkey theo nhân vật bằng C1:F3:30/OptionData; nhấc/bỏ/chuyển liên kết bình HUD; sửa số SD và kiểu chữ số skill. 233 test qua, save thật đã thấy trong DB. Chi tiết [HOTKEY-PERSISTENCE-W0084.vi.md](HOTKEY-PERSISTENCE-W0084.vi.md).

> W0082–W0083: HUD skill 1–5, chọn skill bên phải, bình Q/W/E/R và chuột phải; sửa multi skill ENG C3:DB gây disconnect khi gửi nhầm C3:1D. 231 test qua; quan sát Evil Spirit trên phiên thật vẫn kết nối, mana/EXP cập nhật. Giới hạn và bằng chứng tại [SKILLS-HUD-W0082.vi.md](SKILLS-HUD-W0082.vi.md).

> W0081: Sound911 WAV + Music47 file đã vào catalog/hash và engine web; nối các luồng đang có, bước chân theo frame thật, decoder ADPCM, mute/visibility/scene cleanup. Chi tiết coverage và phần chưa port tại [AUDIO-W0081.vi.md](AUDIO-W0081.vi.md), call-site tại AUDIO-SOURCE-AUDIT.json.

> W0080: bổ sung model người chơi khác từ viewport CharSet; nhận cập nhật/tháo trang bị F3:13/C1:25, nội suy chuyển động, bóng và tên theo pose. Chi tiết [REMOTE-PLAYERS-W0080.vi.md](REMOTE-PLAYERS-W0080.vi.md).

> W0079: Welcome về log hệ thống; notice sự kiện/thương hiệu tại Y62.5%; bảng tên map bằng32 resource gốc/66 mapID, fade và giữ5giây theo UIMapName. Chi tiết [MAP-TITLES-NOTICES-W0079.vi.md](MAP-TITLES-NOTICES-W0079.vi.md).

> W0078: sửa Option/Alt+Space khi UI giữ focus; tên nhân vật/NPC hover, hậu tố đồ rơi theo source; tách sự kiện giữa màn hình và log hệ thống/nhặt đồ, đổi thương hiệu hiển thị. 211 test qua; kiểm tra browser offline, chưa phiên nhân vật thật. Chi tiết [NAMES-NOTICES-W0078.vi.md](NAMES-NOTICES-W0078.vi.md).

> W0077: sửa delay actor/đồ rơi bằng prepared resources trong RAM, xóa đồ theo ACK nhặt thành công và native item viewport flush sau thao tác. Bản client và GameServer đã nạp lại. Chi tiết đo và giới hạn tại [ITEM-LATENCY-W0077.vi.md](ITEM-LATENCY-W0077.vi.md).

> W0076: đồ rơi/nhặt/thả, CursorGet, Alt/Option+Space giữ tên, Space nhặt; thay actor trang bị riêng, giữ cảnh map. Đã đưa lên8080 và kiểm tra fixture; chưa test quyền nhặt bằng hai tài khoản thật. Chi tiết [GROUND-ITEMS-W0076.vi.md](GROUND-ITEMS-W0076.vi.md).

> W0075: NPC mở rộng đã triển khai và kiểm thử tách riêng sau khi khôi phục CSS. Chi tiết chức năng, source, kiểm tra và giới hạn tại [NPC-EXTENSIONS-W0075.vi.md](NPC-EXTENSIONS-W0075.vi.md). Không coi toàn bộ NPC đã đạt parity100%; NPC CS và giao dịch tiêu hao thật chưa kiểm chứng.

# Checkpoint resource/NPC


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

### W0065 — Lorencia / roof

Đã deploy toàn terrain/map0 +2.787 object đã có mapping, chunk bounds culling, fail scene rõ khi thiếu dependency, retry tải tạm thời. Tái hiện và sửa quota helper: tách asset12.000/min, identity/keys600/min, concurrency4, giữ hash/allowlist/realpath. Mái125/126 fade theo layer1 tile4 đúng source bổ sung. Hai lần nạp/refresh tại125,125 sau sửa đều2.787 object/~60FPS; nội thất bar thấy rõ khi roofẩn. Client132 test toàn bộ pass, render bổ sung8/8 pass, gateway16/16 pass. Chưa xác định model168, chưa full effects/material; chưa test đi bộ online qua cửa trong phiên hiện tại. Log `/tmp/mu-w65-tests.log`, ảnh `/tmp/mu-w65-lorencia-bar.png`, backup `web/.build/deploy-backups/W0065-20261026-145847`.

### W0066 — movement/combat

Đã port input route/picking, world animation modes/zone attachment, foot run gate, click marker gốc, normal attack+damageHP+regen và4monster rig2/3/11/12. Online MuOptions ở184,127 click Spider nhận Hạ quái#61:78; HP65→64. Không sửa Data/server/DB. Marker vàng visual trong OFFLINE simulation. Chi tiết nguồn/test/gap ở `MOVEMENT-COMBAT-W0066.vi.md`; không coi là full combat/mọi quái/skill. Bản cuối cần reload phiên đang mở. Log `/tmp/mu-w66-tests.log`, `/tmp/mu-w66-contracts.log`, `/tmp/mu-w66-wasm.log`.

### W0067 — shadows

World/actor/attachment/NPC/monster đang render đã có shadow pass chiếu terrain dùng chung height texture và stencil. Roof fade đi cùng shadow, effect/additive/multiply không cast.146/146 tests pass; Lorencia actor di chuyển và3NPCfixture ~60FPS trong browser OFFLINE. Source/projection/gap: `SHADOWS-W0067.vi.md`. Backup `web/.build/deploy-backups/W0067-20261026-153650`; ảnh `/tmp/mu-w67-player-shadows.png`, `/tmp/mu-w67-npc-shadows.png`. User cần reload để tạo context stencil mới. Chưa physical shadow-map hay đủ cloaking/map policies; không tuyên bố parity100%.

### W0068 — HUD, Inventory, Character

153/153clienttests,16/16gatewaytests pass.47UI/tableassets hash/decode,9actualserverstructABI,8.192Item+3.258Text parsed. Native640×480atlasHUD và190×429I/V/Cpanels; realMuOptionsLv7HP66MP766SD991AG167/Points9399; fullitemlistLegendary/Bone. Iconcamerađãsửatheonative,Nút+gốc; chưacộngđiểmthật/chưathaygear. C3:24túichính cótransactionlock/ACK; GUIthêmclicknhấcđặt,browserACKmovechưaxácnhận. HelperPIDfileđãcậpnhậtchỉrestart9001. Backup `web/.build/deploy-backups/W0068-20261026-161155`. Docs `HUD-INVENTORY-CHARACTER-W0068.vi.md`; ảnhonline `/tmp/mu-w68-live-hud-inventory-character.png`, fixture `/tmp/mu-w68-offline-ui-final.png`.

W0068 bổ sung: click chuyển Kris ô14→66→14 đã ACK online hai lần và hoàn trả đúng vị trí hiện tại của người dùng; Points9399/Zen211.330.843 giữ nguyên. Ảnh `/tmp/mu-w68-live-ui-verified.png`. Chỉ kết luận click nhấc/đặt main bag pass, không coi native drag/equipment là pass.


## W0069 — source mới và hiển thị item

Đã tìm thấy ScriptItem và UI inventory trong Main5.2; sửa góc staff, căn ô trang bị/armor, thêm nền item, nhấc item, hover 3D, màu nâng cấp và Zen. Chưa tìm thấy ItemTRSData.bmd; renderer còn auto-fit, không xác nhận tỷ lệ riêng từng item hoặc parity 100%. Chi tiết và kết quả kiểm tra: [ITEM-VISUALS-W0069.vi.md](ITEM-VISUALS-W0069.vi.md).


## W0070 — tooltip cổ điển và bỏ viền hover

Bỏ viền vàng item; tooltip nền/mép đen, màu từng dòng, chỉ số CalculateItem và requirement; 160 client test qua. Build version hóa module/CSS để tránh mã cũ từ cache. Chưa port đầy đủ các bảng ItemTooltip custom; tỷ lệ vũ khí ngoài map người dùng báo nhỏ vẫn chưa xác định nguyên nhân và chưa sửa bằng hệ số đoán. Xem [ITEM-TOOLTIPS-W0070.vi.md](ITEM-TOOLTIPS-W0070.vi.md).

W0070 bổ sung sau marker: xác định lỗi ở icon ô trang bị, không phải scale ngoài map. Khôi phục góc staff trước W0069 cho Data hiện tại; áp HideSkin của source để Bone Helm không còn đầu người. Xem chi tiết và test asset thật trong tài liệu W0070.


## W0071 — trang bị, shield và thông số

Sửa icon khiên nhìn nghiêng; nút dùng frame nhấn của resource. Thêm tháo/mặc trang bị ô trống theo ACK server, inventory làm nguồn appearance, bổ sung thông số CALC/reset/fruit và trang Chi tiết. 166 test qua; người dùng đang test online. Chi tiết/giới hạn: [EQUIPMENT-CHARACTER-W0071.vi.md](EQUIPMENT-CHARACTER-W0071.vi.md).


## W0072 — HUD căn giữa và đo FPS

HUD tối đa960px/90%viewport, giữ tỷ lệ và căn giữa; FPS/p95/buffer luôn hiện, RTT đo từ ACKGameServer. Sửa mặt trước shield; đủ ba nút inventory, repair≥50 và personal store≥6. Thêm luồng shop Zen cơ bản theo ACK; chưa test giao dịch bán thật. Người dùng xác nhận giao diện ổn; DOM lúc đọc60.0FPS/p9516.9ms/RTT11.3ms. 170 test qua, xem [HUD-DIAGNOSTICS-STORE-W0072.vi.md](HUD-DIAGNOSTICS-STORE-W0072.vi.md) để biết bằng chứng và giới hạn.


## W0073 — Dấu chọn đích

Dấu chọn đích dùng model trắng + cursorpin02 + vòng empact01 nguyên bản ngay lúc click; hiệu ứng tới nơi tách sau ACK/nội suy. Cursor/Push dùng OZT gốc. 172 client test và4 helper test qua. Spear joints/spark particles và các cursor trạng thái khác còn thiếu. Xem [checkpoint W0073](MOVE-TARGET-W0073.vi.md).


## W0074 — NPC/shop và resource item

Luồng NPC shop thường, mua/bán/repair theo ACK và CursorTalk gốc đã có. Sửa C3 tránh Session closed. Mở shop Lumen live đã được xác nhận; chưa agent giao dịch thật. Bổ sung model/texture:902/924item bảng Data đủ resource,22binding chưa chốt; màu đồ cộng áp mọi nhóm và70ngoại lệ source. Quest/kho/chaos/custom particles còn thiếu. Xem [checkpoint W0074](NPC-SHOP-ITEMS-W0074.vi.md) và báo cáo item kèm theo.
