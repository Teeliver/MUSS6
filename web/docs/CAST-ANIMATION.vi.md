# W0047 — Cast animation và audit nguồn hiệu ứng

## Đã làm

- Đọc nhánh native world_view.mm: spellVariant ban đầu3; mỗi cast luân phiên4/3, tương ứng combatActions[4]=147 rồi combatActions[3]=146. Tấn công thường đi nhánh5 riêng. Không suy đoán clip riêng cho từng skill khi source hiện có chưa phân biệt.
- Motion mode4/5 dùng nhịp period/0,6 giây và blend0,12 giây như nhánh cast native. `BodyAnimation.configureSpell` preload action idle/walk/run/attack/reference/146/147 một lần; thân và attachment dùng cùng pose. Root X/Y vẫn theo idle class.
- Debug `motion=cast` thử cast147 trong0,6s, về idle1,4s rồi cast146 và lặp. Khoảng nghỉ1,4s là lịch fixture để nhìn rõ chuyển tư thế, không phải cooldown của skill. Không gửi packet hoặc tự học/dùng skill.
- Test lịch cast bắt đầu147, luân phiên146, cửa sổ0,6s; contract motion kiểm vào/ra hai mode. Native pose comparison mở rộng6action, gồm146/147, đạt86.400bone samples tại timeline120Hz: |quaternion dot|>0,99999, translation error<0,0002.
- Audit loadout Data thử cả hai clip cast rồi về idle. Chỉ kiểm khả năng render/binding, không xác nhận class được phép học skill. Staging giữ172asset, chưa đưa Effect/Skill vào web public asset allowlist.
- `tools/skill_asset_audit.mjs DATA_ROOT` là audit chỉ đọc: hai model Skill/Fire01.bmd và Skill/Laser01.bmd, texture tham chiếu từ từng mesh cùng3sprite được chỉ rõ trong native world_view.mm. Ghi exact-case path, size, SHA256, mesh count; tổng8file và không có lỗi ở Data hiện có.
- Các texture tìm được: Skill/fire02.OZJ, fire01.OZJ, dragon02.OZJ; Effect/JointSpirit01.OZJ, Thunder01.OZJ, Spark03.OZJ. Model Fire01 có2mesh, Laser01 có1mesh. Audit chỉ kiểm byte/file và decode geometry/ref, chưa decode/kiểm parity material hình ảnh.
- Benchmark thêm `--cast` để đo sampler/copy với staff/khiên trên timeline120Hz,6000mẫu; không coi đây là FPS hiển thị.

## Chưa hoàn thiện

- Chưa skill-id selector riêng, Skill.bmd catalog/requirements/mana/cooldown, animation dispatch từ packet/server. Cast147/146 phản ánh adapter native hiện có, chưa chứng minh mọi skill Windows dùng cùng quy tắc.
- Chưa hiệu ứng lên màn hình, projectile/quỹ đạo, material additive/dark, timing sound, hit/damage hoặc server authority. Chỉ có audit nguồn hiệu ứng và clip cast. Không gọi là skill đã hoàn tất.
- Chưa phát đồng thời animation riêng của weapon/wing. Skin dưới mũ, wing nền đen/hide, game camera/GUI/network, GPU skinning/pooling và RF idle vẫn còn thiếu. Windows visual parity chưa xác nhận.
-120Hz numeric reference/CPU benchmark không chứng minh120FPS. CPU sample không gồm WebGL/GPU/compositor; tiếp tục báo số đo browser riêng.
- Không bật MU server/DB, không sửa Data/native production/config. M02/M03 vẫn PARTIAL.

## Tiếp theo

Chuyển material và quỹ đạo/thời điểm của một effect cụ thể theo native, bắt đầu từ Fire01; kiểm trong technical preview rồi mới nối skill packet và hit authority. Giữ các lỗi skin/wing trong TODO.

Kết quả cuối:87/87npmtest, buildWASM vàtest:wasm qua. CPU6000mẫu cast staff/khiên p50=0,2526ms,p95=0,3300ms,max=4,7053ms; browser3600intervals ghi60FPS,CPUframep95=1,60ms,intervalp95=17,00ms. Không chứng nhận120FPS hoặc60FPS ổn định. Baseline W0047 giữ JSON, native pose log, asset audit và ảnh preview.
