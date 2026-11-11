# Sửa mời Duel, đánh người và vị trí điểm — 04/11/2026

## Lỗi được sửa

1. **Hộp mời không đóng:** callback Đồng ý/Hủy gửi AA02 nhưng quên destroy hộp thoại. Bấm tiếp gửi lại answer, đụng các điều kiện DuelUserReserved/DuelUserRequested/DuelUser ở server và có thể reset trận vừa bắt đầu. Đã đóng trước khi gửi và giới hạn mỗi hộp đúng một answer; Esc cũng gửi từ chối như source. ACK start/end hoặc disconnect xoá hộp cũ.
2. **Text sai:** bỏ GlobalText916 ("Please make sure to differentiate"). Dùng CDuelMsgBox gốc: tên đối thủ trong ngoặc vuông màu vàng, GlobalText910/911, ảnh DuelWindow, OK/Cancel, kích thước230×222/y70 và âm thanh gốc. Không đổi packet mời/answer.
3. **Không damage:** world-controls và world-session chặn tất cả player. Đã dùng đối thủ từ ACK Duel thành công (`duel.active/enemyIndex`) để cho đánh thường và skill; Evil Spirit/multi-hit nhận đúng index đối thủ. Player đứng ngoài trận vẫn không thành mục tiêu. Kết thúc Duel bỏ quyền đánh.
4. **Con trỏ:** nhánh CheckAttack của client gốc cho đánh đúng DUEL_ENEMY; ngoài safe zone dùng BITMAP_CURSOR+2 = CursorAttack.tga. Web dùng cùng resource đã có, hotspot/kích thước của cursor gốc. Hover/chọn không mở quyền PK chung.
5. **Điểm số:** theo yêu cầu người dùng, chuyển frame điểm người thi đấu từ phía trên xuống **góc dưới bên phải, sát phía trên HUD**, giữ ảnh/scale gốc. HUD khán giả Duel riêng không đổi.

## Source đối chiếu

- Main5.2/source/NewUICustomMessageBox.cpp: CDuelMsgBox::RenderTexts/OkBtnDown/CancelBtnDown (destroy sau answer).
- Main5.2/source/WSclient.cpp: ReceiveDuelRequest/ReceiveDuelStart.
- Main5.2/source/ZzzInterface.cpp: CheckAttack nhánh KIND_PLAYER/IsDuelPlayer và RenderCursor BITMAP_CURSOR+2; ZzzOpenData.cpp ánh xạ CursorAttack.
- GameServer/Duel.cpp: CGDuelStartRecv/CGDuelOkRecv; SkillManager.cpp::CheckSkillTarget chấp nhận gDuel.CheckDuel. Không thay quy tắc damage/HP/SD server.

## Kiểm chứng

- **489/489 web test qua**, debug **173 file**. Test mới gồm answer một lần/close trước send/Cancel/lỗi gửi; player target theo đối thủ; normal/single-skill/multi-hit; loại bystander; kết thúc Duel mất quyền đánh; hover cursor, click/chuột phải, safe zone.
- Browser fixture `?scene=ui-check&map=0&x=140&y=125&duelinvite=1`: Đồng ý gửi1 answer và còn0 dialog; Esc tăng đúng1 answer và đóng. Đo viewport1280×720: scoreboard right1268/bottom636, HUD top643.5; không chồng HUD. [Ảnh hộp mời](reports/duel-combat-20261104/invitation.png), [ảnh bảng điểm](reports/duel-combat-20261104/scoreboard.png).
- **Server thật, hai QA riêng:** mời/chấp nhận ở Lorencia → arena64 → di chuyển tới cách đối thủ2 ô. Đánh thường nhận HP damage4/SD29; Fire Ball HP4/SD31; Evil Spirit HP4/SD35. Cả người đánh và người bị đánh cùng nhận packet damage tương ứng. Kết thúc nhận AA03, `canAttackPlayer=false`. Đây là số đo của cấu hình QA, không phải damage cố định trong code. [Trace](reports/duel-combat-20261104/live-server.json).
- QA đã đóng và xoá đủ hai account/character cùng các hàng phụ, còn0 hàng QA. Không sửa MuOptions/DKAD. [Biên bản](reports/duel-combat-20261104/qa-cleanup.json).
- Không restart server trong lượt sửa này. Frontend mới đã publish; hai phiên chơi mở từ trước cần refresh/đăng nhập lại để nhận mã mới và xoá trạng thái Duel cũ.
- Chưa dùng kết quả này để chứng nhận mọi skill PvP hoặc toàn bộ hệ PK/Guild war/Siege. Không đổi map/effect map.

Giữ duy nhất code snapshot latest, bổ sung file controls gốc vào snapshot; cây test và credential QA tạm đã dọn. Log/ảnh/trace được giữ trong thư mục report.
