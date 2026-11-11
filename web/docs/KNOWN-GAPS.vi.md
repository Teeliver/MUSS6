> W0104: [Tooltip/class/Orb/Life](ITEM-TOOLTIPS-CLASS-W0104.vi.md): đọc ba bảng tooltip Data gốc, sửa Energy/Vitality, class chữ trắng nền đỏ có khoảng cách và đứng trước options; Life giữa ô, số lượng đậm nhẹ. Orb giữ Gem*.bmd, sửa phần phóng theo panel; ngọc giữ kích thước W0103 theo phản hồi. Full 290 test qua; 80 test liên quan sau giới hạn chỉ Orb qua. Còn các status tooltip phụ thuộc hệ thống chưa port và thiếu bảng custom ItemTRSData.

> W0102: [Item inventory](ITEM-INVENTORY-W0102.vi.md): sửa tỷ lệ Orb, dùng bốn model inventory RF và mesh HideSkin theo source, thêm số lượng bình/ngọc nén bằng atlas gốc. Audit 924 item: 902 item/903 variant ready, 22 chưa ánh xạ. 283 test full qua; 59 test liên quan sau chỉnh mặt trước RF qua. Thiếu ItemTRSData.bmd nên chưa chứng nhận tỷ lệ/góc toàn bộ item 100%.

> W0101: [MU Helper](MU-HELPER-W0101.vi.md): hai nút mở cấu hình và bật/dừng theo ACK GameServer; tự đánh, potion HP, nhặt lọc, quay lại điểm và repair cơ bản. 280 test client qua. Party/buff/combo/lệnh pet/offline train và popup skill native chưa hoàn tất; không công bố parity 100%.

> W0100: [Thanh tên map/tọa độ](HERO-POSITION-W0100.vi.md): đúng CNewUIHeroPositionInfo và resource Minimap_position/MacroUI; tên từ Text gốc, cập nhật theo nhân vật. Hai nút là cấu hình và Start/Stop MU Helper; người dùng chọn để hệ thống Helper ở bước sau.39test liên quan qua.

> W0099: [Sửa HP lệch và Unicorn](PET-HP-UNICORN-W0099.vi.md): inset/crop atlas HP đúng NewUICommon; Unicorn dùng đồng hồ riêng, controller25Hz và nội suy pose/frame.37test liên quan qua; chưa chứng nhận mọi tình huống trên phiên thật.

> W0098: [Máu pet và thú cưỡi](PET-MOUNTS-W0098.vi.md): HP helper từ độ bền/255, sửa follow/UV và cập nhật animation mỗi frame; thêm Uniria, Dinorant, Dark Horse, bốn màu Fenrir từ Skill. Audit7model/210mẫu,43cặp class hợp lệ;269test hồi quy qua. Skill/particle/controller đặc thù chưa đầy đủ.

> W0097: [Áo choàng DL/RF](CAPES-DL-RF-W0097.vi.md): sửa bốn cape bằng texture Data gốc và physics cloth theo source, bone19, va chạm bone2/17, tỷ lệ DL .92/RF 1.03. 265/265 test qua; xem main/remote và chạy offline riêng từng cape. Chưa chứng nhận pixel parity hoặc toàn bộ lớp cloth phụ.

> W0095: Đã kiểm tra18 cánh/áo choàng trên20 cặp class đúng Item_eng.bmd. Bốn áo choàng DL/RF đã có physics cloth ở W0097; MG2 thiếu thunder/spirit joints; DK3 thiếu atlas/particle; MG3 thiếu pulse/layer. Xem WINGS-CLASS-AUDIT-W0095.vi.md và JSON cho từng cặp.

> W0093: [Item, cánh, pet và remote player](ITEM-WING-PET-PLAYERS-W0093.vi.md); audit924 item, sửa wing Link=false, nối companion bits. Pet cưỡi/controller đặc thù và tỷ lệ ItemTRSData còn thiếu.

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

# Giới hạn cần giữ mở khi port sang web

Các dòng kế thừa báo cáo/audit native chưa phải lỗi đã tái hiện trên browser. Bản web đã có và đang được kiểm tra; bảng cũ dưới đây là lịch sử, các checkpoint W ở cuối ghi cập nhật mới hơn.

| ID | Vấn đề | Trạng thái / bằng chứng cần bổ sung | Mốc xử lý |
|---|---|---|---|
| G01 | Thiếu full engine Windows EX603 | BLOCKED parity tuyệt đối; cần nguồn engine/executable và bộ cảnh đối chiếu nếu có | M00/M05 |
| G02 | Native không portable nguyên khối | client.cpp còn socket/blocking await, CommonCrypto, hardware UUID; .mm phụ thuộc macOS | M02 |
| G03 | ClientVerify/identity browser | Chưa thiết kế; không disable HackCheck hoặc giả hardware để che thiếu adapter | M02/M03 |
| G04 | Dependency Data chưa audit toàn bộ | 13.660 file có hash; chưa biết toàn bộ model/version/texture/clip lỗi | M00.3b/M04 |
| G05 | Noria | Audit cũ ghi Object44 và hai terrain texture thiếu/fallback; phải tái kiểm tra theo manifest hiện tại | M00.3b/M05/M11 |
| G06 | Elbeland | Audit cũ ghi Object90 bone -8888, Object166 thiếu và texture thiếu; chưa tái xác minh trong đợt này | M00.3b/M11 |
| G07 | Item/tooltip/equip | Harmony/socket/custom item, set-name và mọi loại wing/pet chưa có coverage hoàn chỉnh | M08 |
| G08 | Ground label Option+Space | Người dùng báo chỉ hiện khi giữ Option; native 0.29.7 có sửa state/poll/redraw nhưng phím thật chưa được xác nhận sau sửa. Có khả năng xung đột hotkey OS | M08/M10 |
| G09 | Ground item colors | Native mới có common rules theo reference MuMain; special/event/custom chưa đủ, không khẳng định EX603 parity | M08 |
| G10 | Evil Spirit | Người dùng báo có cast không có kill; native đổi target selection. Ba lần kiểm tra cũ có một lần zero kill; cần phân biệt miss/ngoài tầm/HP với bỏ sót packet, không ép server giết quái | M09 |
| G11 | Movement/server lag | Có báo cáo giật; số ACK native trong một phiên không chứng minh hết lag hoặc web đạt60FPS. Cần CPU/GPU/network timeline và RTT matrix | M05/M07/M09 |
| G12 | Damage font/layout | Native vừa sửa kích thước/advance; cần browser screenshot tại DPR1/2 và palette/overlay nhiều hit; không lấy screenshot UI mock làm nghiệm thu | M08/M09 |
| G13 | Sound mapping và nhạc | Tiếng scream ở mọi hit đã bị bỏ trong native, nhưng timing/source mapping chưa đầy đủ. Không có MP3 trong Data; chưa xác minh soundtrack chọn server/nhân vật | M10 |
| G14 | Font web | Data không có file font, native dùng font cài ngoài; cần nguồn font phù hợp hoặc phương án atlas có căn cứ, không thay tùy ý | M04/M08 |
| G15 | Coverage class/map | Chưa có web class/map nào Done. DW/Lorencia là MVP, không lấy số model đứng được làm coverage gameplay | M11 |
| G16 | DB rollback và online test | Chưa tạo backup mới; chưa chạy test ghi DB trong đợt M00 này. Phải có backup/tài khoản test trước khi chạy online | M03 trở đi |

Không sửa các lỗi này bằng cách thay resource ngẫu nhiên, nới validation, làm damage/loot giả hoặc bỏ thông báo lỗi. Khi xác nhận sửa được, cập nhật ID này cùng kết quả test trong STATUS.
# Cập nhật W0002

Audit offline xác nhận 17 lỗi parser và 143 tham chiếu texture cục bộ chưa resolve (103 model), xem [báo cáo](ASSET-AUDIT.vi.md). Đây chưa phải kết luận Data hỏng và chưa được sửa. Animation float của clip không giữ, terrain/placement/table và fallback/shared texture vẫn chưa audit. Các giới hạn cũ bên dưới tiếp tục giữ nguyên trừ khi có bằng chứng riêng.


## Bằng chứng mới W0003

Xem [WORLD-AUDIT](WORLD-AUDIT.vi.md). Xác nhận lại Object4/Object44 và Object52/Object166 thiếu theo mapping native. Tổng 5 world hỗ trợ có 13 tên model chưa resolve (22 placement), World1 type168 chưa mapping. 42 world còn lại chưa được parser hỗ trợ. Trong 149 BMD không mang magic model, 9 bảng Text/Item/Skill đã decode; 140 chưa decode. Các thiếu sót này chưa được sửa; marker bị native bỏ qua không tự coi là lỗi resource. Chưa kiểm tra ảnh terrain/light/fallback hoặc visual/collision.

## Bằng chứng mới W0004

[Audit cấu trúc](STRUCTURE-AUDIT.vi.md) mở phạm vi đọc schema lên 47 world: 176/188 file qua kiểm tra; 4 placement và 8 height khác giới hạn/schema native. Chưa sửa các file này. “42 world chưa hỗ trợ” vẫn đúng với renderer/native loader, nhưng nay đã có báo cáo cấu trúc từng file thay vì hoàn toàn chưa đọc. Không suy ra hình ảnh, tile, model hoặc collision đúng từ cấu trúc hợp lệ. Loader engine Windows đầy đủ vẫn chưa có trong source được cung cấp.

## Bằng chứng mới W0005

[Điều tra 12 file](EXCEPTIONS-AUDIT.vi.md): 4 heightmap decode được pixel theo BMP nhưng chưa biết cách chuyển thành height MU; 1 heightmap thiếu pixel theo header, 1 file toàn zero, 2 wrapper chưa phân loại. 4 placement có tổng 2 record NaN + 4 record scale âm, đã ghi offset cụ thể. Không sửa Data hoặc nới production loader; chưa kết luận các khác biệt này là lỗi bản gốc.

## Bằng chứng mới W0006

[Audit animation](ANIMATION-AUDIT.vi.md) đã đọc mọi frame/locked-position ở 5.028 model; 6 file chưa qua và dừng ở lỗi đầu tiên. Thay ghi chú chung “mọi animation chưa audit” bằng phạm vi cụ thể này: 4 model version 0 chưa hỗ trợ, Item/dark_05 có NaN, Object79/Object05 có locked-position vượt ngưỡng. Timing, interpolation, pose/render Windows và các lỗi mesh cũ vẫn chưa xác minh/sửa. Không có thay đổi production để tự bỏ qua float lỗi.

## W0007 — bộ khung không phải client game

Web đã build được nhưng browser/WebGL2 chưa được kiểm tra trực tiếp. Chưa có WASM, protocol/gateway/login, renderer asset hoặc font MU. Chưa đo FPS. M00 vẫn PARTIAL; scaffold M01 không chứng minh các asset ngoại lệ đã được sửa. Native chỉ rebuild target assets và chạy 4 contract có sẵn, chưa full clean build.

## W0008 — framing portable còn thiếu tích hợp

Core framing chỉ được compile/test C++ native. Chưa có emcc trong PATH, chưa WASM/bridge/browser hoặc fixture packet online. Crypto/transport decoding/serial/state/timeout không nằm trong reader này. Chưa thay implementation Socket production. Test framing không chứng minh login hay ClientVerify hoạt động.

## W0009 — đã có WASM, browser vẫn chưa xác minh

Không còn thiếu compiler cho phạm vi core: build script dự án chạy Emscripten 6.0.13-git và test WASM trong Node đã qua. Global launcher Homebrew còn trỏ Python 3.15 chưa cài; dùng lệnh build:wasm với cấu hình dự án, không dùng trực tiếp em++. Chưa browser/lifecycle production/crypto/gateway. Không coi same-fixture contract là kiểm chứng toàn bộ protocol hoặc gameplay.

## W0010 — browser và hai build mode

Đã có bằng chứng WebGL2/WASM framing trong in-app browser. Safari/Chrome độc lập và benchmark scene game vẫn chưa có. Debug FPS đo canvas kiểm tra, ping chưa có giá trị vì chưa nối server. Release đã loại diagnostics/overlay khỏi output, nhưng chỉ có canvas trống và loader core, chưa phải game phát hành.

## W0011 — transport đã có adapter, chưa có kết nối thật

Socket/clock mô phỏng + WASM thật qua kiểm thử session lifecycle/timeout/queue/order. Chưa TCP/WebSocket network/gateway, crypto, identity, login hoặc server ping. Adapter không mặc định bỏ cipher. C++ packet errors được bắt trong WASM trước khi trả cho transport JavaScript. App chưa tự connect; không suy ra gameplay online từ test adapter.

## W0012 — gateway local đã có, MU online chưa có

Gateway WS/TCP loopback và adapter/WASM qua fixture network thật; 7/7 test. Chưa kết nối server MU, route ConnectServer/GameServer thực, ClientVerify/cipher/login hoặc ping MU. Local chỉ bind loopback, chưa WSS/auth/deploy public. Slow-client stress/soak chưa thực hiện. Không mở gateway fixture lâu dài; Debug vẫn báo chưa kết nối.

## W0013 — codec có source contract, chưa discovery/login

Đã có ConnectServer packet codec và GameServer transport byte transform/MHP với explicit fixture keys. Chưa key derivation, C3/C4 crypto, serial/ClientVerify hoặc discovery runtime. Server code trong list16-bit nhưng request F4:03 chỉ1 byte theo source; code>255 bị từ chối thay vì bị cắt. Endpoint không tự trở thành TCP target; route cần khớp cấu hình. Chưa dùng profile/credential thật trong web.

## W0014 — discovery fixture đã qua, MU/browser discovery chưa có

Luồng CS handshake/list/chọn endpoint qua WS/TCP fixture loopback thực, với WASM codec và route kiểm chứng. Chưa đưa vào menu server/browser runtime hoặc server MU thật. Chưa mở GameServer từ select, chưa crypto/identity/login. Debug ping không dùng kết quả fixture để hiển thị là đã nối MU.

## W0015 — derivation/greeting chưa phải game session

MuOnline key derivation đã so source InitHackCheck trên native/WASM; greeting EXTRA1 parser có kiểm endian/code/result. Chưa profile provisioning thật, encoding serial ngoài printable ASCII, custom CustomerName, GameServer network probe, ClientVerify hoặc login. Không suy ra encodedVersion khớp phiên bản chỉ vì parser đọc được. Chưa cập nhật ping theo fixture.

## W0016 — greeting probe qua fixture, chưa authentication

Game probe mã hóa qua WS/TCP loopback thực đã qua cả MHP bật/tắt; không có byte login gửi đi. Chưa game connection browser/server MU, provisioning profile, ClientVerify/identity hoặc C3/C4/login. Key sai có thể chỉ bị phát hiện qua timeout khi header giải mã trông hợp lệ; không coi đó là server lag. Probe đóng sau greeting và không giữ gameplay session.

## W0017 — identity provider là phần chưa có

F3:09 wire codec/XOR đã qua source-reference nhưng browser chưa có provider HWID tương đương native. Không random ID hoặc localStorage để né blacklist/count theo thiết bị. Chưa helper native/browser integration/provisioning, chưa đọc host UUID hoặc identity thật. Source không có ACK F3:09; không đánh dấu ClientVerify chỉ vì gửi xong. C3/C4 SimpleModulus/serial/login còn thiếu.

## W0018 — crypto core đã có, chưa tích hợp phiên game

SimpleModulus/key-file/serial qua source-reference native/WASM bằng key tổng hợp. Chưa provisioning Enc1/Dec2 thật, chưa nối PacketCipher vào session GameServer hoặc login/browser MU. Identity provider W0017 vẫn thiếu. Không suy ra interoperability hoặc 60 FPS gameplay từ contract. Các ghi chú “chưa SimpleModulus” ở đợt trước là trạng thái lịch sử, được cập nhật bởi W0018 trong phạm vi core này.

## W0019 — pipeline có, phase đăng nhập chưa có

Channel mới ghép crypto/framing bằng WASM thật và socket mô phỏng. Chưa network gateway/channel, chưa UI hoặc MU. Adapter chỉ có connect/partial-frame deadline; greeting/identity/login deadline thuộc state machine chưa triển khai. Không đánh dấu authentication từ trạng thái open. Identity provider và provisioning vẫn thiếu; M02/M03 chưa hoàn tất.

## W0020 — identity-sent chưa phải verified

Handshake đã có deadline/cleanup với provider giả trong test. Chưa provider browser thật, provisioning hoặc network MU. Không ACK identity riêng, không coi send thành công là authentication. Chưa login/compatibility policy cho encodedVersion. Packet trong lúc chờ provider hiện bị từ chối; cần kiểm chứng thông báo sớm của server trước khi production. Chưa thay UI hoặc đo gameplay.

## W0021 — login codec chưa phải đăng nhập online

Layout EX603 59 byte và credential-XOR qua source-reference native/WASM. Chưa session login/UI/network MU hoặc account thật. Adapter giữ chính sách account/password ASCII 1–10 của native; field password 20 byte không có nghĩa server đang đọc đủ 20. Chưa version provisioning, text Data/Language hoặc mã lỗi UI. Các mã response giữ raw, chỉ 1 accepted. Provider/keys thật vẫn chưa có.

## W0022 — session login chưa được thử trên MU

Account session đã có bằng WASM/socket mô phỏng; chưa UI, MU/gateway network hoặc credential thật. Mỗi kết nối chỉ cho một login đã gửi, khác phạm vi retry source cần mở rộng sau. Packet bất ngờ trong awaiting-login đóng phiên; chưa dispatcher notice/close gốc. Chưa provider/provisioning, Language UI hoặc danh sách nhân vật. Không suy ra game online/60 FPS từ test này.

## W0023 — roster codec chưa phải màn chọn nhân vật

List/policy ABI đã kiểm chứng bằng struct source trên native/WASM. Chưa lobby request session, UI, render class/equipment hoặc decode charset. Giữ raw18 byte để không làm mất trang bị; không suy ra model đã đúng từ parser. Chỉ hỗ trợ tên ASCII theo adapter native, chưa encoding khác. Chưa MU network/provider/key provisioning; không có game online hoặc đo FPS gameplay.

## W0024 — lobby có fixture, UI/MU chưa có

List+policy đã ghép sau account session bằng WASM/socket mô phỏng. Chưa mạng gateway/MU hoặc browser lobby. Khi chờ pair, notice xen kẽ hiện bị từ chối; cần dispatcher theo source trước production. Charset còn raw, chưa render class/equipment/effect, UI chọn/tạo/xóa hoặc Language. Provider/profile/key provisioning và hiệu năng game chưa được xác minh.

## W0025 — appearance là dữ liệu, chưa render

Bảy slot equipment và class/tier đã có decoder. CharSet chỉ có visual grade/cờ tồn tại, không exact level hoặc option đầy đủ. Wing/pet/custom còn raw; chưa model mapping hoặc ánh sáng/shader/animation. Weapon511 bị source bỏ level/flag, không tự sửa server. Chưa hình ảnh/MU network hoặc FPS game; không coi codec là màn chọn nhân vật đã giống gốc.

## W0026 — cánh/pet chưa có resource renderer

Wing/pet decoder có source fixture, nhưng custom catalog là giả trong test. Mã custom có thể đụng mã builtin; status ambiguous chưa cho phép chọn model. Chưa runtime catalog/asset mapping/attach/animation/effect/UI hoặc network MU. Không suy ra render giống gốc/60FPS từ numeric test. Provider/profile/key provisioning vẫn thiếu.

## W0027 — custom text rỗng không chứng minh builtin/compiled catalog

Ba text catalog có0 active entry. Không có custom asset để kiểm dependency; chưa binary ServerInfo.sse catalog verification. Main extension gọi builtin engine ở địa chỉ Windows, không có thân loader đó trong Main_EX603. Chưa builtin mapping/renderer mới. Source server50/client100 custom entries khác capacity, không tự mở rộng server. W0026 ambiguity chưa được resolve tại runtime từ audit text.

## W0028 — có item paths chưa có asset pipeline

285 unique native-mapped paths có file trong Data nhưng11 lệch case; runtime resolver chưa có, HTTP host phân biệt case sẽ cần manifest. Không kiểm geometry/texture/visual và không coverage toàn active Item.bmd. Body armor class/tier mapping khác icon mapping chưa port. Chưa scene/UI hoặc network MU; không claim render/60FPS.

## W0029 — chưa có GUI game hoặc asset fetch

Trang hiện có canvas/overlay kỹ thuật, không có UI game. Manifest285 file đúng case có hash nhưng chưa stage/fetch/verify bytes tải về hoặc render. SHA-256 version là content identifier không phải chữ ký; trust/provisioning còn thiếu. Texture dependencies/model decode/UI/MU network chưa hoàn thiện; không có bằng chứng FPS gameplay.

## W0030 — loader kiểm fixture, chưa tải model trong browser

Stream/size/hash/abort/budget qua Node fetch fixture và WebCrypto; chưa HTTP server/browser integration hoặc decoder/render. Output reservation không phải hard cap toàn browser heap; fetch/crypto có thể copy thêm. Progress100% nhận byte vẫn chưa verified cho đến promise resolve. Không GUI game, không MU network/provisioning hoặc đo FPS game.

## W0031 — BMD parser chưa xuất geometry cho renderer

Parser đọc BMD v10/v12 trực tiếp; JS bridge hiện chỉ inspection metadata, chưa giữ model/typed buffers hoặc animation playback. Root-lock positions validate nhưng chưa retain/export track; default giữ action0. Ba model v10 thật qua NodeWASM, v12 bằng fixture, chưa sweep toàn Data hoặc browser/WebGL hình ảnh. Texture/alpha/light/effect/UI và MU network/provisioning/FPS game còn thiếu. Giới hạn parser portable chặt hơn native và được ghi riêng.

## W0032 — geometry buffers chưa được vẽ

Đã xuất owned typed arrays initial pose; chưa GPU upload/shader/camera/texture hoặc browser visual test. Normal transform synthetic-tested, chưa shader Windows reference; de-indexed corners chưa tối ưu. Chưa animation/root motion/attach/effect hoặc GUI game. Ba model NodeWASM không chứng minh toàn Data hoặc60FPS game; MU network/provisioning vẫn thiếu.

## W0033 — texture decoder chưa có visual browser reference

OZT pixel contract native/WASM và4OZJ wrapper thật đã qua; JPEG createImageBitmap thật chưa chạy trong browser, chỉ lifecycle mock. Chưa orientation/UV/gamma/alpha-cutout/blend/light correctness. Portable dimensions2048 chặt hơn native4096; không RLE/OZB. JPEG abort chỉ reject/close khi decoder trả bitmap, chưa deadline nội bộ. Chưa stagedHTTP/model preview hoặc GUI/game online.

## W0034 — có preview nhưng chưa visual parity

Browser đã render3BMD textured từ file gốc. Cánh nền đen vì chưa MU material/alpha; mũ equipment self-pose chưa bind Player skeleton, không phải lỗi được che bằng model thay thế. Shader/camera chỉ preview, chưa effects/animation/GUI game hoặc MU network.60FPS snapshot preview không bảo đảm game. Context loss dừng/yêu cầu reload, chưa autorebuild; full lifecycle/crossbrowser chưa kiểm.

## W0035 — bone binding đúng numeric chưa là hình ảnh đúng

Player pose binding đã so native nhưng mũ rời vẫn chưa nghiệm thu visual. Cần full-body/mesh hide/UV convention. Wing nền đen chưa sửa: JPEG không alpha, chưa source material/pass để loại nền, không tự đặt RGB threshold. Preview material constant/Repeat là theo adapter native, không chứng minh Windows shader parity. GUI/animation/MU online còn thiếu.

## W0036 — body/OZJ đã cải thiện, wing/game chưa chuẩn

Đảo hàng JPEG đã được xác minh bằng native pixel reference và sửa; full5part base DW/testhelmet render trong browser. Chưa chứng minh tất cả material/UV/color Windows, rule hide mesh hoặc điều kiện mặc helmet trên DW. Wing vẫn nền đen. Pose tĩnh, chưa animation/GUI/online; Player parse lặp từngpart ở startup chưa tối ưu C++handle. Không suy ra gameplay60FPS.

### W0037 — Class body preview

21 tổ hợp class/tier đã kiểm tra decode/binding/texture dependency, chưa xác nhận hình với Windows. Chỉ base-body type511 được map trong preview JS; equipped armor substitutions chưa chuyển hết sang core. RF male idle giữ nguyên giới hạn native, chưa xác minh. Preview static frame0 không phải animation; wing/hide material còn thiếu. Xem BODY-CLASSES.vi.md.

### W0038 — Giáp mặc

Core resolver đã so 53.760 tổ hợp với adapter native, nhưng Data chỉ kiểm tra 5 fixture giáp và21base. Summoner armor10 thiếu đầu dưới mũ trong browser: chưa chuyển phần skin bổ sung/visibility. Wing nền đen còn nguyên. Chưa bone attachment/animation/effects/equip online; không chứng nhận Windows visual parity. Xem ARMOR-RESOLVER.vi.md.

### W0039 — Mixed armor và hiệu năng

Head class khi slot mũ511 đã hiện trong browser; không được nhầm với sửa skin dưới mũ khi mặc helm. Fullset lỗi vẫn còn, chưa có thân hàm renderWindows để xác minh cách bổ sung. Screenshot mixed có FPS55.0, chưa profile nguyên nhân/frametime, chưa bảo đảm60FPS. Weapon/animation/effects/wing visibility còn thiếu. Xem MIXED-ARMOR.vi.md.

### W0040 — Attachment chưa render

Bone/grip local matrix đã khớp native49.152trường hợp; chưa nhân pose world matrix và áp dụng vào geometry vũ khí. Preview chưa có vũ khí trên nhân vật và chưa đổi stance. Không đánh dấu weapon rendering DONE. Skin dưới mũ/wing/FPS vẫn còn các giới hạn W0039. Xem EQUIPMENT-GRIP.vi.md.

### W0041 — Weapon geometry fixture

Đã ghép geometry Sword01 theo bone world×grip,12trường hợp realData/native qua. Chưa chuyển stance, inventory/CharSet attachment hoặc animation; chưa geometry bow/shield/wing. Bounds camera technical có thể đổi khi thêm weapon, chưa phải game camera. Skin/wing/FPS/GUI/network vẫn còn thiếu. Xem ATTACHMENT-GEOMETRY.vi.md.

### W0042 — Action static

Sword01 dùng twoHand từ Item.bmd có checksum, thân/weapon cùng idle action; vẫn frame0, chưa animation/walk/run/attack hoặc equip online. Chỉ field twoHand, không full ItemCatalog/locale/eligibility. Bow/staff/twoHand policy numeric-tested, chưa visual từng loại. Skin/wing/FPS/GUI/network còn thiếu. Xem EQUIPMENT-ACTIONS.vi.md.

### W0043 — Animation/performance

Idle đã quaternion-interpolate và đồng bộ body/weapon, không phụ thuộc FPS. Vẫn CPU pose/geometry→Float32 copy→GPU bufferSubData, chưa GPU skinning, pooled output, walk/run/attack/transition blend.120Hz numeric reference không phải120FPS hiển thị; CPU p95 không tính GPU/compositor. Skin/wing/GUI/network vẫn thiếu. Xem BODY-ANIMATION.vi.md.

### W0044 — Motion fixture

Idle/walk/run+blend0.12s/rootXY có trong preview, demo cycle là lịch test tại chỗ. Chưa native run eligibility/path/input/network hoặc attack/cast/equip changes; movementRate1fixture. Interval p95 là rAF cadence, CPU p95 không gồmGPU; chưa xác nhận120FPS. Skin/wing/GUI/RFidle vàWindowsparity còn thiếu. Xem MOTION-ANIMATION.vi.md.

### W0045 — Loadout fixtures

Cung/tên Elf và staff/khiên DW đã render + animation,24case geometry native qua. Chưa equip eligibility/live inventory/CharSet/attack/cast/projectile; chỉ một bộ mẫu mỗi loại. Safe/back mới numeric reference, chưa screenshot browser riêng. Skin/wing/GPU skinning/GUI/network/120FPS còn thiếu. Xem WEAPON-LOADOUTS.vi.md.

### W0046 — Attack visual fixture

Clip đánh đã chọn theo item/hand/twoHand và đồng bộ body/attachment. Chưa cast/skill/effect/projectile/target/hit/damage hoặc attack speed theo chỉ số/server. Attack lặp là fixture quan sát, không auto-attack gameplay. Skin/wing/GPU skinning/GUI/network/Windows parity/120FPS vẫn chưa xong. Xem ATTACK-ANIMATION.vi.md.

### W0047 — Cast không phải skill hoàn chỉnh

Cast147/146 luân phiên theo native đã có trong preview, chưa per-skill selector/Skill.bmd/cooldown/mana/packet. Audit8file Fire/Laser/sprite không đồng nghĩa effect đã render hoặc material/trajectory đúng. Chưa effect/sound/projectile/hit/damage; skin/wing/GPU skinning/GUI/network/Windows parity/120FPS còn thiếu. Xem CAST-ANIMATION.vi.md.

### W0048 — 7class chưa đồng nghĩa đủ7bộskill

Đã audit và xem browser7class với generic cast/Fire01 fixture. Fire Ball không qua class filter server choDK/Elf/DL/RF; không cấp skill từ fixture. Full class skill/effect matrix còn TODO. DL có CPU spike16,175ms trong audit, RF idle fallback chưa xác minh, Fire01 vệt lớn cần visual Windows reference. Skin/wing/hide/GUI/network/GPUskin/120FPS còn mở. Xem SEVEN-CLASS-EFFECTS.vi.md.

## W0049 — Menu kiểm thử

Menu class/skill đã có trong Debug, nhưng snapshot chỉ lọc class/chuyển chức. Skill chưa port chỉ hiện model idle và nhãn chưa có effect. Chỉ Fire Ball dùng Fire01 prototype; chưa có toàn bộ skill 7 class, điều kiện level/mana/trang bị, Language client hoặc hit/damage/server. Đổi config server cần chạy lại stage:preview để cập nhật snapshot.

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

W0065: Lorencia nạp toàn bộ terrain/object có mapping, mái125/126 fade theo HeroTile4. Chưa xác định object168; effect/animation/material chưa đủ. Asset helper tách quota12.000/phút khỏi identity/keys600/phút; giữ tối đa4 đồng thời, hash/allowlist/root confinement. Cần kiểm chứng đi bộ online qua cửa nhà/bar cùng người dùng sau reload phiên hiện tại.

W0066: đã có giữ phím/click path, chuyển safe/armed stance, foot run policy và normal attack4monster(2/3/11/12) với damage/HP/regen server. Chưa full monster binding, skill, speed theo buff/mount/wing, authoritative equipment updates, corpse/death animation, floating damage/full HUD. A* không phải Windows two-pass parity; proximity monster picking chưa occlusion; đã ACK thì hoàn tất đoạn trước khi đổi đích. Ngoài Lorencia còn giới hạn12ô. Marker model/rings gốc chưa full magic/joint/spark composite. Xem `MOVEMENT-COMBAT-W0066.vi.md`.

W0067: đã có bóng chiếu terrain cho mọi mesh caster đang render, gồm world/actor/NPC/monster/trang bị. Chưa bóng lên object/tường/tầng, self-shadow, soft-shadow, Shadow01/cloth riêng, đầy đủ shadow gates theo map/buff/cloaking. Thiếu model/binding vẫn không có hình/bóng. Alpha0.30 và cutout/stencil là policy web, chưa parity100% nguồn Windows. Xem `SHADOWS-W0067.vi.md`.

W0068: HUD/panel bằng atlasgốc vàdata server đã có. Equipment chỉxem, túimởrộng chỉđọc, chưadragthaygear/bodyrefresh,shop/warehouse/repair/useitem/grounddrop/fulltooltip/upgradeeffects/skill/potionquickbar/chat/friends. Class/level/base+bonus/combatstats thật vàcộngđiểmquaACK (không dùngđiểmthậtđểtest). Túichính cóC3:24serialize/ACK/timeoutlock; browserdragtest chưaphátraDrop nên bổsungclicknhấcđặt, chưagọitestACKlivepasskhi chưacóbằngchứng. Không tuyênbốpixelparityWindows vì nguồnNewUIimplementationchưađủ. Xem `HUD-INVENTORY-CHARACTER-W0068.vi.md`.

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
