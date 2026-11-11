# W0049 — Menu kiểm thử model và skill (2026-10-25)

## Đã xong

- Preview Debug có chọn đủ 7 class, 3 cấp chuyển chức, danh sách skill và nút dừng effect tại 0,12 giây.
- Danh sách 386 dòng được tạo lúc `npm run stage:preview` từ SkillList.txt của server local, chỉ xuất index/name/requireClass và hash nguồn sang `debug/preview-assets/skill-menu.json`. Không xuất đường dẫn server hay toàn bộ config.
- Lọc theo CheckSkillRequireClass: requirement khác 0 và tier + 1 >= requirement. Đây chỉ là lọc class/chuyển chức, chưa kiểm level, mana, trang bị hay đã học skill.
- Đổi class bỏ loadout/weapon/armor/outfit/safe cũ để tránh gắn trang bị class trước. Chọn skill lưu vào URL và nạp lại preview. URL motion cũ được giữ khi chưa chọn skill.
- Fire Ball (4) dùng cast + Fire01 prototype hiện có cho DW/MG/Summoner. Các skill khác ghi rõ chưa có effect và chỉ hiện model idle; không lấy Fire01 thay thế.
- Module menu chỉ được đưa vào build Debug. Release không chứa module này hoặc asset preview. Đây là công cụ test, chưa phải GUI game gốc; tên skill lấy từ snapshot server, chưa nối Language client.

## Kiểm thử

- 92/92 npm tests qua, bao gồm lọc class/tier, dữ liệu sai, chuyển URL và không thay effect sai class; build Debug/Release qua.
- Staging 175 asset gốc và snapshot skill hoàn tất, không sửa Data nguồn.
- Browser: đổi DW -> DK loại Fire Ball; chọn Falling Slash hiện trạng thái chưa có effect; đổi về DW, chọn Fire Ball và bật dừng effect thành công. Ảnh lưu tại `web/.local-baselines/2026-10-25-w0049/preview-menu.png`.
- Quan sát ảnh cuối 60 FPS, CPU frame p95 1,20 ms; đây là mẫu quan sát, chưa chứng nhận ổn định 60/120 FPS.

## Còn thiếu

- Animation/effect/sound riêng của từng skill cho 7 class; Fire01 vẫn là prototype, chưa xác nhận Windows visual parity.
- Skill.bmd/localization, điều kiện đầy đủ, targeting/hit/damage/cooldown/server authority.
- Profile spike DL, xác minh RF idle, skin dưới mũ/wing/hide và GPU skinning/pooling.
- M02/M03 vẫn PARTIAL. Không bật MU server/DB trong đợt này.

## W0050 — 2026-10-25: preview một nhân vật

- DONE: bỏ hai mẫu Sword01/Wing01 hiển thị độc lập; không tải/decode/render chúng khi mở preview. Nhân vật được render ở một viewport duy nhất, không còn chia ba cột. Chọn class/skill qua menu hiện có.
- 92/92 npm tests và build Debug/Release qua; staging lại asset Debug.
- PARTIAL: menu vẫn ghi rõ skill chưa có effect; thay đổi này không bổ sung effect riêng hay xác nhận parity.

- W0050 bổ sung: bỏ góc quay phụ thuộc thời gian tuyệt đối của browser; khóa góc nhìn mặc định π + 0,4 rad khi reload/đổi class/skill, animation nhân vật vẫn chạy.

## W0052 — 2026-10-25: Energy Ball preview

- DONE: skill17 có prototype riêng chọn trong menu, không dùng Fire01 thay thế. Sprite gốc Effect/Thunder01.OZJ, additive/no depth write, billboard theo góc camera cố định. Kích thước0,6world unit, cao1unit, tốc độ15unit/s theo native/client/world_view.mm nhánh skill17. Đích giả3unit, lifetime0,2s, lặp2s chỉ là lịch kiểm thử. Body dùng cast146/147 adapter hiện có.
- Staging thêm đúng1texture, tổng176asset, hash/exact-case giữ nguyên. Manifest Python/JS cho phép filename an toàn trong Effect, vẫn chặn traversal/nested path. Release không stage asset này.
- 96/96 npm tests và build Debug/Release qua; 2 Python resource manifest tests qua. Kiểm numeric kích thước/height/speed/billboard/lifetime/buffer identity; menu kiểm classfilter và chọn energy thay vìfire.
- Browser DW tier2 chạy Energy Ball và bật freeze0,12s; không thấy lỗi render trong lượt kiểm.
- PARTIAL: nguồn đang chuyển là adapter native hiện có, chưa xác nhận visual parity Windows100%; genericcast, chưa sound/target thật/hit/damage/mana/cooldown/server. Các skill khác và đầy đủ7class vẫn TODO. M02/M03 chưa hoàn tất, chưa đảm bảo60/120FPS.
- Không bậtMUserver/DB hoặc sửaData/native production/config.
