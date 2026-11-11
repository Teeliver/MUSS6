# Map — lửa Elbeland và kiểm tra effect gắn bone — 04-11-2026

Tiếp tục báo cáo tổng hợp, xử lý các emitter bị ẩn mà chưa có hiệu ứng và kiểm tra yêu cầu bone bằng BMD thật. Không coi mọi policy đã viết là hiệu ứng đã hiện được với bộ Data đang có.

## Hoàn thiện được với Data hiện tại

**Elbeland/map51:** `GMNewTown::RenderObjectVisual` type0 tạo TRUE_FIRE subtype0, type61 tạo TRUE_BLUE subtype0, cùng xác suất1/3. Data có27 marker0 và23 marker61. Hai loại này đã bị HiddenMesh=-2 từ trước nhưng web chưa phát particle; nay đủ50 emitter.

Texture đúng `Effect/fantaF.OZJ` và `Effect/fantaB.OZJ`, clamp theo ZzzOpenData. ZzzEffectParticle subtype0:life24, scale giữ placementScale, velocityX=(rand10−5)×0,4world/tick,velocityZ=(rand10+5)×0,2world/tick. MovePosition theo góc placement trước cập nhật;scale−0,02,velocityXY×0,95,Z+1world,RGB=life/25. Màu đỏ/xanh nằm trong texture; không nhuộm lại bằng màu mô phỏng. Không dùng TRUE_FIRE subtype5 life20 của Crywolf thay cho subtype0.

**Ánh sáng Empire:** mở rộng type37 theo cảbốncontroller/sáuscene69–74. Data hiện chỉ đặt loại này trên72/73 nên số sprite thật không tăng ở những map không có placement. Type12 có hai sprite mỗi bone2/3: đỏ scale0,5 cố định và trắng pulse abs(sin(time×0,0008))×0,9+0,1,scale=ownerScale×0,3×pulse. Offset(8,−3,−3)/(3,−3,−3,5). Hai nhánh BITMAP_SHINY+5/+6 cuối cùng đều bind Shiny05 trong ZzzOpenData; dùng đúng file Data `Effect/shiny05.OZJ`. Data hiện không có placement12; nhánh này kiểm tra bằng policy/test, chưa có bằng chứng hình ảnh model thật.

## Lửa gắn bone: đã có pipeline, đang bị chặn bởi model cụ thể

Source GMEmpireGuardian1/2/3/4 type50 yêu cầu bones2…7, offset(0,0,5); mỗi bone phát hai HIK3_MONO subtype4:

|Lớp|Scale|RGB|
|---|---|---|
|Xanh|placementScale×0,6|(0,05;0,1;0,3)|
|Trắng|placementScale×0,3|(1;1;1)|

Texture `Effect/firehik_mono03.OZJ`. Lifetime34…38;scale(rand72+72)×0,01×inputScale;rotationrand360;gravity(rand10+40)×0,1world;alpha0. Tăngalpha0,2/0,3 khi đủtuổi, fade0,2 khi lifetime<15;shrink(rand4+5)×0,01,Z+gravity,rotation+3độ. Không mượn subtype2 có vận tốc ngang.

Provider bone nay chia sẻ một rig/pose25Hz theo type giữa sprite, terrain light và emitter; áp owner transform một lần. Particle dùng chung renderer/pool3000 của native map particle, không mở particle renderer thứhai. Validate toàn bộ chỉ số bone trước khi phát; texture/model lỗi được báo, không tự dựng bone hoặc thay model.

**Bị chặn thực tế:**map72 có1placement type50; `Object73/Object51.bmd` trong Data hiện có **1bone**, không có2…7 như source yêu cầu. Các file cùng tên trongObject70…74 cũng chỉ1bone;Object75 không có file. Không tìm thấy bản Object51 thay thế trong `file thieu`. Runtime bỏ riêng12emitter của placement này và giữ model/các effect khác. Đây là lệch giữa bộ source và Data, không thể sửa đúng bằng xoay hoặc lấy bone0 thay thế.

Cần đối chiếu đúng BMD của object50 từ bộ client tương ứng, xác nhận mapping và cập nhật catalog/manifest trước khi bật effect này. Không ghi đè Data gốc bằng tài nguyên đoán.

## Audit và kiểm chứng

- **474/474 test web qua**, debug172file publish.9test mới kiểm tra recipe/màu/scale, lifetime/fade riêng, texture Data thật, chia sẻ pose, giới hạn pool, giữ cảnh khi bone sai, dispose và cleanup khi nạp effect khác thất bại.
- Tool mới `audit_map_bones.mjs` quét đủ47map,7nhóm map/type có recipe thực:6pass,1blocked(type50/map72). Lấy mẫu bone tại0/0,2/1/10giây. Đây chỉ là các recipe đã port, không phải kiểm tra mọi bone của mọi effect PC.
- Elbeland tại27,219:101particle TRUE_FIRE/BLUE,162bone sprite,50emitter;dropped0,lỗi bone0. Mẫu60FPS,framep9516,9ms,CPU p955,8ms,buffer1582×1798. Không phải benchmark dài hạn.
- Browser lửa đỏ Elbeland23,232:74particle,60,1FPS,CPU p957ms,dropped0. Empire72 tại5,196:boneIssues1,boneParticleEmitters0,20bone sprite còn hoạt động,60FPS,CPU p956,9ms; lỗi không dừng cảnh. [Số đo](map-bones-20261104/browser-samples.json).
- Hồi quy Market79 tại108,126:20bone sprite,55flame,292water,dropped0,lỗi bone0,60FPS,CPU p955,9ms.
- Geometry/resource ledger cũ vẫn254placement unresolved; lệch bone được ghi riêng, không cộng lẫn hoặc làm giảm số thiếu bằng cách giấu model.

[Audit bone](map-bones-20261104/bones.json) · [Effect inventory47map](map-bones-20261104/effects.json) · [Log test](map-bones-20261104/client-tests.log) · [Ảnh lửa xanh](map-bones-20261104/elbeland-blue.png) · [Ảnh lửa đỏ](map-bones-20261104/elbeland-red.png)

## Chưa hoàn tất

Joint spark20/51, chrome đỏ hai pass của64, projectile/event/action và render-after-character vẫn còn trong backlog tổng hợp. Map51 cloud60/TRUE_BLUE61 đã phân biệt: chỉ61 đã hoàn thiện lượt này;cloud60, các effect khác chưa được coi là xong. Bốnmap terrain thiếu texture và254placement unresolved trong báo cáo tổng hợp vẫn còn.

Không sửa DB/account/Data, không restart server. Giữ một archive code mới nhất trước lượt sửa; backupDB riêng giữ nguyên. Dọn cây test tạm sau khi lưu log.
