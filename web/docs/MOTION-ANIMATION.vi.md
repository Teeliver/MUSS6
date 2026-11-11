# W0044 — Idle/walk/run và chuyển tư thế

## DONE

- `motion_animation.hpp` theo world_view.mm: idle period/1.2key/s (chu kỳ1.2s), walk0.33×25=8.25key/s, run0.34×25=8.5key/s với movementRate=1. Profile này khác idle preview/lobby6.25key/s ởW0043; chỉ dùng khi bật query motion.
- Blend0.12s từ pose đang hiển thị sang pose target bằng lerp+slerp dùng chung W0043; giữ clipTime khi walk↔run, reset khi idle↔gait. Clock theo giây, không tăng speed vì120FPS.
- Native world chuẩn hóa root X/Y theo pose idle cơ bản, giữ rootZ. Configure motion port bước này trước khi sample, tránh root track tự kéo nhân vật đi ngang; body+weapon cùng pose/root reference. Preview đi/chạy tại chỗ, không phải world movement/pathfinding.
- BodyAnimation preload idle/walk/run và reference idle class một lần khi configure; không parse BMD theo từng frame. Motion và static idleW0043 cùng dùng mesh/GPU cache đã có.
- Query Debug `motion=idle|walk|run|cycle`. Cycle3giây/mode chỉ là lịch test, không phải native input/gameplay. Action walk/run từ equipment policy+Item.bmd khi có weapon, hoặc class cơ bản; safe dùng base class actions. Overlay báo mode hiện tại.
- Đo rAF interval p95 trên cửa sổ tối đa3600mẫu, bên cạnh CPU frame p95. Hidden không lấy khoảng nghỉ làm frametime. Đây là cadence trình duyệt, không GPU time; sample count không đồng nghĩa FPS120.
- Contract kiểm blend giữa/chốt0.12s, giữ phase đi↔chạy, reset về đứng, rootXY/Z, clock lùi, kết quả cùng thời điểm ở60/120Hz. Contract chạy host/WASM; sampler quaternion W0043 tiếp tục reference-test.
- Benchmark `benchmark_animation.mjs DATA --motion`:6000samples sau30warmup, timeline120Hz khoảng50s, cycle idle/walk/run. CPU-only chạy nhanh hơn thời gian thực, không thay thế browser profile wall-time.

## PARTIAL

- Chưa movement trên map, run eligibility/safe-zone RunPolicy, input/mouse/path/speed/network; không tự gán demo cycle thành gameplay.
- Chưa transition sang attack/cast/death hoặc đổi equipment/zone khi đang chạy; chỉ3mode đã khai báo. movementRate cố định1 ởfixture, chưa nối profile/config.
- Skin dưới mũ, wing nền đen/hide, GPU skinning/pooled output, gamecamera/GUI/MU network và RF idle/Windows parity còn thiếu. Bounds vẫn theo initialpose; cần clip envelope ởscene camera thật.
-120Hz synthetic timeline không chứng minh120FPS hiển thị. Bản thân rAF hiện không có hardcap60; kết quả còn phụ thuộc trình duyệt, màn hình và workloadGPU. Không thể kết luận GPU headroom từ CPU p95.
- Không đổi Data/native production/config, không bật MU server/DB. M02/M03 vẫn PARTIAL.

## Validation

Kết quả build/test/CPU benchmark/browser frame intervals cuối ghi STATUS.vi.md, baselineW0044. Bước sau: mở rộng visual fixtures bow/staff/shield và chuyển động/action theo metadata, tiếp tục đo khi scene phức tạp hơn.

Kết quả cuối: 84/84 npm test và test:wasm qua. Benchmark CPU 6.000 mẫu: p50 = 0,3689 ms, p95 = 0,4291 ms, max = 0,8749 ms. Browser sau 3.600 interval: FPS 60,0, CPU frame p95 = 1,70 ms, rAF interval p95 = 17,70 ms. Lần trước ở 2.749 mẫu ghi 1,50/18,60 ms. Chưa xác nhận 60 FPS ổn định hay 120 FPS. CPU frame không gồm sort metrics, DOM update, GPU/compositor. Screenshot và AX lưu ở baseline W0044. Staging giữ 163 asset; MU server/DB không bật.
