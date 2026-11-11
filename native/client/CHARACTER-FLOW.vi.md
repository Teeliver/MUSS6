# Chọn, tạo và xóa nhân vật — native 0.24

## Nguồn và phạm vi đối chiếu

Bộ Main_EX603 người dùng cung cấp là DLL mở rộng. Các phần engine đối chiếu ở bản tham chiếu `/tmp/muonline-client-sources-master`, không coi là engine EX603 đã xác minh từng compile flag. Data luôn đọc từ MuClientEX603, không thay đổi cấu trúc Data hoặc schema server để dựng UI.

- ZzzScene.cpp CreateCharacterScene / RenderCharacterScene: World74 logic tương ứng folder World75/Object75, camera (9758.93,18913.11,675.5), góc (-84.5,0,-75), FOV35.
- WSclient.cpp ReceiveCharacterList: 5 vị trí slot, góc quay; ngoại hình từ CharSet server.
- CharInfoBalloon.cpp: character_ex 118×54, vị trí đầu +350; tên, GuildStatus, class/level; màu theo CtlCode.
- ZzzScene.cpp SelectedHero: gmmzine, hai lớp 1.8/1.2 ô, quay ±WorldTime×0.01 độ, độ sáng sin(WorldTime×0.0015)×0.3+0.5. Native dùng hai mặt phẳng trên nền tại khu vực phẳng của scene; chưa phải bộ RenderTerrainAlphaBitmap tổng quát cho địa hình gồ ghề.
- CharMakeWin.cpp: nội dung 454×406, portrait viewport 410×335, cha_id 346×38 tại y317, bảy cha_bt 108×26; stat/description từ GlobalText. NewFace01…07, scale/góc/offset, action1 mở đầu rồi action0, tốc độ0.3 (Summoner0.25).
- CharSelMainWin.cpp / MsgWin.cpp: Delete kiểm tra guild, CtlCode, giới hạn level; xác nhận GlobalText1712 rồi ô mã1713; message_back352×113 và delete_secret_number171×23.

## Mạng và dữ liệu

Character giữ nguyên slot/name/level/control/18 byte CharSet và bổ sung GuildStatus đã có trong packet gốc. Chọn slot giữ index riêng, không dùng selection của NSTableView ẩn.

Danh sách F3:00 được theo sau bởi DE(flag0) trong GameServer/DSProtocol.cpp. Client nhận DE để lấy mask class và CharacterDeleteMaxLevel theo PMSG_CHARACTER_CREATION_ENABLE_SEND của EX603 EXTRA1. Bit mask: SU1, DL2, MG4, RF8; DW/DK/Elf luôn mở. Không dùng thuật toán MaxClass của nhánh engine khác để giải mã mask này.

Tạo dùng F3:01, tên 4–10 ký tự ASCII chữ/số, classCode=classIndex×16. Tên trùng/không tạo được hiển thị GlobalText1716/396. Bộ lọc từ cấm CheckName/CheckSpecialText đầy đủ của engine chưa port; không tuyên bố kiểm tra tên tương đương100%.

Xóa dùng F3:02, name10 + PersonalCode10, chờ kết quả thật rồi đọc lại danh sách; không xóa model/bản ghi trước ACK. Session kiểm tra lại danh sách/quyền sở hữu, GuildStatus255, CtlCode và giới hạn level trước khi gửi. Server vẫn kiểm tra PersonalCode, CharacterDeleteSwitch và gọi DataServer/WZ_DeleteCharacter. Không ghi trực tiếp database từ client. Mã cá nhân được xóa khỏi ô ngay khi gửi; buffer được ghi đè khi tác vụ kết thúc.

## Renderer và bảo trì

GPU rig dùng lại MuCharacterRig/entity_view để không nhân đôi hệ skinning. Không đọc file hay gọi socket trong callback render. Mục tiêu renderer60FPS, không khẳng định mọi frame đều đạt16.67ms khi chưa đo trên GUI.

Sau báo lỗi click: bỏ mesh hit-test của SceneKit trên skinned geometry đang animate; chọn bằng vùng nhân vật chiếu lên màn hình. Không giữ charactersMutex qua projectPoint hoặc thay đổi SCNNode, tránh khóa chéo với render thread. Snapshot mảng rig được giữ strong rồi nhả khóa trước khi gọi SceneKit.

## Chưa hoàn tất

- Trang bị/vũ khí/cánh, skin chuyển cấp và hiệu ứng nâng cấp trong scene chọn nhân vật chưa port đầy đủ. Những loại chưa hỗ trợ được ghi vào log nội bộ; có thể vẫn hiển thị body cơ bản.
- Idle nữ/ Summoner dùng action2/3. Idle đặc thù RF trong selection vẫn cần đối chiếu enum/action của đúng build.
- Nền trời, đuốc, khói, hiệu ứng môi trường scene75 chưa đủ.
- Gameplay/world renderer hiện vẫn giới hạn DW/Lorencia; tạo và preview class khác không có nghĩa đã chơi đủ các class/map.
- Luồng xóa không tự chạy trên nhân vật có sẵn. Kiểm thử packet không chứng minh stored procedure MySQL đã xử lý mọi trường hợp xóa.

### Fix Elf world entry and database deletion (2026-10-16)

World entry no longer rejects Elf: map 3 loads World4 terrain/Object4 placements,
Class03 body, female idle 2/walk 16 and shared run 25 from Player.bmd.
World0-specific roof/blend rules are restricted to World0. Environment remains
batched by texture and 16x16 spatial chunks.
The supplied World4 lacks TileGrass02.OZJ and TileGround02.OZJ; these slots
currently use World1 textures with a diagnostic. Exact Noria texture parity needs
those original assets. This is an explicit remaining asset limitation.

The local database lacked WZ_DeleteCharacter. Install
native/db/delete-character-procedure.sql for new databases as well. It validates
ownership, guild membership and blocked state, then atomically deletes Character
and clears its account slot. GameServer retains personal-code/level checks.
Validated in MuOnlineNativeValidation2 using a disposable Elf: wrong owner
preserves the row; correct owner removes row and slot. Existing user characters
were not deleted during validation. Ancillary character-data cleanup still needs
comparison against the original SQL Server procedure before claiming full parity.
