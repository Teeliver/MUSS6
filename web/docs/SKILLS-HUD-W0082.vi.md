> W0084 đã thay giới hạn RAM bằng lưu/đọc F3:30 trên server; thao tác bình HUD và chữ số cũng đã cập nhật. Xem [HOTKEY-PERSISTENCE-W0084.vi.md](HOTKEY-PERSISTENCE-W0084.vi.md).

# W0082–W0083 — Skills, bình thuốc và sửa disconnect

Cập nhật 2026-10-28. Phạm vi đã đưa lên bản debug 8080.

## Đối chiếu source

- `Main5.2/source/NewUIMainFrameWindow.cpp`, `_enum.h`, `SkillManager.h`: tọa độ atlas skill và hotkey item. `audit_skill_hud.py` trích nhánh RenderSkillIcon rồi biên dịch để lấy tọa độ, không suy icon từ skill ID. Catalog 386 skill, 10 atlas gốc có hash.
- `GameServer/SkillManager.h`, `ItemManager.h`, `Move.h`: kiểm tra ABI request bằng clang; packet skill C3:19, duration C3:1E, multi C3:DB của ENG, dùng đồ C3:26, teleport C3:1C.
- `GameServer/Protocol.h`: LANGUAGE=1 (ENG) dùng PROTOCOL_CODE4=0xDB; LANGUAGE=3 (CHS) mới dùng 0x1D. `native/game/CMakeLists.txt` và native client đều compile ENG.
- `GameServer/HackPacketCheck.cpp` và Data/Hack/HackPacketCheck.txt: opcode29 yêu cầu không mã hóa, opcode219 yêu cầu mã hóa. Bản web trước gửi C3:1D sai cả route và yêu cầu encryption, có thể bị đóng phiên. Đã sửa C3:DB, thêm regression so trực tiếp macro ENG trong source. Không tắt kiểm tra packet của server.

## HUD và thao tác

- Ô hiện/chọn skill nằm local X384/640 (ô xanh bên phải). Năm ô quick skill 1–5 nằm X224,256,288,320,352. Sửa theo chú thích người dùng; trước đó đặt ô chọn vào vị trí quick1 và chỉ có bốn ô quick.
- Click ô chọn mở skill đã học. Hover hoặc focus skill rồi Ctrl+0–9 để gán, phím số để chọn; năm ô 1–5 hiện trên HUD. Không tự thêm skill chưa học.
- Nhấc bình trong túi rồi click Q/W/E/R để tạo liên kết; hỗ trợ drag link. Vật phẩm vẫn ở túi, không gửi move item. Gộp đúng nhóm HP/MP/SD/AG, ưu tiên bình lớn. Số lượng đã tăng cỡ chữ theo HUD scale và in đậm.
- Q/W/E/R gửi yêu cầu dùng item; số lượng thực theo ACK server. Không trừ đồ hoặc mana giả ở client. Phím tắt hiện chỉ lưu RAM, chưa lưu CharacterOption qua các lần đăng nhập.
- Chuột phải dùng skill vào quái, tiếp cận khi ở xa; giữ để lặp skill mục tiêu, nhả để dừng. Chặn tấn công trong safe zone. Trên Mac dùng secondary click của trackpad.
- Motion casting thêm mode6/7 từ action146/147 vào WASM. Hiệu ứng gốc đã nối Fire Ball(4), Energy Ball(17), Evil Spirit(9); không coi mọi hiệu ứng skill đã port đầy đủ.

## Kiểm chứng

- 231 test Node qua, bao gồm ABI native request, opcode ENG regression, gán/nhóm potion, session không sửa inventory lạc quan, chuột phải và safe zone. Build debug115/release32 file thành công; deploy atomic và giữ backup.
- Browser offline: gán bốn bình Q/W/E/R, Q giảm tổng200→199, Ctrl+1/Ctrl+2 gán skill. Fire Ball fixture ngoài safe zone nhận3 cast requests khi giữ chuột phải; không có console error/warn. Đây là fixture, không chứng minh damage thật.
- Phiên MuOptions thật sau refresh: đủ năm ô skill, ô chọn bên phải; chọn Evil Spirit, DOM ghi16 cast requests và lastCastSkill9. Trong lượt người dùng test, MP856→520→498, EXP5899→6128→6407 và ACK move tiếp tục; không có console error/warn tại lần đọc. Không phải stress test mọi skill; thao tác người dùng diễn ra đồng thời nên không quy toàn bộ EXP tăng cho riêng Evil Spirit.
- Bằng chứng HUD offline `/tmp/mu-w82-hud-skills.png`; bằng chứng bản online sau sửa `/tmp/mu-w83-live-skills.png`.

## Phần còn lại

Chưa port toàn bộ skill đặc biệt/master/Rage Fighter, toàn bộ hiệu ứng/âm thanh theo từng skill, và lưu hotkey qua đăng nhập. Skill area/self hiện mỗi secondary click gửi một cast; lặp khi giữ mới dành cho skill mục tiêu. Cần kiểm thử từng nhóm skill và đối chiếu wave timing của multi skill để đạt parity đầy đủ; không đánh dấu100%.
