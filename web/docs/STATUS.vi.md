## 2026-11-05 — Chết và hồi sinh Duel

Self/remote dùng animation chết231 từ Player.bmd, giữ corpse tới destroy/respawn. Sửa server chọn nhầm người chết làm đối thủ; hồi sinh/reset vòng cùng map không reload, vẫn FinishLoading đúng. Hai phiên thật đã qua kill→điểm1–0→hồi sinh cả hai→đi lại. **501/501 web,8/8 native game**, debug174file/WASM mới. QA đã dọn. [Báo cáo, ảnh và giới hạn](DUEL-DEATH-20261105.vi.md).

## 2026-11-04 — Sửa hộp mời và damage Duel

Đóng hộp mời sau một answer, sửa GlobalText910/911 và GUI gốc; mở đánh thường/skill/Evil Spirit đúng đối thủ Duel, CursorAttack gốc. Theo yêu cầu chuyển điểm xuống góc phải cạnh HUD. Test2 phiên thật có HP/SD damage cả3 loại; **489/489 web qua**, debug173file. QA đã dọn. [Báo cáo](DUEL-COMBAT-20261104.vi.md).

## 2026-11-04 — Hoàn tất Duel Watch client/server

Titus → phòng đấu → HUD khán giả/HP/SD/điểm → rời về Lorencia và đi lại đã test với3 phiên thật. Sửa lỗi server xoá membership khi gate thất bại; bấm rời quá sớm giữ trạng thái và retry được. **485/485 web,34/34 native qua**, debug173file; ảnh/button từ DATA gốc. QA đã xoá sạch. [Báo cáo và giới hạn kiểm chứng](DUEL-WATCH-20261104.vi.md). Map không mở rộng trong lượt này.

## 2026-11-04 — Elbeland: lửa đỏ/xanh và kiểm tra bone effect

Khôi phục50emitter Elbeland (27TRUE_FIRE/23TRUE_BLUE) đúng texture/subtype0. Dùng chung provider bone và native particlepool cho HIK3 subtype4; bổ sung policy đèn Empire37/12 theo source. **474/474 test qua**, debug172file. Browser Elbeland60FPS,dropped0. Audit47map phát hiện riêng type50/map72: source cần bone2–7 nhưng Data chỉ1bone; bỏ riêng effect, không thay bone đoán. [Báo cáo](MAP-BONE-VALIDATION-20261104.vi.md).

## 2026-11-04 — Empire: effect theo vòng đời, khói máy và vật liệu

Hoàn thiện marker85 FLAME6→particle12, type64 CLOUD22 theo frame/model period, sửa rate riêng từng controller và các mesh sáng/tint/UV dùng chung69–74. Giữ cửa sổ thời gian source, loại lịch phát bù gây quá tải. **465/465 test qua**, debug172file. Browser map72:54effect đã tạo,đỉnh540flame/84cloud,dropped0,60FPS. Coverage47map vẫn254unresolved; các joint/chrome/event chưa đóng. [Báo cáo](MAP-EVENTS-20261104.vi.md).

## 2026-11-04 — Map: sửa tổng hợp và kiểm kê toàn bộ

Gom nhánh chung Empire69–74 (lửa/mây/khói/nước/UV/rate), thêm bone light Market/Elbeland/Empire, sửa scatter/fade nước và JPEG Flare có trailer khác0. Sửa cap256 làm mất cloud; browser kiểm tra lại dropped0. **456/456 test qua**, debug171file; audit đủ47map/98.353placement. Còn4map terrain thiếu texture và254placement chưa giải quyết; hiệu ứng event/joint/material chưa chứng nhận đầy đủ. Không coi lượt này là100% bản PC. [Báo cáo tổng hợp và danh sách còn vướng](MAP-CONSOLIDATED-20261104.vi.md).

## 2026-11-04 — Đợt 8D: lửa United Market Place

Port12 marker57 thành glow và3 family lửa màu HIK1/Curse4/HIK3 đúng texture/lifetime/fade; không dùng nhánh mono. Có policy58 chưa có placement Data. **448/448 test qua**, debug169 file. Mẫu51 flame/12sprite/289water, dropped0 và60FPS. MarketPlace còn bone light30/35; unresolved254 giữ nguyên. [Báo cáo](MAP-PHASE8D-20261104.vi.md).

## 2026-11-04 — Đợt 8C: nước United Market Place

Port14 marker nước54/55/56 map79 bằng WATERFALL5/0,3/8,2/4 đúng xác suất. Sửa thứ tự RNG/scale ghi đè dùng chung Elbeland; renderer cấp slot theo nhóm và báo dropped thay vì cắt âm thầm ở256. **443/443 test qua**, debug169 file. Map79 mẫu236 particle/dropped0,60FPS; kiểm tra lại Elbeland117 particle/dropped0. Unresolved254 giữ nguyên; marker lửa57 chưa port. [Báo cáo](MAP-PHASE8C-20261104.vi.md).

## 2026-11-04 — Đợt 8B: đèn Santa Town / dependency build

Port169 sprite đèn màu map62 đúng texture/màu/pulse/scale và cap1000; type16 animation1,5keys/s. Trình duyệt mẫu169 sprite/dropped0,60FPS. Sửa404 item-use-policy bị thiếu allowlist; build kiểm tra graph import reachable trước publish. **438/438 test qua**, debug169 file. Không restart server; unresolved vẫn254. [Báo cáo](MAP-PHASE8B-20261104.vi.md).

## 2026-11-04 — Đợt 8A: vật liệu Atlans / bubble map

Sửa BlendMesh/pulse Atlans theo ZzzObject; thêm bubble marker7/67/68 đúng atlas9 frame, timer, subtype/lifetime và UV buffer động. Pool bubble3000 có đếm dropped; map67 mẫu375 bubble/dropped0,60FPS. **426/426 test qua**, debug build thành công. Unresolved254 giữ nguyên; chưa đóng toàn bộ hiệu ứng map. [Báo cáo](MAP-PHASE8A-20261104.vi.md).

## 2026-11-04 — Đợt 7: gỡ chặn terrain map

Terrain tải được38→43/47 (gỡ7/40/42/46/67); binding texture theo OpenFileTile và chỉ yêu cầu layer thực sự vẽ. Atlans/DG3 dùng32 frame nước Object8 theo source. **420/420 test web qua**, debug build thành công; map7 và67 mở được trên trình duyệt offline. Triage đủ254 placement; vẫn giữ4 map thiếu texture và chưa chứng minh mọi type marker. [Báo cáo](MAP-PHASE7-20261104.vi.md).

## 2026-11-04 — Đợt 6: vật liệu DoppelGanger

Bổ sung BlendMesh/pulse đúng slot của DG3, alpha type76 DG4 và giữ render override type96. Batch tách theo pulse/opacity. **416/416 test web qua**, debug build thành công. Kiểm tra hình ảnh map67 đang bị chặn bởi thiếu TileWater01.OZJ; chưa kết luận visual parity. Unresolved placement vẫn254. [Báo cáo](MAP-PHASE6-20261104.vi.md).

## 2026-11-04 — Đợt 5: map sự kiện

Port khói marker132 của Empire4/scene73–74, nhánh HIK DoppelGanger và rate/UV event object. Map74 có3 emitter thực; unresolved257→254 do phân loại đúng marker. **413/413 test web qua**, debug build thành công. Data smoke1×1 và các nhánh FX còn thiếu được ghi rõ. [Báo cáo](MAP-PHASE5-20261104.vi.md).

## 2026-11-04 — Đợt 4: Elbeland water / frustum CPU

Khôi phục16 placement nước bằng lọc đúng mặt zero-area sentinel; unresolved từ273 còn257. Thêm UV GMNewTown và CPU p95. Frustum support-point thay8 góc, giữ hình/margin; **410/410 test web qua**. Có benchmark hàm và số đo trình duyệt, chưa cam kết mọi map60 FPS. [Báo cáo](MAP-PHASE4-20261104.vi.md).

## 2026-11-04 — Đợt 3: Raklion / Kanturu / Tarkan

Port HIK xanh/đỏ, CLOUD10/11, WATERFALL7, UV/BlendMesh và material pulse theo placement Tarkan. Phân loại4 marker Kanturu, unresolved từ277 còn273; giữ model thiếu, không đoán thay thế. **405/405 test web qua**. Có số đo GPU mẫu và giới hạn chưa port trong [báo cáo đợt3](MAP-PHASE3-20261104.vi.md).

## 2026-11-04 — Đợt 2: Crywolf / Swamp

Khôi phục 515 placement cây hợp lệ, unresolved toàn map từ 792 còn 277. Bổ sung các subtype lửa/khói/sương, terrain light và phân loại marker ở map34/56; sửa đọc JPEG có padding0. **400/400 test web qua**; kiểm tra GPU offline hai map. Vẫn còn 5 placement thiếu model ở hai map và giới hạn effect/texture. [Chi tiết](MAP-PHASE2-20261104.vi.md).

## 2026-11-04 — Triển khai sửa sau audit

Đã sửa retry/cache icon, xử lý sync/async Helper và dừng khi đổi map, lớp material Excellent theo source, tải UI tối đa 3 worker. **394/394 test web qua**; build debug cập nhật. Chỉ giữ snapshot mới nhất mỗi nhóm đã dọn. [Chi tiết và phần còn lại](REMEDIATION-20261104.vi.md).

## 2026-11-04 — Audit client/server và tài liệu

Đã kiểm kê 2.317 tệp source và 146 Markdown; build native sạch thành công, web 387/387, gateway 16/16, Python 24/24. Có các lỗi/giới hạn cần xử lý: material Excellent, Promise Helper, cache icon lỗi, mapping map/item và tài liệu startup. Chưa sửa gameplay trong lượt audit. [Báo cáo và bằng chứng](AUDIT-CLIENT-SERVER-20261104.vi.md).

## 2026-11-01 — Sửa D/Buy mở shop người chơi

Đã nối callback lần đầu vào map và khi dựng lại scene; sửa payload, mở panel theo ACK/state, cho phép buyer cấp thấp và đóng shop không đợi ACK.29test qua, GUI offline đã kiểm tra; chưa xác nhận hai account thật. [Chi tiết](PERSONAL-SHOP-W0117.vi.md).

## W0117 — Cửa hàng cá nhân (2026-11-01)

Đã bổ sung mở shop người khác, mua, rời shop, danh sách hàng, viewport shop title, đổi tên shop và UI purchase tối giản; cho phép cập nhật tên khi đang bán. **324/324 test** pass. Chi tiết/giới hạn: [PERSONAL-SHOP-W0117.vi.md](PERSONAL-SHOP-W0117.vi.md).

## W0116 — rà item/tooltip/giá theo source PC (2026-10-31)

Đã sửa độ bền tối đa (cap giữa vòng, staff+36, helper 4/5, lucky 255, wing41), màu ô item (helper 106/109–115), chỉ số trang bị (ancient buộc excellent, yêu cầu cấp ≥220, Charisma, bonus damage/defense/MagicPower, option +5 Strength, BOOK/SKILL_SCROLL Energy, Cepter, loại trừ excellent Archangel/wing), màu tên/tooltip (wing ưu tiên, nền ancient), màu tên đồ rơi theo `HookColorFont`, class text MG/DL bậc 2, full-set smoke cần boots, giá NPC (stack, phạt độ bền, công thức wing/helper, excellent 100%/25%), **ItemTRSData.bmd** cho kích thước icon và **override scale/angle/pos/model RenderItem3D** cho item đặc biệt. **321/321 test** pass. Chi tiết: [ITEM-SOURCE-AUDIT-W0116.vi.md](ITEM-SOURCE-AUDIT-W0116.vi.md).

## W0115 — effect toàn bộ skill 7 class (2026-10-31)

Đã phủ **114/114 skill chủ động** cast được: **3 exact** (Fire Ball/Evil Spirit/Energy Ball giữ prototype), **60 source** có token `CreateEffect`/`CreateJoint`/`CreateParticle` thật trong `ZzzCharacter.cpp`, **51 derived** cùng họ hình ảnh, **0 missing**. Bổ sung 12 skill ngoài bộ cũ: 62/65 dùng AG và 10 master skill có handler thật trong `CSkillManager::RunningSkill` (344, 425, 426, 427, 430, 432, 461, 495, 521, 565); 511/515/517 là passive, 79 Explosion không có handler nên loại. Thêm catalog 13 họ + runtime 12 mode, giới hạn 12 effect đồng thời, dispose theo scene. **315/315 client test** pass, **13/13 texture** `Data/Effect` giải mã, build debug **147 file**. 51 derived chưa có token nguồn riêng và chưa port đầy đủ material/animation/âm thanh; preview menu vẫn chỉ dựng 3 prototype, effect đầy đủ ở world scene. Chi tiết/bảng 114 skill: [SKILLS-FULL-W0115.vi.md](SKILLS-FULL-W0115.vi.md); coverage: [SKILL-COVERAGE.vi.md](SKILL-COVERAGE.vi.md).

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

> W0096: [Pet từ Data/source gốc](PET-RESOURCES-W0096.vi.md):8helper pet, Pet.bmd scale/blend/speed, Satan bone34 + RF armor offset. Người dùng xác nhận hướng đi đã đúng; đã sửa UV/mask Satan và sprite đỏ theo bone34;262test qua.

> W0095: [Kiểm tra cánh theo class](WINGS-CLASS-AUDIT-W0095.vi.md):18 loại/20 cặp đúng Item table; sửa blend cánh Elf, thêm pass MG/DK; áo choàng và FX còn thiếu được ghi rõ.

> W0093: [Item, cánh, pet và remote player](ITEM-WING-PET-PLAYERS-W0093.vi.md); audit924 item, sửa cánh/companion theo W0094, nối companion bits. Pet cưỡi/controller đặc thù và tỷ lệ ItemTRSData còn thiếu.

> W0092: [Cửa tự mở Devias](DOORS-W0092.vi.md): trượt/xoay theo khoảng cách và âm thanh nguồn; cửa event/CS còn luồng riêng.

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

# Nhật ký và trạng thái WebGL

Checkpoint mới nhất: [W0074 — NPC/shop, resource và màu item](NPC-SHOP-ITEMS-W0074.vi.md).

Checkpoint trước: [W0073 — Con trỏ và dấu chọn đích](MOVE-TARGET-W0073.vi.md).

Checkpoint trước:  [W0072 — HUD/FPS, shield và inventory footer](HUD-DIAGNOSTICS-STORE-W0072.vi.md).

## Quy tắc cập nhật

- DONE: có artifact và bằng chứng kiểm tra cho đúng phạm vi ghi trong ticket.
- PARTIAL: đã làm một phần; phần thiếu phải được ghi, không tích checkbox toàn mốc.
- TODO: chưa triển khai. BLOCKED: thiếu đầu vào được chỉ rõ.
- Mỗi đợt ghi thay đổi, lệnh kiểm tra/kết quả, sai khác và bước kế tiếp.
- Test native không phải test web. Có file không đồng nghĩa decoder đúng; có code không đồng nghĩa chạy đúng trong browser.

## Bảng mốc

| Mốc | Trạng thái | Bằng chứng / phần còn thiếu |
|---|---|---|
| M00 | PARTIAL | Hash Data/source/config, archive source và audit đã có; dependency/decoder toàn Data, baseline visual và DB backup chưa có |
| M01 | PARTIAL | Debug/Release tách; W0010 browser WebGL2/WASM canvas đã kiểm tra, chưa GUI/scene game và browser matrix |
| M02 | PARTIAL | Core/WASM framing, crypto, login/lobby/appearance codecs và asset manifest/loader có test; chưa model/texture runtime/render/GUI game |
| M03 | PARTIAL | Gateway/discovery/probe qua network fixture; login/lobby qua socket mô phỏng, chưa provider/provisioning/MU online/WSS deploy |
| M04–M12 | TODO | Chưa pipeline runtime, renderer game, UI, combat, audio web hoặc deploy |

## Đợt W0001 — 2026-10-23

Mục tiêu: bắt đầu M00, chỉ đọc source/Data và lưu bằng chứng cục bộ. Không bật server, không sửa database hoặc resource gốc.

| Ticket | Trạng thái | Đã làm / giới hạn |
|---|---|---|
| M00.1a | DONE | Công cụ baseline, SHA-256 và archive 649 file source đã chọn; mở lại archive kiểm tra từng nội dung với manifest |
| M00.1b | DONE | Hash 22 file cấu hình; không copy nội dung cấu hình vào archive. Hash không phải bản backup có thể phục hồi config |
| M00.1c | TODO | Backup DB trước test online có ghi. Đợt này không truy cập DB và không bật MySQL |
| M00.2 | DONE | SOURCE-MAP xác định EX603 server/extension, implementation native, reference ngoài và phần thiếu |
| M00.3a | DONE | Inventory 13.660 file Data, 911.910.018 byte, SHA-256 từng file, extension và kiểm tra trùng case; 0 lỗi đọc, 0 symlink, 0 collision |
| M00.3b | TODO | Decode toàn bộ model/table/terrain; kiểm tra texture/bone/clip/dependency thiếu. Inventory không chứng minh 5.183 BMD đều là model hợp lệ |
| M00.4 | DONE | KNOWN-GAPS ghi các lỗi/giới hạn cần kiểm tra lại; không đánh dấu các lỗi Mac cũ là đã hết |
| M00.5a | DONE | Ghi máy thực và ma trận/tiêu chí nghiệm thu đề xuất trong BASELINE |
| M00.5b | TODO | Đo browser thực, DPR, screenshot và benchmark cảnh chuẩn; Chrome chưa có baseline |

Kiểm tra thực hiện trong đợt này:

1. `python3 -m unittest discover -s web/tests -v`: 3/3 qua. Kiểm tra hash đổi khi nội dung đổi, không sửa nguồn, không theo symlink và không đưa runtime/config vào tập source.
2. `baseline.py`: kết thúc 0; hash-baseline-complete, archive được verify lại. Điều này chỉ chứng minh phạm vi hash/archival, không phải M00 hoàn tất.
3. `ctest --test-dir native/build -R '^(mu_client_contract|mu_client_localization_contract|mu_game_timer_contract|mu_game_book_requirements)$' --output-on-failure`: 4/4 qua, 1,46 giây. Các executable native có sẵn; chưa clean-rebuild trong đợt này, chưa test WASM/browser.
4. Mẫu Mac đọc tại máy: Apple M2, RAM 8 GiB, macOS 26.3, Safari 26.3. Không ghi hardware UUID hoặc serial.

Artifact cục bộ (thư mục bị ignore, quyền 0700): `web/.local-baselines/2026-10-23-m00-001/`. Xem BASELINE để biết phạm vi archive và lệnh tái lập.

Thay đổi production: không có. Chưa sửa protocol, tốc độ, item, skill, server config hoặc Data.

Bước kế tiếp: M00.3b — tạo audit dependency bằng parser hiện có, xuất file/model/texture/clip lỗi và coverage; sau đó chốt baseline visual trước khi đánh dấu M00 hoàn tất. Không nhảy thẳng sang làm UI hay gateway.

## Đợt W0002 — 2026-10-23

M00.3b: **PARTIAL**. Đã build và chạy CLI audit trên toàn bộ ứng viên BMD/OZJ/OZT. 5.025 model và 7.278 texture đọc được; 9 model + 8 texture báo lỗi parser; 149 BMD chưa phân loại sâu. Có 143 tham chiếu texture cục bộ chưa resolve thuộc 103 model. Chi tiết, giới hạn và cách tái lập: [ASSET-AUDIT](ASSET-AUDIT.vi.md), [báo cáo JSON](asset-audit-W0002.json).

Đã xong: công cụ audit offline, báo cáo từng file, kiểm thử fixture và baseline 4/4 qua sau khi build CLI. Chưa xong: giá trị mọi frame animation, terrain/object placement, table, chính sách shared texture/fallback gốc và baseline visual browser. Chưa đánh dấu resource/parser lỗi là đã sửa. M00 vẫn PARTIAL; M01–M12 vẫn TODO.

Không thay đổi production, Data, DB hoặc cấu hình server. Không khởi động server. Bước tiếp theo: audit placement/terrain và phân loại 149 BMD còn lại, đối chiếu các dependency chưa resolve trước khi xây asset pipeline.

## Đợt W0003 — 2026-10-23

M00.3b tiếp tục **PARTIAL**. Đã bổ sung CLI đọc terrain/object bằng parser native, kiểm tra model dependency theo mapping hiện tại và chạy parser Text/Item/Skill cho ba ngôn ngữ. [Kết quả và giới hạn chi tiết](WORLD-AUDIT.vi.md), [JSON từng world/table](world-audit-W0003.json).

- DONE trong phạm vi audit: terrain/object của 5 world hỗ trợ đều đọc được; 9 bảng Text/Item/Skill decode được; phân loại theo đường dẫn toàn bộ 149 BMD không mang magic model từ W0002.
- Phát hiện: 13 tên model chưa tìm thấy ứng với 22 placement, 1 type chưa mapping; 12 type native bỏ qua được tách riêng. Chưa sửa hay kết luận Data gốc hỏng.
- TODO: 42 world parser chưa hỗ trợ; 140 BMD chưa decode; terrain texture/light/fallback, mọi animation frame và visual/browser baseline chưa xác minh.
- Kiểm tra: build CLI thành công; `python3 -m unittest discover -s web/tests -v` **6/6 qua, không skip**.
- Production/Data/config/DB không thay đổi; server không bật. M00 vẫn PARTIAL; M01–M12 chưa triển khai.

Bước sau: đối chiếu loader/mapping và format còn thiếu với source để mở rộng phạm vi decoder có bằng chứng; chưa chuyển sang dựng UI khi chưa chốt nguồn asset.

## Đợt W0004 — 2026-10-23

M00.3b vẫn **PARTIAL**. Đã thêm audit cấu trúc độc lập để đọc tất cả 47 world theo schema của native, không mở rộng map production. [Chi tiết](STRUCTURE-AUDIT.vi.md), [JSON từng file và SHA-256](world-structure-W0004.json).

- DONE phạm vi cấu trúc: 47 mapping + 47 attribute + 43 placement + 39 height qua schema; 176/188 file. Có 35 world qua cả bốn loại; 5 world hỗ trợ native khớp số liệu W0003.
- Cần đối chiếu: 4 placement và 8 height không qua schema hiện tại, đã ghi rõ file/record/header. Không tự kết luận Data hỏng và không sửa Data để bỏ qua lỗi.
- Source: chưa tìm thấy loader engine Windows EX603 đầy đủ trong cây được cung cấp. Mapping/visual parity vẫn thiếu bằng chứng; không đoán tên model thay thế.
- Kiểm thử: 10/10 qua, không skip. Chưa chạy browser/gameplay. Server/DB không bật, production không thay đổi.
- Còn thiếu: decoder cho các trường hợp khác schema, model/tile/visual của 42 world, 140 BMD bảng và toàn bộ animation frame. M01–M12 vẫn chưa triển khai.

Bước tiếp theo: phân tích 12 trường hợp schema còn lại, bổ sung decoder có fixture khi đủ bằng chứng. Mọi mục thiếu hoặc chưa chuẩn tiếp tục giữ trong checklist.

## Đợt W0005 — 2026-10-23

Đã điều tra đủ 12 trường hợp W0004, đối chiếu hash input không thay đổi. [Phát hiện và giới hạn](EXCEPTIONS-AUDIT.vi.md), [báo cáo từng file/record](world-exceptions-W0005.json).

- DONE trong phạm vi audit: decode pixel 3 BMP 24-bit và 1 BMP palette 120; định vị đầy đủ 2 record NaN và 4 record scale âm trong 4 file placement.
- Chưa hoàn thiện: World11 thiếu pixel so với BMP header; World19 toàn zero; World7/55 chưa phân loại wrapper. Chưa biết quy tắc chuyển pixel thành height và xử lý NaN/scale âm của engine gốc.
- Kiểm thử 14/14 qua, không skip. Không đổi Data/production/config, không bật server/DB. Chưa kiểm thử browser/render/gameplay.
- M00 tiếp tục PARTIAL; không đổi kết quả W0004 thành “tất cả đạt”. Những mục thiếu bằng chứng engine gốc vẫn mở; M01–M12 chưa triển khai.

## Đợt W0006 — 2026-10-23

M00.3b tiếp tục **PARTIAL**. Đã bổ sung audit mọi float position/rotation/locked-position của BMD10/12, không chỉ clip được giữ. [Bằng chứng](ANIMATION-AUDIT.vi.md), [JSON tổng hợp](animation-audit-W0006.json).

- DONE trong phạm vi: quét 5.034 model, cùng tập đường dẫn W0002 và cả 5.034 hash khớp W0001. 5.028 model qua hoàn chỉnh, gồm 10.396.461 float animation; 6 file dừng tại lỗi đầu tiên.
- Ngoại lệ: 4 file version 0 chưa hỗ trợ; Item/dark_05 có NaN; Object79/Object05 có locked-position vượt ngưỡng số native. Hai lỗi số mới được ghi, chưa sửa hoặc kết luận source Data hỏng.
- 5.023 model cùng qua mesh W0002 và animation numeric W0006. Những lỗi mesh cũ vẫn mở; không gộp “animation pass” thành “model hoàn thiện”.
- Test: 18/18 qua, không skip. Chưa đo visual, gameplay, animation timing hoặc browser FPS.
- Production/Data/config không thay đổi, server/DB không bật. M00 vẫn PARTIAL, M01–M12 chưa triển khai.

## Đợt W0007 — 2026-10-23

Bắt đầu **M01 PARTIAL** song song với các mục M00 còn mở. Việc tạo build trống không phụ thuộc việc biết cách engine Windows xử lý các asset ngoại lệ; không đóng M00 để chuyển mốc.

- DONE: cấu trúc client/gateway, ES modules và diagnostics WebGL2, pin Node 26.11.1/npm 11.20.0; [ADR-001](ADR-001.vi.md), [quy ước ID/coordinate/endian/ownership](CONVENTIONS.vi.md). Gateway chưa có implementation.
- DONE: build web độc lập với CMake/Cocoa, output allowlist 4 file ở web/dist (ignore). Không dependency ngoài, không cần npm install. Có lệnh build/test/serve trong README.
- Kiểm tra thực tế: `npm --prefix web run build` qua; `npm --prefix web test` **2/2 qua**. Test kích thước canvas/DPR/giới hạn cấp phát và output đúng allowlist/nội dung nguồn.
- Native: `cmake --build native/build --target mu_client_assets -j 2` build thành công; Ninja có cảnh báo “premature end of file; recovering” lúc đầu và hoàn tất. Đây chỉ là target assets, không phải clean build toàn bộ client/server.
- `ctest --test-dir native/build -R '^(mu_client_contract|mu_client_localization_contract|mu_game_timer_contract|mu_game_book_requirements)$' --output-on-failure`: **4/4 qua**, executable có sẵn; không phải test browser/WASM.
- TODO: mở diagnostics trên browser thật để xác minh WebGL2/resize/context, build WASM và full native, renderer game, gateway, identity, protocol và asset tải thật. Không có đo FPS game hoặc claim 60 FPS.
- Không thay Data/production/server config; không bật MU server/DB. Font diagnostics hệ thống không phải font UI game; font MU cho web vẫn mở.

Bước tiếp theo: xác minh diagnostics trong browser và tách codec/frame reader portable có fixture ở M02; giữ các vấn đề parity/resource M00 mở, không cần đoán format để tiếp tục phần độc lập.

## Đợt W0008 — 2026-10-23

Bắt đầu **M02 PARTIAL**: core framing C++17 thuần, standalone CMake không phụ thuộc Cocoa/socket. [Phạm vi và bằng chứng](CORE-FRAMING.vi.md), [hướng dẫn core](../../client-core/README.vi.md).

- DONE phạm vi framing: C1/C2/C3/C4, chunk chia/ghép, frame/buffer bounds, EOF, ownership và trạng thái lỗi. Quy tắc theo native Socket::try_frame; chưa tích hợp reader vào production.
- Kiểm thử thực: npm test **3/3 qua**, CMake build core qua, CTest **1/1 qua**. Fixture tổng hợp, không phải login thực và không xác thực ciphertext C3/C4.
- TODO: transport decode/crypto, serial/session, gameplay state, timeout adapter và gateway. `emcc` chưa có trong PATH; chưa compile WASM/native differential hoặc test browser.
- Không đổi production/Data/config; không bật MU server/DB. M00/M01 vẫn PARTIAL. Chưa có client game web hoạt động.

## Đợt W0009 — 2026-10-23

Đã bổ sung toolchain Emscripten và build/chạy core trên WASM. [Cách cài, giới hạn launcher và kiểm chứng](WASM-TOOLCHAIN.vi.md), [metadata/hash](wasm-build-W0009.json).

- DONE: formula 6.0.13, compiler 6.0.13-git; adapter build cục bộ dùng Python 3.14.6 + Node 26.11.1. Global Homebrew launcher còn thiếu Python 3.15; không gọi global toolchain hoàn thiện.
- DONE: bridge Embind Uint8Array và output copy; module ES6 + WASM và cùng C++ contract đã build/chạy bằng Node. Native và WASM đều qua cùng contract fixture tổng hợp.
- Kiểm thử: `test:wasm` qua cả bridge checks/C++ contract; npm test 3/3 qua; build web tĩnh 4 file qua.
- Cài lần đầu thất bại do dung lượng, đã báo người dùng; sau khi người dùng dọn ổ đã cài riêng formula, không nâng cấp Node/Python. Không xóa file cá nhân/Data.
- TODO: browser thực, lifecycle/exception adapter production, crypto/identity/state và gateway/login. M02 vẫn PARTIAL, M00/M01 chưa đóng. Chưa có game web online.
- MU server/DB không bật; Data/config/production không thay đổi.

Bước tiếp theo: tích hợp module vào diagnostics và kiểm chứng trong browser thật trước khi nối transport/gateway.

Trong kiểm tra W0009, test allowlist phát hiện Finder tạo .DS_Store trong dist. Đã sửa build tái tạo riêng thư mục output sinh tự động trước khi copy allowlist; không thay test để bỏ qua file dư. Sau sửa, chạy lại npm test, test:wasm và build đều qua.

## Đợt W0010 — 2026-10-23

Đã tổ chức lại runtime và thêm hai build mode theo yêu cầu người dùng. [Cây thư mục, lệnh và quy tắc](PROJECT-LAYOUT.vi.md).

- DONE: app entrypoint / platform rendering / WASM adapter / diagnostics tách riêng. Build WASM không còn phụ thuộc thư mục baseline theo ngày, chuyển sang `.build/wasm` bị ignore. Không đổi source/Data server-client gốc.
- DONE: Debug có FPS canvas đo thực, khung/buffer/DPR, WebGL2/WASM status; ping ghi chưa kết nối vì chưa có transport. Release loại diagnostics khỏi output allowlist, không chỉ ẩn UI. Hai output `dist/debug` và `dist/release` riêng.
- DONE: chạy trong in-app browser qua HTTP localhost, quan sát WebGL2 sẵn sàng và framing WASM đạt 4 loại frame/19 split cases. Không kiểm tra bằng file:// vì trình duyệt không cho phép protocol này.
- Kiểm thử: npm test 4/4 qua; test:wasm qua shared browser self-test, bridge checks và cùng C++ contract. Build WASM qua sau thay đường dẫn.
- Chưa xong: ping thực cần server/gateway, renderer cảnh/gameplay, crypto/login, full native/WASM parity. Chưa benchmark game 60 FPS hoặc xác minh Safari/Chrome độc lập. M00/M01/M02 vẫn PARTIAL theo các mục còn mở.
- MU server/DB không bật. Preview static local port8080 phục vụ dist được bật để người dùng kiểm tra.

Cập nhật W0010 theo phản hồi người dùng: Debug dùng canvas full viewport giống Release; chỉ số đo nằm trong overlay nhỏ cố định góc trên trái, không bắt chuột. Đã bỏ heading, mô tả, panel và viền trang diagnostics. Browser xác nhận canvas + overlay, WebGL2 sẵn sàng và WASM đạt. FPS đo canvas qua requestAnimationFrame; không coi giá trị canvas là hiệu năng game. Release không có overlay. Sau chỉnh, npm test 4/4 và test:wasm qua.

## Đợt W0011 — 2026-10-23

Đã bổ sung transport bất đồng bộ/vòng đời phiên và reader adapter an toàn cho lỗi C++. [Chi tiết và giới hạn](TRANSPORT.vi.md).

- DONE phạm vi adapter: mỗi session một socket/reader, nhận byte theo thứ tự, timeout kết nối/packet, queue bounds, EOF và dispose đúng một lần. Encode/decode adapter bắt buộc; không có crypto mặc định giả.
- DONE: feedResult/finishResult bắt lỗi trong WASM; create-reader chuyển lỗi sang Error JavaScript và delete idempotent. Production native không thay đổi.
- Kiểm thử: rebuild WASM qua; npm test **10/10 qua**, test:wasm qua. Socket/clock giả lập nhưng dùng core WASM thật. Chưa có network/gateway/server test.
- Debug/Release giữ layout W0010, diagnostics không có trong Release. Chưa khởi tạo transport từ app; ping chưa kết nối.
- M02 vẫn PARTIAL: crypto, identity/serial/state và online fixture còn thiếu. M03 chưa gateway implementation. MU server/DB không bật, Data/config không thay đổi.

Bước kế tiếp: gateway local với route allowlist cố định và kiểm thử TCP/WebSocket fixture; chưa login thật hoặc bypass ClientVerify.

## Đợt W0012 — 2026-10-23

Bắt đầu **M03 PARTIAL**: gateway loopback WS/TCP đã có implementation, CLI, cấu hình mẫu và lockfile riêng. [Chi tiết](GATEWAY.vi.md), [hướng dẫn gateway](../gateway/README.vi.md).

- DONE local: route allowlist cố định, Origin kiểm tra trước upgrade, listener/upstream loopback, mỗi phiên một TCP, binary-only, payload/queue/byte budget, pause/resume và heartbeat/shutdown.
- Cài ws8.22.0 từ registry npm chính thức, pin exact + package-lock; không optional addon/install scripts, không đưa dependency vào frontend. Kết quả npm audit tại lúc cài: 0 vulnerability được báo.
- Test gateway thực qua loopback **7/7**. Gồm Node WS + TCP fixture và chuỗi TransportSession → WASM → gateway → TCP; không phải browser UI hoặc server MU thật. Tất cả listener fixture đóng sau test.
- Frontend/core npm test **10/10**, test:wasm qua; Release tiếp tục không có diagnostics/config/server dependency.
- TODO: cipher/identity/ClientVerify, ConnectServer routing/discovery/login thật, WSS deployment, load/soak và slow-client stress. Không claim game online/ping MU từ test fixture.
- Không bật MU server/DB, không sửa Data/config MU hoặc tự nối gateway trong app. M00/M01/M02 vẫn giữ các phần chưa hoàn thiện.

Bước sau: đối chiếu transport cipher và server-code routing theo source, tạo fixture codec sạch trước khi thử discovery/login thật.

## Đợt W0013 — 2026-10-23

Đã thêm codec ConnectServer và byte-transform transport vào core C++/WASM, kèm JS checked-result adapter và helper route chọn server. [Phạm vi/source/giới hạn](CONNECT-CODEC.vi.md).

- DONE phạm vi codec: handshake, list/endpoint request, list count/length/endian/duplicate, endpoint metadata và code16-bit trong list vs code1-byte khi chọn; route phải khớp code/address/port cấu hình.
- DONE transport byte transform + MHP theo source, explicit byte keys; contract test so với thân hàm GameServer/HackCheck.cpp trong namespace fixture, không chỉ roundtrip. Không đọc key/serial thực.
- Chưa xong: key derivation, Enc1/Dec2, XOR/SimpleModulus C3/C4, serial, ClientVerify/login. ConnectServer plain transport không bị áp GameServer cipher.
- Không tự connect từ app, không bật MU server/DB hoặc đổi Data/config/native production. M02/M03 tiếp tục PARTIAL. Debug/Release layout giữ nguyên.

Kết quả kiểm chứng W0013: build:wasm thành công; npm test **12/12 qua**, không skip. test:wasm qua framing/reader cũ và codec JS-WASM mới, cùng protocol-contract C++ so với reference hàm server. Build Debug/Release và allowlist tiếp tục qua. Không có test server MU online trong đợt này.

## Đợt W0014 — 2026-10-23

Đã thêm luồng discovery/chọn server bất đồng bộ trong `client/src/session/discovery.js`. [API, kiểm chứng và phần thiếu](DISCOVERY.vi.md).

- DONE phạm vi session: handshake → list request/response; phiên mới handshake → endpoint request/response → kiểm chứng route. Không tự nối GameServer, không biến endpoint thành TCP đích tùy ý.
- DONE: deadline phase, AbortSignal, lỗi packet/route, peer close, config snapshot và cleanup. ConnectServer plain-byte adapter theo source, GameServer cipher vẫn riêng.
- Kiểm thử: frontend/core **16/16**, gateway/network **9/9**, test:wasm qua, không skip. Network dùng ConnectServer fixture local + WASM thật; không phải MU server thật/browser UI.
- UI Debug/Release giữ W0010, app chưa tự discovery; ping vẫn chưa kết nối. Fixture/gateway test đóng sau chạy. Không bật MU server/DB hoặc đổi Data/config/native production.
- TODO: game handshake, profile/key derivation, ClientVerify/identity, C3/C4 crypto và login thật. M02/M03 tiếp tục PARTIAL.

Bước kế tiếp: handshake GameServer và profile/key derivation bằng fixture source, trước khi mở kết nối MU thật.

## Đợt W0015 — 2026-10-23

Đã thêm transport key derivation theo MuOnline source và parser GameServer greeting EXTRA1 trong core/native-WASM. [Phạm vi và giới hạn](GAME-PROFILE.vi.md).

- DONE: derivation modulo17/32 bước và MHP conditional;12 bộ fixture so trực tiếp với thân InitHackCheck source server, không đọc config/serial thật.
- DONE parser: greeting14 byte, result/index/server code/endian/version raw; JS adapter kiểm code đã chọn, không tự coi version là hợp lệ.
- Kiểm thử: build:wasm qua; npm test **16/16**, test:wasm/source-reference/greeting qua, không skip. Đây là numeric/packet fixture, chưa game connection thực.
- Chưa hoàn tất: profile provisioning/encoding ngoài printable ASCII, custom CustomerName, session GameServer probe, ClientVerify, SimpleModulus/XOR/C3-C4/serial/login. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không đổi native production/Data/config. Debug/Release giữ layout; ping chưa nối server.

Bước sau: probe GameServer greeting qua gateway với transport cipher và fixture mã hóa; chưa tài khoản thật.

## Đợt W0016 — 2026-10-23

Đã thêm probe GameServer greeting qua transport cipher/derive keys/gateway. [Phạm vi và bằng chứng](GAME-PROBE.vi.md).

- DONE probe: route kiểm chứng lại, explicit profile, decode trước framing, greeting/code kiểm tra, deadline/AbortSignal/EOF và cleanup; trả metadata rồi đóng, không gửi application/login/ClientVerify byte.
- Test frontend/core **20/20**, gateway thực loopback **12/12**, test:wasm/source-reference qua, không skip. Network fixture MHP off/on, sai result/code/key và silent server; không phải server MU thật.
- Ca wrong-key có thể tạo header trông hợp lệ, cần deadline. Test clock mô phỏng đã sửa để kiểm nhánh này; process test cũ chờ clock đã được dừng.
- TODO: profile provisioning thật, ClientVerify/identity, packet crypto/serial/login và browser game probe. M02/M03 vẫn PARTIAL. Không coi greeting accepted là authentication/version match.
- UI Debug/Release giữ nguyên, ping chưa MU; MU server/DB không bật, Data/config/native production không thay đổi. Fixture đóng sau test.

Bước sau: đối chiếu ClientVerify/HWID source và thiết kế identity browser, không bypass server checks.

## Đợt W0017 — 2026-10-23

Đã audit luồng ClientVerify/HWID source và bổ sung codec identity + XOR C1/C2. [Chi tiết/bằng chứng/phần thiếu](CLIENT-IDENTITY.vi.md).

- DONE packet/schema: F3:0949 byte, format44+NUL theo producer Main/native; packet-XOR chain tiến/ngược đúng source, C3/C4 chưa hỗ trợ bị từ chối.
- DONE source fixture: so với thân XorData Main và server + xác nhận table EX603, chạy native/WASM. Không sửa server validator/HID limits/blacklist/verify timeout.
- Test: WASM build qua; npm test **20/20**, test:wasm identity/XOR/source-reference qua, không skip. Chỉ identity giả trong test, không gửi network MU hoặc đọc ID máy.
- Identity provider browser còn thiếu: không random/localStorage fallback. Provider native phù hợp OS là hướng đề xuất, chưa implement/trust/provisioning. Không gọi send thành công là verified vì source không có ACK riêng.
- Chưa hoàn thiện: provider/identity runtime, SimpleModulus/serial/login; M02/M03 vẫn PARTIAL. Không bật MU server/DB hoặc đổi Data/config/native production. Debug/Release giữ nguyên.

Bước tiếp theo có thể làm độc lập: tách SimpleModulus và packet serial/key-file parser bằng fixture sạch; chưa mở login khi identity provider chưa có.

## Đợt W0018 — 2026-10-23

Đã tách SimpleModulus, key-file parser và serial vào core C++/WASM. [Nguồn, API, kiểm thử và giới hạn](SIMPLE-MODULUS.vi.md).

- DONE phạm vi core: layout key-file 54 byte; block/checksum/length; C1/C2 XOR outbound, C3/C4 encode/decode và serial gửi/nhận độc lập, lỗi protocol làm instance thất bại.
- Đối chiếu byte với thân hàm Main gốc trên native/WASM; fixture tổng hợp, không đọc/copy Enc1/Dec2 hoặc identity thật. Giữ DWORD wrap và tránh đọc vượt block cuối.
- Kiểm thử: build:wasm qua, npm test **21/21**, test:wasm qua, không skip. Debug/Release kiểm allowlist và cú pháp module xuất ra. Chưa network MU hoặc benchmark gameplay.
- TODO: provisioning key/profile, ghép packet cipher vào GameServer session, identity provider và login. M02/M03 vẫn PARTIAL; UI chưa thay đổi, ping chưa nối MU.
- Không bật MU server/DB, không đổi Data/config/native production. Bước sau là pipeline fixture theo đúng thứ tự transport/framing/packet crypto.

## Đợt W0019 — 2026-10-23

Đã ghép pipeline packet GameServer trong adapter riêng. [Phạm vi và phần thiếu](GAME-CHANNEL.vi.md).

- DONE phạm vi adapter: route/profile explicit; gửi packet crypto → transport, nhận transport → framing → packet crypto; cipher/reader riêng từng phiên và cleanup khi đóng/lỗi.
- Test **25/25** và test:wasm qua, không skip. Ca chia frame, coalescing, replay, outbound lỗi, callback lỗi, key sai, timeout và phiên độc lập; WASM thật/socket mô phỏng. Chưa network MU/gateway cho channel mới.
- Debug/Release build allowlist và cú pháp qua; UI chưa nối channel nên không thay đổi. Không bật MU server/DB, không đổi native production/Data/config.
- TODO: phase greeting/ClientVerify/login, identity provider và key/profile provisioning, browser/network interoperability. M02/M03 vẫn PARTIAL; WebSocket open không phải authentication.

## Đợt W0020 — 2026-10-23

Đã thêm state machine greeting → identity provider → gửi F3:09, theo GCConnectClientRecv source. [Chi tiết](GAME-HANDSHAKE.vi.md).

- DONE phạm vi adapter: deadline từng phase, AbortSignal, không gọi provider trước greeting hợp lệ, không gửi sau timeout/hủy; giữ channel sau identity-sent. Không đánh dấu verified vì source không có ACK riêng.
- Kiểm thử **30/30** và test:wasm qua, không skip; WASM thật/socket mô phỏng. Debug/Release allowlist/cú pháp qua. Không có identity/account thật hoặc network MU.
- TODO: provider/provisioning thật, version compatibility policy, login codec/state machine/UI, kiểm chứng chính sách packet đến sớm với MU thật. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không đổi Data/config/native production; UI chưa thay đổi. Bước sau: đối chiếu/tách login packet và response bằng fixture sạch.

## Đợt W0021 — 2026-10-23

Đã tách codec đăng nhập EX603 vào core/WASM. [Source, ABI và phần thiếu](ACCOUNT-CODEC.vi.md).

- DONE phạm vi codec: C3 F1:01 dài 59 byte, credential-XOR/padding/tick LE/version/serial; parser C1 F1:01 giữ raw result, chỉ mã 1 accepted. JS kiểm input trước Embind coercion.
- Đối chiếu struct packed từ server với define 603 và hàm PacketArgumentDecrypt source trên native/WASM. Chú ý password field 20 byte nhưng server hiện đọc 10; giữ giới hạn native, không cắt input âm thầm.
- Build WASM, **32/32 test** và test:wasm qua, không skip; Debug/Release build allowlist/cú pháp qua. Dữ liệu/key tổng hợp, chưa login MU thật.
- TODO: login state machine/response deadline/retry policy, UI, language messages, provider/provisioning thật. M02/M03 vẫn PARTIAL; UI chưa thay đổi.
- Không bật server/DB hoặc sửa native production/Data/config. Bước sau: session đăng nhập có timeout/cancel/chặn gửi trùng bằng fixture.

## Đợt W0022 — 2026-10-23

Đã ghép session đăng nhập sau identity-sent. [API/kiểm chứng/giới hạn](ACCOUNT-SESSION.vi.md).

- DONE phạm vi session: chặn gửi trùng, timeout/cancel/cleanup, kết quả raw; chỉ code 1 authenticated. Input sai chưa gửi cho sửa lại; rejection sau gửi đóng phiên, không tự retry.
- Kiểm thử **36/36** và test:wasm qua, không skip. WASM thật/socket mô phỏng, tài khoản/key giả; success/coalesced packet, rejection, timeout, abort, remote close, malformed response, callback/queue lỗi.
- Debug/Release allowlist/cú pháp qua; UI chưa thay đổi. Không bật server/DB, không dùng credential thật hoặc sửa native production/Data/config.
- TODO: provider/provisioning, MU/gateway interoperability, notice dispatcher, retry/UI Language và danh sách nhân vật. M02/M03 vẫn PARTIAL.
- Bước sau: request/response danh sách nhân vật theo ABI source EX603.

## Đợt W0023 — 2026-10-23

Đã tách request/parser danh sách nhân vật và policy tạo/xóa EX603. [Source/ABI/giới hạn](CHARACTER-CODEC.vi.md).

- DONE codec: request F3:00; list header8/dòng34, 0–5 nhân vật, slot/name/level/control/guild và charset18 byte; policy EXTRA1 DE:00 dài14 byte. Kiểm padding/endian/count/duplicate; charset sở hữu bản sao riêng.
- Source-reference trích struct server với define603/EXTRA1 trên native/WASM. Build WASM, **38/38 test**, test:wasm và targeted malformed JS test qua, không skip. Debug/Release allowlist/cú pháp qua.
- TODO: lobby session chờ list+policy, decode charset/model/effect, UI chọn/tạo/xóa, Language và MU network. M02/M03 vẫn PARTIAL; UI chưa thay đổi.
- Không bật server/DB hoặc đổi Data/config/native production. Bước sau: ghép list/policy vào lobby có timeout/cancel.

## Đợt W0024 — 2026-10-23

Đã ghép session lấy list+policy sau đăng nhập. [API/kiểm thử/giới hạn](LOBBY-SESSION.vi.md).

- DONE phạm vi adapter: chỉ gửi khi authenticated, chặn request trùng, chờ đúng list rồi policy, một deadline cho cả pair; hủy/lỗi đóng phiên, refresh tuần tự được hỗ trợ.
- Kiểm thử **42/42** và test:wasm qua, không skip. WASM thật/socket mô phỏng; đủ chuỗi identity/login/list, coalescing, response sai/thứ tự sai, timeout/cancel/cleanup và phiên độc lập. Debug/Release allowlist/cú pháp qua.
- TODO: charset decoder/render/UI chọn-tạo-xóa, notice dispatcher, provider/provisioning và MU network. M02/M03 vẫn PARTIAL; UI chưa thay đổi.
- Không bật MU server/DB hoặc đổi Data/config/native production. Bước sau: charset class/trang bị theo source và fixture regression.

## Đợt W0025 — 2026-10-23

Đã giải mã CharSet class/tier và bảy slot vũ khí/giáp. [Source, phạm vi và giới hạn](APPEARANCE.vi.md).

- DONE codec: slot/index/present, visual grade và boolean excellent/ancient; giữ raw18 byte. Không suy ra exact level/options từ dữ liệu nén.
- Reference trích packing equipment từ DSProtocol.cpp; sweep mọi weapon12-bit/armor9-bit và level0–15 trên native/WASM. Test phát hiện source bỏ level/flag cho weapon511; kỳ vọng fixture đã sửa đúng source.
- Build WASM, **44/44 test** và test:wasm qua, không skip. Debug/Release allowlist/cú pháp qua. Chưa test hình ảnh/renderer hoặc MU network.
- TODO: wing/pet/custom decoding, model mapping và scene/effect/UI; provider/provisioning/network và hiệu năng game. M02/M03 vẫn PARTIAL, UI chưa thay đổi.
- Không bật server/DB, không đổi Data/config/native production. Bước sau: wing/pet/catalog requirements theo source.

## Đợt W0026 — 2026-10-23

Đã thêm decoder wing/pet và phân biệt builtin/custom/unknown/ambiguous. [Nguồn và giới hạn](COMPANIONS.vi.md).

- DONE phạm vi nhánh builtin khảo sát và Fenrir variant; custom trả catalog index, không tự chọn item/model. Giữ raw18 byte. Một số mã cánh builtin/custom trùng bit được đánh dấu mơ hồ.
- So trực tiếp source packing wing/pet bằng fixture catalog giả, cross-product nhánh builtin/pet/variant trên native/WASM. Build WASM, **46/46 test** và test:wasm qua, không skip. Debug/Release allowlist/cú pháp qua.
- TODO: custom catalog thật/index collision, resource mapping, animation/bone/effect/render/UI; provider/provisioning và MU/browser network. M02/M03 vẫn PARTIAL; UI chưa thay đổi.
- Không bật MU server/DB hoặc đổi Data/config/native production. Bước sau: audit custom catalog và item→resource theo source/Data.

## Đợt W0027 — 2026-10-23

Đã audit catalog custom text và quy tắc resource theo Main/source. [Bằng chứng/giới hạn](CUSTOM-CATALOG.vi.md).

- DONE công cụ offline: schema text, directory mapping Item/Player, exact/case/missing asset status, duplicate/bounds/path validation và collision evidence; 3/3 unit test qua, audit Data/catalog thực chạy thành công.
- Kết quả: serverWing0, clientWing0, clientItem0 active entry; ví dụ comment không được load. Potential custom indices15/31/47 trùng builtin wings130/131/132, active collision0. Chưa kiểm binary ServerInfo.sse catalog.
- Không đổi frontend/core; không chạy lại bộ46/46 W0026 trong đợt audit này. UI chưa đổi. Không bật server/DB hoặc đổi Data/config/native production.
- TODO: builtin item→resource mapping, compiled catalog/runtime provisioning, model/texture/render/UI và MU network. M02/M03 vẫn PARTIAL.
- Bước sau: builtin equipment resource mapping đối chiếu native/Data, không đoán asset.

## Đợt W0028 — 2026-10-23

Đã port item model-path rules từ adapter native vào core/WASM và audit file Data. [Nguồn/coverage/giới hạn](ITEM-RESOURCES.vi.md).

- DONE trong phạm vi native mapping: relative path + unmapped explicit; host reference sweep8192 ID ×4 levels, WASM/bridge checks. Build WASM, **48/48 test** và test:wasm qua, không skip; Debug/Release allowlist/cú pháp qua.
- Audit thực: 1142 mapped cases,285 unique paths;1098 exact/44 case-mismatch,0 missing trong mapped domain. Có11 unique path lệch hoa/thường; không rename Data hoặc claim toàn Item.bmd đã map đủ.
- TODO: exact-case manifest resolver, body armor class/tier, mapping còn thiếu, texture/geometry/render/UI và MU network/provisioning. M02/M03 vẫn PARTIAL; UI chưa đổi.
- Không bật server/DB hoặc đổi native production/Data/config. Bước sau: resource manifest/collision validation trước HTTP asset load.

## Đợt W0029 — 2026-10-23

Đã tạo manifest model đúng case/hash và JS resolver. [API/bằng chứng/giới hạn](RESOURCE-MANIFEST.vi.md).

- DONE phạm vi manifest:285 models,5.399.772 byte,11 case corrections; không rename/copy Data. Generator kiểm path/root/size/hash, resolver snapshot/allowlist/collision; manifest thực validate/resolve đủ285 entry.
- 2/2 Python test và **50/50 npm test** qua, không skip; Debug/Release allowlist/cú pháp qua. Không đổi C++/WASM, không chạy lại test:wasm riêng đợt này.
- Làm rõ: web hiện chỉ canvas kiểm tra + Debug overlay, CHƯA có giao diện game. Không có scene/UI game để nghiệm thu hình ảnh.
- TODO: HTTP staging/fetch loader/hash byte verification/abort/cache, decoder/render/UI, provisioning và MU network. Manifest version/hash format chưa phải trust/signature verification. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc đổi Data/config/native production. Bước sau: loader asset giới hạn byte + hash/abort bằng fixture.

## Đợt W0030 — 2026-10-23

Đã thêm loader asset stream theo manifest có size cap/hash/abort/deadline. [API và giới hạn](RESOURCE-LOADER.vi.md).

- DONE phạm vi loader: trả bytes chỉ sau size+SHA256 đúng; default2 concurrent/64MiB output reservation, không cache tự động; tiến trình byte thực, timeout bao fetch/read/digest và cleanup khi lỗi/hủy.
- **54/54 npm test** qua, không skip;4 loader fixture mới dùng Response/ReadableStream/WebCrypto thật trong Node. Debug/Release allowlist/cú pháp qua. Không đổi C++/WASM, không chạy lại test:wasm riêng.
- Chưa HTTP/browser load model thật, staging route, parser/render/UI hoặc MU network/provisioning. Web chỉ canvas kỹ thuật, chưa GUI game. M02/M03 vẫn PARTIAL.
- Không bật server/DB hoặc đổi Data/config/native production. Bước sau: portable BMD parser có bounds validation và kiểm asset trước WebGL upload.

## Đợt W0031 — 2026-10-23

Đã thêm bộ đọc model BMD trực tiếp C++/WASM từ parser native, không chuyển đổi Data. [Nguồn/API/giới hạn](BMD-MODEL.vi.md).

- DONE parser trong phạm vi: v10/v12, mesh/UV/normal/face/bone/action, numeric/index/hierarchy/EOF bounds và allocation caps. Bridge hiện chỉ xuất metadata, chưa GPU buffers.
- Build WASM, **56/56 test** và test:wasm qua, không skip. Fixture encrypted/plain + malformed so native parser; ba model gốc Sword01/Wing01/HelmMale01 v10 qua host/native comparison và WASM Node inspection.
- TODO: full animation/root motion/geometry export, texture# W0033 — Texture gốc OZT/OZJ và dependency mapping

## Source / implementation

Native/client/assets.cpp decode_tga + jpeg_payload và native/client/asset_images.mm resolve_model_texture/decode_texture là nguồn adapter hiện có. Model texture reference .jpg→.OZJ, .tga→.OZT trong cùng directory; không tìm file cùng tên ở folder khác để che missing. JS textureReference giới hạn model Item/Player và filename an toàn; chưa nested/custom texture path.

Core `mu/texture.hpp`: OZT uncompressed truecolor type2,24/32bit BGR(A)→RGBA, TGA imageID và top/bottom,left/right origin theo native. Giữ alpha32bit và alpha255 cho24bit. Input<=16MiB, dimensions1..2048; native cho4096, portable chủ động giới hạn decodedRGBA<=16MiB. Không hỗ trợ RLE/palette/OZB hoặc tự chuyển format khi không đọc được.

OZJ bỏ wrapper24 byte như native, kiểm SOI/EOI và JPEG segment bounds/SOF dimensions trước khi gọi decoder browser. Hỗ trợ SOF baseline/progressive8bit1 hoặc3 components, không CMYK/arithmetic/lossless. Đây không phải decoder entropy JPEG C++: WASM trả JPEG payload nguyên, browser createImageBitmap giải mã trực tiếp qua Blob(image/jpeg). Không sinh file JPEG/glTF bên ngoài hoặc sửa Data.

Bridge `textureResult` checked-error/type/cap và copies bytes. inspectTexture cho metadata/payload; decodeOriginalTexture trả RGBA owned hoặc bitmap có dispose idempotent. JPEG dùng premultiplyAlpha:none,colorSpaceConversion:none,imageOrientation:none; kiểm kích thước bitmap so metadata. Abort trước decode không chạy; nếu abort khi createImageBitmap đang chạy, bitmap trả muộn được close rồi reject (chưa cancel ngay công việc decoder nội bộ/không deadline riêng). Chưa kiểm màu/EXIF/orientation trên browser thực.

Manifest generator/resolver W0029 được mở rộng cho suffix bmd/OZJ/OZT, còn giữ directory allowlist/case identity/hash/collision guards. Không dùng hình thay thế khi thiếu texture.

## Data thực và kiểm thử 23/10/2026

| Texture Data | Dimensions | Bytes |
|---|---:|---:|
| Item/sword02.OZJ |64×16|6316|
| Item/elfin_wing.OZJ |64×64|9682|
| Player/head_02.OZJ |64×64|12284|
| Player/hide.OZJ |9×9|4556|

Bốn wrapper thật qua WASM Node inspection. Manifest4 file,32838byte,0 case correction, SHA256 từngfile được tạo cục bộ tại `web/.local-baselines/2026-10-23-w0033/texture-manifest.json`; metadata tại textures.json. Đây chưa phải pixel decode browser hoặc asset HTTP load. hide9×9 không bị thay bằng ảnh khác.

Build WASM,61/61 npm test, test:wasm và2/2 Python manifest tests qua, không skip. OZT reference pixel so nguyên decode_tga native trên24/32bit và4origin; contract cũng chạy WASM. JS-WASM kiểm wrapper dimensions/length, RGBA alpha/channel; JPEG browser adapter dùng fake createBitmap để kiểm options/dimensions/disposal/abort, không claim JPEG pixels thật đã được kiểm. Debug/Release allowlist/cú pháp qua.

## TODO / bước tiếp theo

- HTTP staging và browser createImageBitmap thực của texture gốc; WebGL UV/row orientation/color parity/image regression chưa có.
- Renderer shader/material/blend/alpha-cutout/light, VBO/VAO/texture upload/context loss/disposal/camera và preview model đầu tiên.
- Chưa decode toàn Data, RLE/OZB, custom/nestedtexture policy, resource cache/scheduler/performance.
- Animation/effects/body class-tiers/GUI game và MU network/provisioning vẫn thiếu; M02/M03 PARTIAL.
- Không bật MU server/DB, không đổi native production/Data/config. Trang hiện chỉ canvas kiểm tra + overlay, chưa GUI game.

Bước sau: staging bộ3BMD+4texture và manifest cục bộ có phạm vi rõ, ghép WebGL preview textured model để kiểm ảnh browser. Không gắn nhãn preview là màn chọn nhân vật/gameplay đã hoàn thiện.
 decoder, WebGL render/GUI và browser/network asset load; chưa sweep toàn model hoặc FPS game. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không sửa native production/Data/config. Bước sau: geometry/pose buffers rồi texture decoder để vẽ model đầu tiên.

## Đợt W0032 — 2026-10-23

Đã xuất BMD geometry Float32Array ở initial pose cho WebGL. [Nguồn, test và giới hạn](BMD-GEOMETRY.vi.md).

- DONE phạm vi buffer: positions/normals/UV theo triangle corners, bone parent transforms, normal rotation/normalize, copy ownership và output cap; chưa GPU upload/render hoặc animation playback.
- Build WASM, **57/57 npm test** và test:wasm qua, không skip; host pose đối chiếu native và synthetic rotation/normal/ownership tests. Ba model thật Sword01/Wing01/HelmMale01 xuất geometry bằng WASM Node, finite/length checks qua.
- TODO: texture exact-case resolver/decoder, WebGL preview/lifecycle/camera, animation/effects/UI và MU network/provisioning/FPS game. M02/M03 vẫn PARTIAL; web chỉ canvas kỹ thuật.
- Không bật server/DB hoặc đổi Data/config/native production. Bước sau: texture gốc và preview model trên canvas.

## Đợt W0033 — 2026-10-23

Đã thêm texture OZT decoder/OZJ wrapper parser và browser JPEG adapter. [Nguồn/phạm vi/test](TEXTURES.vi.md).

- DONE core: OZT24/32bit RGBA/alpha/4origin; OZJ24-byte wrapper + bounded JPEG dimensions/segment inspection. JPEG pixels do createImageBitmap browser, chưa kiểm pixel browser thật.
- Manifest/resolver nhận bmd/OZJ/OZT;4texture gốc của3model thử qua WASM Node inspection, manifest32838byte có hash,0case correction. Không copy/chuyển đổi Data gốc.
- Build WASM, **61/61 npm test**, test:wasm và2/2 Python manifest tests qua, không skip. OZT so pixel native; JPEG adapter lifecycle test dùng mock bitmap. Debug/Release allowlist/cú pháp qua.
- TODO: browser JPEG decode/HTTP staging và WebGL preview/UV/color/blend verification, animation/GUI và MU network/provisioning. M02/M03 vẫn PARTIAL; canvas chưa vẽ model.
- Không bật MU server/DB hoặc sửa native production/Data/config. Bước sau: bounded local staging + textured model preview trên browser.

## Đợt W0034 — 2026-10-24

Đã có preview WebGL2 textured3model trong browser. [Cách chạy, bằng chứng và sai khác](MODEL-PREVIEW.vi.md).

- DONE technical preview: Debug query preview=models, staging7asset gốc có manifest/hash, WASM BMD geometry và JPEG browser decode thật, GPU upload/draw/dispose; Release không chứa preview. Quyền đọc Downloads đã thông.
- **62/62 npm test** qua, không skip; GPU lifetime/bounds/upload failure unit test, Debug/Release allowlist/cú pháp qua. IAB screenshot buffer2560×1440 quan sát60FPS cho3model nhỏ, không claim gameplay60FPS.
- PARTIAL visual: cánh còn nền đen do material chưa có alpha/source render pass; mũ rời cần bind Player skeleton, shader/camera chưa parity. Chưa GUI game/scene nhân vật hoặc MU online.
- HTTP static loopback8080 phục vụ web/dist cho preview, không bật MU server/DB; không đổi native production/Data/config. Bước sau: material alpha và Player bone binding theo source để sửa lỗi nhìn thấy.

## Đợt W0035 — 2026-10-24

Đã thêm Player skeleton binding và đối chiếu material native. [Source/test/sai khác còn lại](PLAYER-BINDING.vi.md).

- DONE numeric binding: name/parent/index checks, action/frame pose, JS-WASM export; HelmMale01 corner positions so native Player clip1/frame0 qua. Debug staging8asset, thêm Player.bmd.
- Material preview bỏ lighting giả và dùng Repeat/alpha discard theo native. Wing JPEG vẫn opaque, chưa có source rule để xóa nền đen; không tự dùng RGB threshold.
- Build WASM, **63/63 npm test** và test:wasm qua, không skip. Browser textured preview Player bind có screenshot, chưa visual parity; mũ rời còn cần body/visibility/UV verification.
- TODO: full-body context/material hide/UV và wing render pass, animation/GUI/MU network/provisioning. Không bật MU server/DB hoặc đổi Data/config/native production. M02/M03 vẫn PARTIAL.

## Đợt W0036 — 2026-10-24

Đã ghép fullbody5part và xác định/sửa OZJ row orientation bằng native pixel reference. [Bằng chứng/test/phần thiếu](BODY-PREVIEW.vi.md).

- DONE preview: DW base5part chung Player pose, testhelmet cùng thân, staging14asset; cache decoded image/asset bytes trong startup và reuse GPU texture. Không phải gameplay equip validation.
- JPEG browser sửa imageOrientation:flipY sau khi so native RGBA từngpixel: mean RGB error83.5277→0.0613/255 cho skin256×256. OZT không đổi. Browser base/helm screenshot xác nhận texture/body đã rõ đúng bố cục hơn.
- **65/65 npm test** qua, không skip;5part pose/order/shared texture lifecycle, Debug/Release allowlist/cú pháp. Không đổi core/WASM đợt này, không test:wasm riêng.
- TODO: wing nền đen/material hide/Windows visual parity, class/tier mapping, animation/effects/equip/GUI/MU network/provisioning. M02/M03 vẫn PARTIAL;60FPS snapshot preview không bảo đảm gameplay.
- Không bật MU server/DB hoặc đổi native production/Data/config. Bước sau: body class/tier mapping và class preview, wing material tiếp tục điều tra.

## Đợt W0037 — 2026-10-24

Mở rộng thân cơ bản đủ 7 class và 3 tier. [Chi tiết DONE/PARTIAL/TODO](BODY-CLASSES.vi.md).

- DONE: 105 đường dẫn so với hàm native biên dịch; pose đứng riêng Elf/Summoner; 21/21 tổ hợp ghép BMD/Player/texture qua audit, staging Debug123asset.
- **67/67 npm test** qua. Browser Elf/Summoner có screenshot. Release tiếp tục không chứa preview/diagnostics.
- PARTIAL: chỉ base body type511; chưa port đầy đủ resolver armor/equipment, animation/weapon/effects/GUI/MU network. Wing/hide và RF idle/Windows parity chưa xong; không hứa gameplay60FPS từ preview.
- Không đổi Data/native production/config; MU server/DB không bật. Bước sau: resolver trang bị/bone attachment và material visibility.

## Đợt W0038 — 2026-10-24

[Resolver giáp theo class và giới hạn](ARMOR-RESOLVER.vi.md).

- DONE core/WASM resolver theo native, 53.760 tổ hợp so đường dẫn/lỗi; 5 fixture giáp thêm vào Debug, Data audit26/26, staging162asset. Giữ nguyên case extension texture gốc.
- **71/71 npm test**, test:wasm và 2 Python manifest test qua. Browser DW/Summoner hiển thị giáp.
- PARTIAL: Summoner thiếu đầu dưới mũ, wing nền đen/hide visibility; chưa skin bổ sung/weapon attachment/animation/effects/equip online/GUI. Mapping pass không đồng nghĩa visual parity. M02/M03 vẫn PARTIAL.
- Không đổi Data/native production/config; MU server/DB vẫn không bật. Tiếp theo: skin bổ sung/visibility rồi bone attachment.

## Đợt W0039 — 2026-10-24

[Ghép trang bị riêng từng slot](MIXED-ARMOR.vi.md).

- DONE: base/equipped dùng chung resolver WASM, bỏ mapping JS trùng lặp;5slot hỗn hợp, type511 giữ thân class, parser/query và input ownership kiểm tra.
- Audit31/31 tổ hợp, staging162asset giữ nguyên; **73/73 npm test** qua. Browser Summoner không đội mũ giữ đầu/tóc class, cùng4phần giáp.
- PARTIAL: chưa sửa đầu/skin dưới mũ khi có mũ, wing/hide, weapon/animation/effects/GUI/network. Screenshot FPS55.0, cần profile; không chứng nhận60FPS.
- Không đổi Data/native production/config; MU server/DB không bật. Bước sau: hand/back bone attachment và frametime profiling; M02/M03 vẫn PARTIAL.

## Đợt W0040 — 2026-10-24

[Bone/grip attachment portable](EQUIPMENT-GRIP.vi.md).

- DONE: policy bone và local transform gắn trang bị port sang core/WASM, giữ ngoại lệ hand/back/bow/crossbow/shield/robe; numeric reference49.152tổ hợp với header native Apple SIMD khớp tolerance0.0001.
- **75/75 npm test** qua, không skip; rebuild WASM và test:wasm qua, gồm contract attachment49.152case. Debug/Release build separation qua; stage lại Debug162asset sau test.
- PARTIAL: chưa áp dụng transform vào geometry/pose để hiện vũ khí trên thân; chưa stance/animation. Preview hình ảnh không thay đổi đợt này. Skin dưới mũ/wing/hide/GUI/network/FPS profiling còn thiếu.
- Không sửa Data/native production/config, MU server/DB không bật. M02/M03 vẫn PARTIAL; bước sau ghép geometry bằng pose world × grip × item model.

## Đợt W0041 — 2026-10-24

[Geometry vũ khí gắn Player pose](ATTACHMENT-GEOMETRY.vi.md).

- DONE core/WASM attachment geometry world bone×grip×item, normals xoay riêng, kiểm tra bone/pose/hierarchy; preview Sword01 tay0/1 hoặc đeo lưng safe1.
- **77/77 npm test** qua, rebuild WASM và test:wasm qua, không skip. Native realData comparison12case qua (action1/2/3 ×2hand ×safe); log lưu baselineW0041.
- Browser DW cầm tay và đeo lưng đã render Sword01 trên thân, screenshot right-hand.png/back.png. Staging162asset, không tăng Data. FPS60snapshot không chứng nhận hiệu năng dài.
- PARTIAL: chưa stance theo weapon, animation/online equip, bow/shield/wing geometry, skin dưới mũ/hide, Windows visual parity/gamecamera/GUI/network/perf profile. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc sửa native production/Data/config. Bước sau stance metadata/action rồi animation và frametime profiling.

## Đợt W0042 — 2026-10-24

[Tư thế vũ khí và Item.bmd](EQUIPMENT-ACTIONS.vi.md).

- DONE policy idle/walk/run portable,573.510tổ hợp so trực tiếp hàm native; reader hẹp twoHand Item.bmd kiểm checksum gốc,5index realData so decoder native qua. Sword01 single-hand nên idle4.
- Thân/weapon dùng cùng action, safe/back giữ base idle. Browser DW cầm kiếm action4 có screenshot sword-stance.png; tab giữ preview này. Direct Item_eng.bmd staged nguyên file, tổng163asset; manifest chỉ cho đúng path này ngoài Item/Player.
- **80/80 npm test**, rebuild WASM, test:wasm và2Python manifest test qua, không skip. Checksum native là kiểm định layout, không phải cryptographic; manifest SHA256 vẫn kiểm bytes.
- PARTIAL: vẫn frame0, chưa animation/walk/run gameplay/attack/online equip; chưa full ItemCatalog hoặc locale selection, skin dưới mũ/wing/GUI/network/gamecamera/FPS dài còn thiếu. Snapshot60FPS không chứng nhận60FPS gameplay. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc đổi Data/native production/config. Bước sau animation clip/thân-weapon đồng bộ và đo chi phí cập nhật.

## Đợt W0043 — 2026-10-24

[Idle animation và CPU profiling](BODY-ANIMATION.vi.md).

- DONE quaternion interpolation theo native, thời gian độc lập FPS; body+weapon cùng pose.57.600bone samples trên timeline120Hz so native qua. Cached BMD/mesh/binding, update vertex buffers giữGPU resources.
- **83/83 npm test**, rebuild WASM và test:wasm qua, không skip. Contract animation + GPU lifecycle/topology qua. Staging163asset giữ nguyên; MU server/DB không bật.
- Benchmark sau khi build/test dừng:600samples DW+Sword01,2124vertex, CPU sample/copy p50=0.3593ms,p95=0.3890ms,max=0.7619ms. Không gồmWebGL/GPU/compositor. Script và JSON tái chạy được.
- Browser animation hiển thị ~60.0–60.1FPS; CPU animation/upload/draw submission p95 tại2lần quan sát0.90ms và1.60ms. Screenshot/AX lưu baselineW0043. Không xác nhận120FPS, không coi120Hz reference là120FPS. Cần profile dài và GPU/hardware refresh cho kết luận120.
- PARTIAL CPU skinning/copy allocation còn, chưa GPU skinning/pooled output; idle-only, chưa walk/run/attack/transitionblend. Skin dưới mũ/wing/GUI/network/gamecamera/RF idle vàWindowsparity còn thiếu. M02/M03 vẫn PARTIAL.
- Không sửa Data/native production/config. Bước sau: animation transitions/walk/run và profiling dài trước khi mở rộng scene.

## Đợt W0044 — 2026-10-24

[Idle/walk/run và chuyển tư thế](MOTION-ANIMATION.vi.md).

- DONE: tốc độ clip theo native world, blend 0,12 giây, giữ phase đi ↔ chạy, reset khi về đứng và chuẩn hóa root X/Y. Thân/vũ khí cùng pose; demo `motion=cycle` chạy tại chỗ, chưa di chuyển qua network.
- **84/84 npm test**, rebuild WASM và test:wasm qua, không skip. Test chuyển tư thế, root và clock 60/120 Hz đạt. Staging giữ nguyên 163 asset.
- Benchmark CPU sau build/test: 6.000 mẫu, 2.124 đỉnh, timeline 120 Hz; p50 = 0,3689 ms, p95 = 0,4291 ms, max = 0,8749 ms. Không gồm WebGL/GPU/compositor; không phải FPS thời gian thực.
- Browser sau nhiều chu kỳ, cửa sổ 3.600 rAF interval: FPS 60,0; CPU frame p95 = 1,70 ms; interval p95 = 17,70 ms. Lần trước ở 2.749 mẫu: CPU 1,50 ms và interval 18,60 ms. Interval vẫn vượt ngân sách 16,67 ms nên chưa xác nhận 60 FPS ổn định hoặc 120 FPS. CPU frame không gồm sort metrics, DOM update, GPU/compositor; chưa đủ căn cứ kết luận nguyên nhân giật.
- PARTIAL: chưa map movement/input/run policy/zone/attack/cast/equipment transition/GUI/network; movementRate = 1 trong fixture. Skin, wing, GPU skinning, pooled output, camera game, RF idle và Windows parity còn thiếu. M02/M03 vẫn PARTIAL.
- Không sửa Data/native production/config; MU server/DB không bật. Bước sau: visual fixture bow/staff/shield và profile khi tăng scene; giữ toàn bộ lỗi còn mở trong TODO.

## Đợt W0045 — 2026-10-24

[Cung Elf, staff và khiên DW](WEAPON-LOADOUTS.vi.md).

- DONE: fixture loadout tập trung, path từ core resolver; hai attachment cùng Player pose, kiểm slot trùng và giới hạn3món. Elf cung/tên, DW staff/khiên đã xem browser qua cycle idle/walk/run.
- **85/85 npm test**, rebuild WASM và test:wasm qua. Native geometry24case cho4model mới và12case Sword01 qua. Audit33cấu hình và172asset staging qua; không xuất cảData.
- CPU benchmark6000mẫu/loadout: bow p95=0,3266ms,max=1,2033ms; staff-shield p95=0,3144ms,max=1,5163ms. Không gồmGPU/WebGL/compositor. Browser bow60FPS/intervalp95=17,60ms; staff-shield60FPS/intervalp95=17,20ms. Chưa xác nhận60FPS ổn định hoặc120FPS.
- PARTIAL: fixture render, chưa equip online/requirements/attack/cast/projectile/effect. Skin dưới mũ/wing/hide/GPUskin/cameraGUI/network/RFidle vàWindowsparity còn thiếu. M02/M03 vẫn PARTIAL.
- Không bậtMUserver/DB hoặc sửaData/native production/config. Bước sau attack/cast theo source, sau đó projectile/effect/skill data; giữ các lỗi mở trong TODO.

## Đợt W0046 — 2026-10-25

[Chọn clip đánh và chuyển tư thế](ATTACK-ANIMATION.vi.md).

- DONE: attack selector theo item/hand/twoHand, so trực tiếp nhánh native cùng 573.510 tổ hợp; thêm mode đánh với nhịp native world 0,6 giây và blend 0,12 giây. Body và attachment cùng pose.
- **86/86 npm test**, rebuild WASM và test:wasm qua. Geometry reference 32 trường hợp bow/arrows/staff/shield (gồm attack) và 12 trường hợp Sword01 qua. Audit loadout thử cả attack → idle; staging giữ 172 asset.
- CPU benchmark 6.000 mẫu/loadout: bow p95 = 0,3385 ms, max = 2,2608 ms; staff-shield p95 = 0,3959 ms, max = 1,3457 ms. Không gồm WebGL/GPU/compositor. Browser Elf đánh cung: 60 FPS, CPU frame p95 = 1,90 ms, interval p95 = 17,20 ms ở 1.388 mẫu; chưa chứng nhận 60 FPS ổn định hoặc 120 FPS.
- Overlay Debug phân biệt clip idle/walk/run và attack, tránh nhầm action idle với mode đang phát. Sau chỉnh chữ overlay đã build Debug, kiểm cú pháp và stage lại; logic core không đổi.
- PARTIAL: chưa cast/skill/projectile/effect/hit/damage/server, chưa attack speed theo chỉ số. Fixture đánh lặp không phải auto-attack gameplay. Skin/wing/GPU skinning/GUI/network/RF idle và Windows parity còn mở. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc sửa Data/native production/config. Bước sau: cast selector và nguồn effect/skill, rồi mới nối projectile/hit/server authority.

Browser DW sau reload bản overlay mới: clips4/17/26, attack39, motion=combat; 60 FPS, CPU frame p95 1,60 ms, interval p95 17,60 ms ở 3.600 mẫu. Tab giữ bài thử DW; screenshot cuối ở baseline W0046.

## Đợt W0047 — 2026-10-25

[Cast animation và audit nguồn hiệu ứng](CAST-ANIMATION.vi.md).

- DONE: cast147 rồi146 luân phiên theo native, nhịp0,6 giây và blend0,12 giây; thân/trang bị cùng pose. Preload clip một lần, root X/Y chuẩn hóa theo idle class. Demo có khoảng nghỉ1,4 giây riêng để quan sát, không phải skill cooldown.
- **87/87 npm test**, rebuild WASM và test:wasm qua. Native pose reference mở rộng146/147:86.400bone samples qua. Audit loadout thử cả hai clip; staging giữ172asset.
- Audit chỉ đọc8file nguồn effect từ native (Fire01/Laser01 và texture tham chiếu, JointSpirit01/Thunder01/Spark03); có exact-case/hash/mesh references, không lỗi. Chưa stage effect hoặc xác nhận material/quỹ đạo/render parity.
- CPU benchmark6000mẫu staff/khiên cast: p50=0,2526ms,p95=0,3300ms,max=4,7053ms; không gồmWebGL/GPU/compositor. Browser3600intervals:60FPS,CPUframep95=1,60ms,rAFintervalp95=17,00ms. Chưa xác nhận60FPS ổn định hoặc120FPS; giữ cả spike trong baseline.
- PARTIAL: chưa per-skill selector/Skill.bmd/requirements/mana/cooldown/effect/sound/projectile/hit/damage/server. Skin dưới mũ, wing/hide, GPU skinning/pooling, gameGUI/network/RFidle/Windowsparity còn thiếu. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc sửa Data/native production/config. Bước sau chuyển đúng material/quỹ đạo/thời điểm một effect theo source, rồi mới tích hợp skill packet/hit authority.

## Đợt W0048 — 2026-10-25

[Fire01 fixture và kiểm tra 7 class](SEVEN-CLASS-EFFECTS.vi.md).

- DONE: Fire01 model animation + material opaque/additive theo native, prototype quỹ đạo đến đích giả; camera envelope, effect freeze inspection. Không hit/monster/server. Thêm3asset gốc, staging175file.
- Đã kiểm geometry/idle/walk/run/attack/cast trên đủ7class tier0; mở browser và lưu7ảnh. Không thấy crash/geometry vỡ trong lượt thử, chưa xác nhận full class skills/Windows parity. RF idle fallback vẫn chưa xác minh; Fire01 vệt lớn cần visual reference.
- **90/90 npm test**, rebuild WASM và test:wasm qua; thêm Python test parser/rule SkillList qua. Asset/binding audit và renderer state/lifetime tests qua.
- Ma trận SkillList.txt386mục theo CheckSkillRequireClass đã lưu JSON/hash. Fire Ball qua class filter choDW/MG/Summoner, không choDK/Elf/DL/RF. Các ảnh7class là diagnostic render, không cấp phép skill; bộ skill/effect riêng7class chưa hoàn tất.
- CPU audit từng class lưu p95/max riêng; DL có spike16,175ms cần profile, RF có cảnh báo nguồn idle. Browser7class quan sát60FPS, CPUframep95 từ1,10–2,70ms và intervalp95 từ16,80–17,60ms ở các window khác nhau. Chưa xác nhận60FPS ổn định hoặc120FPS; không che spike.
- PARTIAL: full per-class skill selector/eligibility/catalog/effects, sound/targeting/projectilehit/damage/server; skin dưới mũ/wing/hide/GPUskin/pooling/gameGUI/network còn thiếu. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB hoặc sửa Data/native production/config. Bước sau dùng ma trận skill/class để triển khai từng effect hợp lệ theo nguồn, đồng thời profile DL và xác minh RF idle/Fire visual.

## Đợt W0049 — 2026-10-25

[Menu preview class và skill](PREVIEW-MENU.vi.md).

- DONE: menu Debug chọn 7 class/3 cấp chuyển chức/skill, snapshot SkillList.txt 386 dòng và lọc đúng class/tier; dừng Fire01 để xem. Đổi class xóa trang bị fixture cũ. Release không chứa module menu.
- 92/92 npm tests qua, build Debug/Release và staging 175 asset qua. Browser đã thử đổi DW/DK, skill chưa hỗ trợ, Fire Ball và freeze; ảnh baseline W0049 đã lưu.
- PARTIAL: mới Fire Ball có effect prototype; các skill khác chỉ model idle và ghi rõ chưa có effect. Chưa eligibility đầy đủ/localization/hit/damage/server hoặc parity 100%. M02/M03 vẫn PARTIAL.
- Không sửa Data/native production/config, không bật MU server/DB.

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

Browser W0051: DW tier2 chọn Fire Ball, render tiếp sau 2.409 frame; ảnh ghi 60FPS, CPUframep95 2,10ms, intervalp95 17,20ms. Không dùng mẫu này để khẳng định nhanh hơn baseline khác hoặc đảm bảo60/120FPS. Ảnh/tests lưu baseline W0051.

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

## W0066 — click movement và normal attack (2026-10-26)

Đã có route nội suy liên tục, giữ phím, click đất/A*, safe/armed stance và attachment chuyển động theo vùng, foot run gate, marker model/rings gốc, click đuổi và đánh thường4loại quái có binding. Damage/HP/death/reward/regen theo server; chưa skill/full monsters/mount/buff/hit/death animation/HUD. MuOptions online đi qua cửa đông170,127→184,127 và click Spider nhận “Hạ quái #61:78”, HP65→64, ~60FPS. Marker vàng xác nhận visual tại tab OFFLINE riêng sau sửa Effect texture resolver. Chi tiết và giới hạn: `MOVEMENT-COMBAT-W0066.vi.md`. M07/gameplay vẫn PARTIAL.

## W0067 — bóng đổ world (2026-10-26)

Thêm terrain-projected shadow cho object, nhân vật/trang bị, NPC/quái đang render; GPU dùng cùng position buffers và một height field/map, stencil chống chồng đen, mesh sáng/effect không cast, mái ẩn thì bóng mái tắt.146/146 clienttests pass; Lorencia OFFLINE2.787object khoảng60FPS, đã thấy bóng dáng nhân vật khi di chuyển. Chưa physical shadow-map/tường/tầng/self-shadow, mọi policy cloaking/map hay dedicated Shadow01. Chi tiết: `SHADOWS-W0067.vi.md`.

## W0068 — HUD/Inventory/Character info (2026-10-26)

HUD/panel dùng47resource/bảng gốc đã hash, layout640×480 và190×429; I/V mở túi,C mở thông số,F9Debug. Nối server HP/MP/SD/AG/EXP/level/Zen,base+bonus/combatstats,inventory/durability/update và9structABI sinh từ source. Có icon3D đồ thật,camera inventory,tooltip,cộng điểm chờACK (không test dùng điểm thật); túi chính có C3:24transaction chờACK, kéo hoặc click nhấc/đặt. Ô trang bị vàtúimởrộng hiệnchỉxem; chưafull equipment runtime/quickbar/chat/friends/tooltip/upgrades. Online MuOptionsLv7 nhận HP66,MP766,SD991,AG167,Points9399,bộLegendary+9/Bone+2 đúngpacket.153/153clienttests,16/16gatewaytests pass. Chi tiết/gap/evidence: `HUD-INVENTORY-CHARACTER-W0068.vi.md`.

W0068 online follow-up: MuOptions click nhấc/đặt Kris14→66→14,haiACKserverpass,đãhoàntrảđúngôgốc.Points9399/Zen211.330.843khôngđổi.Ảnhlivebảncuối `/tmp/mu-w68-live-ui-verified.png`.


## W0069 — source mới và hiển thị item

Đã tìm thấy ScriptItem và UI inventory trong Main5.2; sửa góc staff, căn ô trang bị/armor, thêm nền item, nhấc item, hover 3D, màu nâng cấp và Zen. Chưa tìm thấy ItemTRSData.bmd; renderer còn auto-fit, không xác nhận tỷ lệ riêng từng item hoặc parity 100%. Chi tiết và kết quả kiểm tra: [ITEM-VISUALS-W0069.vi.md](ITEM-VISUALS-W0069.vi.md).


## W0070 — tooltip cổ điển và bỏ viền hover

Bỏ viền vàng item; tooltip nền/mép đen, màu từng dòng, chỉ số CalculateItem và requirement; 160 client test qua. Build version hóa module/CSS để tránh mã cũ từ cache. Chưa port đầy đủ các bảng ItemTooltip custom; tỷ lệ vũ khí ngoài map người dùng báo nhỏ vẫn chưa xác định nguyên nhân và chưa sửa bằng hệ số đoán. Xem [ITEM-TOOLTIPS-W0070.vi.md](ITEM-TOOLTIPS-W0070.vi.md).

W0070 bổ sung sau marker: xác định lỗi ở icon ô trang bị, không phải scale ngoài map. Khôi phục góc staff trước W0069 cho Data hiện tại; áp HideSkin của source để Bone Helm không còn đầu người. Xem chi tiết và test asset thật trong tài liệu W0070.


## W0071 — trang bị, shield và thông số

Sửa icon khiên nhìn nghiêng; nút dùng frame nhấn của resource. Thêm tháo/mặc trang bị ô trống theo ACK server, inventory làm nguồn appearance, bổ sung thông số CALC/reset/fruit và trang Chi tiết. 166 test qua; người dùng đang test online. Chi tiết/giới hạn: [EQUIPMENT-CHARACTER-W0071.vi.md](EQUIPMENT-CHARACTER-W0071.vi.md).


## W0072 — HUD căn giữa và đo FPS

HUD tối đa960px/90%viewport, giữ tỷ lệ và căn giữa; FPS/p95/buffer luôn hiện, RTT đo từ ACKGameServer. Sửa mặt trước shield; đủ ba nút inventory, repair≥50 và personal store≥6. Thêm luồng shop Zen cơ bản theo ACK; chưa test giao dịch bán thật. Người dùng xác nhận giao diện ổn; DOM lúc đọc60.0FPS/p9516.9ms/RTT11.3ms. 170 test qua, xem [HUD-DIAGNOSTICS-STORE-W0072.vi.md](HUD-DIAGNOSTICS-STORE-W0072.vi.md) để biết bằng chứng và giới hạn.


W0074 kiểm tra cuối:182client +4helper test qua; debug88/release32. Người dùng xác nhận resource/màu item ổn. Còn hiện dần icon ở lần đầu mở bảng; chưa tối ưu prewarm, đã ghi trong checkpoint.
