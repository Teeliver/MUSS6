# Combat macOS — phạm vi hiện tại

- Click trái mặt đất: di chuyển; click trái NPC: đối thoại.
- Chọn Energy Ball/Fire Ball bằng thanh skill hoặc phím 1–5.
- Click phải / secondary click hai ngón lên quái: dùng skill đã chọn. Không đổi thao tác học sách trong Inventory.
- Hover quái dùng Data/Interface/CursorAttack.tga, NPC dùng CursorTalk.tga; cursor nạp một lần lúc chuẩn bị map. Player khác không dùng cursor quái.
- Tham chiếu: ZzzOpenData.cpp (BITMAP_CURSOR+2), ZzzInterface.cpp RenderCursor trong bộ reference /tmp/muonline-client-sources-master. Chưa xác nhận bộ reference trùng hoàn toàn EX603.
- Hỗ trợ hai skill DW đơn mục tiêu 4/17 và Evil Spirit 9; server quyết định học skill, MP, sát thương. Chưa hoàn thiện toàn bộ skill/effect/giữ chuột tự cast.
- Build và kiểm thử tự động không thay thế kiểm thử combat online và FPS thực tế.

## 0.29.1

- Sửa giới hạn 5 phần giáp tách khỏi 6 clip animation; tên skill cố định 32 byte được đọc an toàn kể cả không có NUL.
- Đánh thường gửi C1/PROTOCOL_CODE2 (index, action 120, direction); animation local vì GCActionSend chỉ gửi viewport người khác. Sát thương luôn từ server.
- Ctrl+click và secondary click đều dùng skill; tự đi tới tầm trước khi thực hiện một lần.
- Ô skill chính mở danh sách các skill cơ bản <=24 đã học; Ctrl+1..5 gán skill chọn. Chưa lưu gán phím về OptionData.
- Tooltip Skill BMD EX603: 650 bản ghi x88 byte, checksum 0x5A18; nhãn Text[170,174,175,360], source font. Chưa đủ công thức nâng cao/master/set/harmony.
- F3:05 cập nhật vitals và kích hoạt hiệu ứng lên cấp thường: 15 trail Flare, Magic_Ground2 và pLevelUp.wav. Chưa port hiệu ứng master.
- Hai projectile DW đang có resource: Fire01.bmd cho Fire Ball; Thunder01 cho Energy Ball. Chưa hoàn thiện toàn bộ particle, terrain light, hit effects và các skill còn lại; chưa xác nhận giống 100%.
- Số damage dùng FontTest.tga và bảng màu tham chiếu WSclient.cpp/ZzzEffectPoint.cpp, giới hạn 64 số đang hiện. 0.29.2 bổ sung hàng đợi giới hạn 64 sự kiện để giữ nhiều hit trong cùng đợt pump.

## 0.29.2

- Cast target tách khỏi damage target: damage packet sau ACK không thể đổi mục tiêu projectile.
- Đuổi quái dùng tx/ty cập nhật qua packet move, không dùng x/y ban đầu trong viewport. Click mục tiêu mới hủy route cũ chưa gửi; phím di chuyển hủy ý định đánh.
- Projectile nhắm vị trí node quái đang hiển thị (fallback tx/ty); tách yaw theo thế giới khỏi rotation đổi hệ tọa độ model.
- Font số damage giữ FontTest.tga gốc, crop atlas16x16, render pixel không làm mờ; normal/miss/critical/excellent theo damage type của server, không suy đoán từ màu item. Critical/perfect/excellent có scale50 giảm về15 theo source.
- Quy tắc đăng nhập dưới level6 ngoài safezone về thành trong ObjectManager::CharacterInfoSet được giữ nguyên theo xác nhận người dùng. DB MuOptions level3 đã đối chiếu.

- Packet kết liễu thực tế EX603 là 0x9C (GCMonsterDieSend dùng PMSG_REWARD_EXPERIENCE_SEND), gồm ViewDamageHP bên cạnh EXP. Parser damage nhận cả 0x11, 0x16 legacy và 0x9C. Màu đòn kết liễu cam theo ReceiveDieExpLarge; màu excellent/critical/perfect theo type từ 0x11.

- Đuổi mục tiêu đi từng đoạn tối đa3 ô, tính lại endpoint; cận chiến áp sát1 ô thay vì lấy trần kiểm tra3 ô của GameServer làm cự ly dừng. Chỉ kết thúc ý định đánh khi có hit/ACK đúng mục tiêu; giới hạn5 lần thử lại và tôn trọng thao tác di chuyển của người chơi.
- Model quái có clip6 MONSTER01_DIE từ OpenMonster; tốc độ .55 x25 ticks/s. RecentDead tối đa64 giữ thông tin qua packet destroy để hiệu ứng, số damage và clip chết không bị cắt. Renderer bỏ node sau khi clip chết kết thúc, không thay HP server.
- Kiểm thử live 0.29.2: MuOptions đánh thường78 damage, Energy Ball52 damage, cả hai được GameServer xác nhận lên đúng mục tiêu. Trace xác minh trần cận chiến3 ô từ CGAttackRecv và vị trí hiện tại khác endpoint quái khi đang đi; trace mặc định tắt.

## 0.29.3 — số damage và thời gian chạy hiệu ứng

- FontTest.tga giữ nguyên RGB viền đen, các mức xám và alpha. Tô màu bằng nhân RGB trên pixel nguồn, không dùng lockFocus/SourceAtop (cách cũ xóa viền và dựng lại ảnh theo backing scale). Vị trí số được canh pixel của màn hình.
- Không hủy animation cast chỉ vì còn đoạn di chuyển đã được server xác nhận.
- Projectile Fire Ball/Energy Ball và hiệu ứng level-up cập nhật bằng dt của renderer nhân vật; không phụ thuộc đồng hồ SCNAction khi thay/nhúng SCNView. Hàng đợi hiệu ứng được chuyển sang renderer trước khi thao tác SceneKit; không giữ mutex lúc commit transaction. Giới hạn 64 hiệu ứng đang chạy.
- Entry regression kiểm tra cast khi còn đường đi, chạy nhiều khung hình của hiệu ứng level-up, kiểm tra geometry thực và chụp render. Không coi việc chỉ tạo node là bằng chứng hiệu ứng đã chạy.
- MuOptions được cấp 1 Scroll of Evil Spirit (15:8), slot16, theo DBItemByteConvert. Sao lưu inventory trước khi cấp; so sánh mọi byte ngoài slot được cấp không thay đổi. Không sửa điểm, chỉ số, level hoặc điều kiện học sách.
- 0.29.5 bổ sung Evil Spirit (phần dưới). Chưa xác nhận toàn bộ hiệu ứng bằng đối chiếu từng frame với client Windows EX603.

## 0.29.5 — Evil Spirit, đồ trên đất và nối đường chạy

- Evil Spirit được chọn ở thanh skill: click trái mặt đất để cast không cần mục tiêu; click phải cũng hỗ trợ. Gửi C3:1E với target FFFF; ACK server mới phát hiệu ứng. Bốn spirit di chuyển theo tick 25 Hz, các đợt multi-hit gửi PROTOCOL_CODE4, tối đa 5 mục tiêu/đợt. Server giữ quyền kiểm tra tầm, mana, sát thương và chết; không tạo damage giả phía client.
- Resource JointSpirit01, Laser01 và sEvil.wav lấy từ Data. Blend tối theo GL_ZERO/GL_ONE_MINUS_SRC_COLOR và luminosity lifetime*.1 trong reference; chuẩn bị shader trước khi cast. Chưa xác nhận tương đương từng frame với Windows EX603.
- Ground viewport C2:20/C2:21, pickup C3:22 và drop C3:23 theo struct EX603. HackPacketCheck yêu cầu mã hóa pickup/drop. Chỉ xóa item khỏi túi khi server xác nhận; click nhấc item trong túi rồi click mặt đất để quăng. Không quăng trực tiếp từ kho.
- Giữ Option (Alt) để hiện tên toàn bộ đồ trên đất; bình thường hover mới hiện. Model thường dùng BMD/Data gốc, không bảo đảm đã ánh xạ mọi loại đặc biệt. Zen hiện model Gold01, chưa chia nhiều đồng theo số tiền.
- EXP dùng Text[486]; nhặt item dùng Item BMD + Text[918], Zen dùng Text[224]. Thông báo hệ thống dùng màu RGB(100,150,255), font nguồn; giảm cỡ chat xuống 9 và notice xuống 10 theo tỉ lệ UI.
- Tỉ lệ rơi do gObjMonsterDieGiveItem, ItemDrop, ItemMoneyDrop, ItemBagManager trên GameServer quyết định. Các cấu hình Data/Item/ItemDrop.txt, Data/Item/ItemMoneyDrop.txt, Data/MapManager.txt, MonsterList.txt và GameServerInfo - Common.dat được giữ nguyên; client chỉ nhận kết quả.
- Nối đoạn đường trước khi hết đoạn cũ, giữ pha animation đi/chạy và cố định control point của đoạn Catmull-Rom đang chạy để append không đổi đường cong giữa bước. Không tăng vận tốc 3/3.75 ô mỗi giây của cơ chế hiện tại.
- Kiểm thử: 14 contract/regression qua; live Evil Spirit có ACK, trừ MP, damage và kill/EXP từ server. Live drop/pickup xác minh viewport, ACK và inventory thật. Khoảng callback renderer khi đã ổn định p50 16.67 ms, p95 16.72 ms; vẫn ghi nhận khựng lúc chuẩn bị GPU ban đầu, chưa cam kết khóa 60 FPS mọi tình huống.

- Live kiểm tra rơi đồ 0.29.5: Spider level2 tại Lorencia, ItemRate120, effective ItemDropRate100, map ItemDrop1; Evil Spirit damage201, server kills1, EXP1 và ground viewport item1. Không chỉnh cấu hình/RNG để ép rơi.

## 0.29.6 — điều khiển và phản hồi combat

- Click trái luôn đi/đánh thường; chọn Evil Spirit không chiếm click di chuyển. Control+click hoặc secondary click mới gọi area cast. Entry live regression phát mouse event thật: click thường sinh destination, Control+click chỉ sinh đúng một area cast.
- Control hoặc Option giữ tên ground item tạm thời. Option+Space khóa hiện tên; nhấn Option rồi thả để tắt. Space nhặt món gần nhất trong 2 ô qua cùng luồng pickup/ACK hiện có; không tự tạo item hoặc bypass điều kiện server.
- Bỏ vòng chờ 200 ms trên serial network queue của từng attack/cast. ACK/damage tiếp tục nhận bằng pump không blocking ở timer UI. TCP_NODELAY cho socket client và accepted socket GameServer; không sửa vận tốc/tỉ lệ rơi/timer gameplay.
- Evil Spirit chỉ thay geometry theo tick 25 Hz khi simulation đổi frame, không tạo lại geometry trùng 60 lần/giây. Không project tên mọi item mỗi frame khi nhãn đang ẩn.
- PMSG_ACTION_SEND 0x18/action120/121 có actionRevision riêng: chuyển sang clip BMD3/4 đúng một lần mỗi packet tấn công. Không khởi động lại attack khi chỉ có packet di chuyển. Nạp, nội suy, blend xương giống các clip khác; live regression xác minh xương Spider thực sự đổi pose rồi vẫn chạy hết clip chết.
- Damage nhận vào chính nhân vật được định tuyến theo playerIndex session, dùng FontTest.tga và màu đỏ; trước đây chỉ tìm node quái nên hit vào bản thân bị bỏ mất.
- Sound bank nạp sẵn Data/Sound, phát trên main queue, không đọc file trong renderer. Có bước chân theo Noria/Devias/đất, vung vũ khí/cung, skill cơ bản, nhận damage theo giới tính class, nhặt/quăng và attack/death của bốn model quái hiện được port. Chưa tuyên bố đã nối mọi ambient/effect hoặc mọi loại quái trong toàn bộ MU. Chưa đối chiếu đầy đủ timing sound với Windows EX603.
- Kiểm tra online GameServer local: normal78, Energy Ball76, Evil Spirit199; kill1 và ground items2. ACK di chuyển trong đợt test khoảng0.23–11.87ms. CPU server trước sửa khoảng14.5%, không có bằng chứng server CPU bão hòa; vẫn cần test trải nghiệm thực tế ở tải lớn. Renderer thực p50 ~16.67 ms, có một số frame >25 ms, không cam kết khóa60FPS mọi cảnh.

## 0.29.7 — sửa âm thanh, nhãn đồ và số damage

- Bỏ tiếng pMaleScream/pFemaleScream ở mọi hit: gán tiếng la toàn bộ lần nhận damage trước đó không có căn cứ đầy đủ từ source engine gốc.
- Evil Spirit dùng danh sách mục tiêu trong bán kính6 của SkillList/CheckSkillRadio hiện tại thay vì điều kiện chạm đầu sprite ngẫu nhiên tại4 frame. Server tiếp tục kiểm tra bán kính cấu hình, tấn công, miss/HP/MP; không ép chết quái. Trong ba lần kiểm tra có hai kill/damage148 và154, một lần không có kill; chưa có bằng chứng mọi cast luôn trúng và không được biến miss hợp lệ thành kill giả.
- Phím nhãn đồ ghi nhận cạnh nhấn Option; Option+Space khóa và thả phím không xóa. Thêm poll Option/Space khi cửa sổ game là key window để không phụ thuộc duy nhất sự kiện keyDown. Không can thiệp hotkey toàn hệ thống nếu một ứng dụng khác chiếm tổ hợp.
- Nhãn có anchor riêng ngay từ ground viewport, không chờ model. Bật khóa yêu cầu redraw ngay; draw tự project nhãn thay vì chờ timer/occlusion. Live regression so sánh pixel trước/sau khóa với món chưa có model đã qua. Không coi việc set biến là bằng chứng đủ cho phím vật lý: Mac hiện khóa nên chưa kiểm tra thao tác thật.
- Số damage vẫn FontTest.tga, crop mỗi chữ số theo nét gốc và giữ1px viền. Advance theo chiều rộng thật cộng khoảng trống; trước đây vẽ16px nhưng chỉ tiến8px gây chồng. Cỡ thường10, critical/excellent bắt đầu24 thay vì15/50 ở hệ quy chiếu UI; điều chỉnh nhỏ theo yêu cầu người dùng. Nhãn tên9, giữ fontGulim nguồn.
- Phân màu nhãn ground: gray cho+0 thường, gần trắng+1/+2, cam+3/+4, xanh+5/+6 hoặcskill/luck/option, vàng+7 vàZen/jewel, xanh láExcellent, Ancient chữ xanh nền(60,60,200), sockettím. Thứ tự và RGB tham chiếu BuildGroundItemLabelDescriptor tại https://github.com/sven-n/MuMain/blob/main/src/source/Engine/Object/ZzzInventory.cpp. MuMain không được xác nhận trùng binary EX603; special/event/custom item còn phải đối chiếu riêng.
- Thêm iTitle, âm thanh nút/cửa sổ/đăng nhập và ambienceWind/Forest/Dungeon từ Data. Bộ ZIP client hiện không có MP3; không bổ sung nhạc bên ngoài hoặc tuyên bố đã phục hồi toàn bộ soundtrack gốc.
