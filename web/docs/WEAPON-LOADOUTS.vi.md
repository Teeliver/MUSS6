# W0045 — Cung Elf, staff DW và khiên

## Đã làm

- `src/preview/loadouts.js` tập trung fixture, thứ tự slot và class; model path lấy từ core item resolver, không ghép tên file thủ công trong renderer. Query `loadout=bow&class=2` đặt Bow01 ở slot1 và Arrows02 ở slot0; `loadout=staff-shield&class=0` đặt Staff01 ở slot0 và Shield01 ở slot1. Không trộn với query weapon cũ; class/name sai bị từ chối.
- Item.bmd thật xác nhận Staff01 là một tay, Sword01 một tay, Bow01 hai tay; stance dùng policy và bytes Item.bmd hiện có. Đây không phải kiểm tra toàn bộ equip requirements/eligibility.
- BodyAnimation hỗ trợ tối đa3 attachment, từ chối slot trùng; từng attachment cùng pose với thân và giữ grip/bone riêng. `setWeapon` cũ giữ chức năng thay một món, validation trước khi thay state. Không tái decode theo frame.
- Audit hai loadout đọc model/texture thật, thử sample idle/walk/run, kiểm slot trùng bị từ chối. Tổng33 cấu hình base/armor/mixed/loadout được kiểm tra; staging chỉ các file cần thiết,172 asset, Data không sửa.
- Native geometry comparison mở rộng24 trường hợp: Bow01/Arrows02/Staff01/Shield01 ×3action tương ứng ×safe/combat. Dùng decoder+SIMD+grip native so từng corner, tolerance0.002 world unit; cùng12case Sword01 trước đó đều qua.
- Browser đã xem Elf cung/tên và DW staff/khiên với motion=cycle. Screenshot tại `.local-baselines/2026-10-24-w0045/`. Bó tên gắn lưng theo policy, không lấy nhầm bone tay.
- Tool benchmark thêm `--loadout=bow|staff-shield`, dùng core resolver, Item.bmd và hai attachment thật;6000 CPU samples/mỗi fixture, timeline120Hz. Không coi timeline120Hz là120FPS hiển thị.

## Kiểm thử và số đo

Kết quả cuối test/build ghi STATUS.vi.md. CPU benchmark ngoài browser: bow2193vertex p50=0.2610ms,p95=0.3266ms,max=1.2033ms; staff-shield2424vertex p50=0.2562ms,p95=0.3144ms,max=1.5163ms. Đây chỉ sample/owned WASM-JS copy, không gồm WebGL/GPU/compositor và không dùng để so cải thiện tuyệt đối với fixture khác.

Browser bow ở1506intervals:60FPS,CPUframep95=1.70ms,rAFintervalp95=17.60ms. Staff-shield ở3600intervals:60FPS,CPUframep95=2.30ms,intervalp95=17.20ms. Hai lần quan sát chưa chứng nhận ổn định60FPS hay đạt120FPS; chưa có GPU timer/scene lớn.

## Chưa hoàn thiện

- Chỉ fixture technical preview, chưa inventory/CharSet/live equip, class/strength requirements hoặc server authority. Chưa bắn tên/attack/cast/skill/projectile, âm thanh và damage.
- Loadout safe/back có numeric reference; browser mới chụp combat/cycle. Chưa thử mọi loại bow/crossbow/staff/shield hoặc upgrade/Excellent effects. Wing vẫn đứng riêng và có nền đen; skin dưới mũ/hide chưa sửa.
- CPU geometry skinning/copy vẫn còn; chưa GPU skinning/pooling, camera/GUI game, network MU hoặc chứng nhận Windows visual parity. RF idle chưa xác minh.
- Không bật MU server/DB, không sửa Data/native production/config. M02/M03 vẫn PARTIAL.

## Tiếp theo

Tích hợp attack/cast clip và transition theo source, rồi mới projectile/effect với dữ liệu skill gốc; tiếp tục giữ các lỗi skin/wing và mục tiêu frametime trong TODO.
