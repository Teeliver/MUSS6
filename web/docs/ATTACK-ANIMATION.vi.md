# W0046 — Chọn clip đánh và chuyển tư thế

## Đã làm

- Port `equipped_attack` từ nhánh chọn attack trong `native/client/character_resources.mm`: tay không38, vũ khí một tay39/hai tay43, spear46/47, bow50, crossbow51, staff hai tay148 và vũ khí chỉ tay trái41. Giữ ưu tiên bow tay trái như native. Đây là chọn action, không phải damage/attack eligibility.
- `equipmentActionsResult` trả thêm attack, dùng twoHand từ Item.bmd đã kiểm checksum. Test host trích trực tiếp nhánh native và so attack cùng 573.510 tổ hợp đã dùng cho idle/walk/run; không tự tái viết reference.
- MotionAnimation hỗ trợ mode3, dùng cùng tốc độ native world: period/0,6 giây; giữ blend0,12 giây, reset phase khi chuyển gait↔attack. Chỉ gait đi↔chạy giữ phase. Root X/Y tiếp tục chuẩn hóa theo idle class.
- `BodyAnimation.configureCombat` preload idle/walk/run/reference/attack một lần. Thân và tất cả attachment dùng cùng pose; không thêm decode vào vòng render, không tạo lại GPU resource theo frame.
- Debug `motion=combat` là lịch thử đứng3s → đi3s → chạy3s → đánh0,6s. `motion=attack` lặp clip đánh để quan sát. Đây là lịch fixture, không mô phỏng packet/server attack; không tạo damage hay projectile.
- Audit Data loadout cung/tên và staff/khiên nay thử cả attack rồi chuyển về idle, xác nhận clip có trong Player.bmd. Không cần thêm asset; staging172file giữ nguyên.
- Numeric geometry reference nhóm bow/arrows/staff/shield mở rộng32case, gồm các attack clip50/39, cùng12case Sword01 trước đó. Dùng decoder, SIMD và grip native, tolerance0,002 world unit. Contract motion thêm chuyển vào/ra attack và nhịp0,6s.
- Benchmark thêm `--combat`, dùng cùng loadout/Item.bmd thực và timeline120Hz; số đo cuối ghi STATUS.vi.md.

## Chưa hoàn thiện

- Chưa cast/skill selector, projectile, effect, sound, hit event hoặc damage server. Chưa đánh mục tiêu, click/targeting, range hay attack speed theo chỉ số. Nhịp0,6s chỉ theo native preview world hiện có, chưa tích hợp nhịp tấn công server.
- `motion=attack` lặp là công cụ kiểm tra hình, không phải auto-attack. Chưa one-shot state theo xác nhận server, hủy đòn hoặc blend đổi equipment trong lúc đánh.
- Chưa xác minh đầy đủ Windows visual parity. Source Main extension chưa có thân mọi hàm engine; reference hiện là adapter native được cung cấp. Skin dưới mũ, wing/hide, GUI/network, GPU skinning/pooling và RF idle còn mở.
- 120Hz timeline không chứng minh120FPS. CPU benchmark không gồm WebGL/GPU/compositor; cần giữ theo dõi frametime rAF và scene thật. M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không sửa Data/native production/config.

## Tiếp theo

Chọn cast clip và chuẩn bị nguồn effect/skill từ Data/source; sau đó mới nối projectile/hit và server authority. Không đánh dấu combat hoàn tất chỉ vì clip đã phát.

Kết quả cuối: 86/86 npm test, rebuild WASM và test:wasm qua. Audit staging172asset qua. CPU benchmark6000mẫu bow/staff-shield p95 lần lượt0,3385/0,3959ms, max2,2608/1,3457ms; không gồmWebGL/GPU. Browser Elf attack khoảng60FPS,CPUframep95=1,90ms,intervalp95=17,20ms ở1388mẫu. Chưa xác nhận120FPS. Screenshot và log lưu baseline2026-10-25-w0046.
