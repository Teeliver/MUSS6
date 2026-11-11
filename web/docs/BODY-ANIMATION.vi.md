# W0043 — Animation thân/vũ khí và đo CPU frame

## DONE

- Sampler portable `animation_pose.hpp`: translation lerp, quaternion shortest-path slerp, normalize; chuyển Euler ZYX theo pipeline geometry. Timeline giây độc lập số frame render. Idle rate6.25key/s lấy từ native25×idleKeySpeed0.25, loop bỏ terminal locked frame như native. Không tăng animation speed khi120FPS.
- So trực tiếp Player.bmd/native Apple SIMD ở4action1/2/3/4, timeline240mẫu/action tại120Hz:57.600bone samples qua; quaternion |dot|>0.99999, translation sai số<0.0002. Contract synthetic kiểm shortest rotation, loop/locked, clock lỗi.
- WASM BodyAnimation giữ Player clip,5model và1weapon đã decode. Binding chỉ kiểm khi addPart, không parse BMD/recheck từng vertex theo frame. `geometry_from_pose` dùng mesh immutable, không copy lại mesh source mỗi sample. Body và weapon lấy cùng pose rồi gắn world×grip như W0041.
- Renderer cập nhật buffer positions/normals bằng bufferSubData, giữ VAO, UV, texture, shader/program; không recreate GPU resource theo frame. Test lifecycle/topology/error kiểm tra nhánh update.
- rAF không đặt giới hạn60FPS và không dùng timer cố ép120; tự theo lịch trình duyệt/màn hình. Tab hidden ngừng draw; resume lấy elapsed time, tránh chạy catch-up nhiều frame. Dispose/context lost giải phóng animation handle.
- Debug overlay thêm CPU frame p95 (animation sample + copy/validation + gửi buffer/lệnh render trên CPU). Giới hạn240mẫu và cập nhật chữ1lần/s. Không dùng gl.finish gây stall, không gọi đó là GPU time.
- Tool `tools/benchmark_animation.mjs DATA_ROOT` đo600sample sau30warmup trên timeline120Hz, kiểm pose thật có thay đổi và dùng2124vertex DW+Sword01. Đây là CPU sample/WASM-JS copy, không phải đo FPS/WebGL/GPU.

## PARTIAL / hiệu năng

- Chưa GPU skinning: CPU vẫn tính rigid geometry và copy Float32 buffers mỗi frame, sau đó gửi đỉnh lênGPU. Chưa immutable typed-memory-view/pooled output hoặc skeleton texture shader. Đây là bước có số đo để tối ưu tiếp, không phải cấu hình cuối.
- 120Hz timeline/reference không chứng minh đạt120FPS hiển thị. Chỉ báo số đo thực; cần màn hình/trình duyệt đủ tần số và profile GPU/compositor cùng scene lớn. Ngân sách120FPS là8.33ms cho toànframe, CPU sample chỉ là một phần.
- Hiện idle animation, chưa walk/run/attack/gameplay transition blend0.12s như native. Chưa animation independent của wing/weapon clip, online equipment updates. Bounds panel cố định theo pose ban đầu, chưa compute clip envelope.
- Vẫn technical preview, skin dưới mũ/wing nền đen/hide/Windows parity/gameGUI/network chưa xong. RF idle chưa xác minh.
- Không bật MU server/DB, không sửa Data/native production/config. Kết quả test và số đo cuối ởSTATUS.vi.md, baselineW0043. M02/M03 vẫn PARTIAL.

Số đo cuối (không chạy compiler/test đồng thời):600sample CPU-only p50=0.3593ms,p95=0.3890ms,max=0.7619ms. Browser2lần quan sát60.1/60.0FPS, CPU frame p95=0.90/1.60ms; không có GPU timer hoặc profile dài nên chưa chứng nhận120FPS.83/83npmtest vàtest:wasm qua. Asset staging163file giữ nguyên.
