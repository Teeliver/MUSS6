# Cây thư mục và build modes — W0010

Các thư mục Server/Main/Data gốc được giữ nguyên. Phần mới tách theo trách nhiệm:

```text
client-core/
  include/mu/                C++ thuần: framing, sau này codec/state
  frame_reader_bindings.cpp  ranh giới Embind, không có DOM hoặc socket
  CMakeLists.txt             build native độc lập
web/
  client/
    index.html, style.css    trang Debug
    release.html/css        trang Release, chưa có UI game
    src/session/            discovery/chọn server bất đồng bộ, không DOM
    src/protocol/           adapter codec WASM, kiểm chứng server route
    src/app/                entrypoint Debug (main.js), Release (release.js)
    src/platform/rendering/ WebGL surface, kích thước buffer/DPR
    src/platform/wasm/      loader cache Promise và checked-result reader adapter
    src/platform/transport/ vòng đời phiên WebSocket, timeout, queue và byte order
    src/diagnostics/        FPS, self-test framing; chỉ được build Debug
  gateway/                  chỗ dành cho M03, chưa có dịch vụ
  tools/
    build-layout.mjs        allowlist duy nhất cho từng mode
    build.mjs               đóng gói mode được chọn
    build_wasm.mjs          compiler pin, config/cache cục bộ
    *_audit.*, world_*.py   audit M00; không nhập vào runtime browser
  tests/                    contract C++, Node, Python và WASM integration
  docs/                     ADR, quy ước, trạng thái và bằng chứng từng đợt
  .build/wasm/              output/compiler cache đang dùng, bị ignore
  .local-baselines/          bằng chứng lịch sử theo đợt, bị ignore; không làm dependency build
  dist/debug/               chỉ output Debug, bị ignore
  dist/release/             chỉ output Release, bị ignore
```

Runtime không được import tools/tests/docs/server config. Generated module chỉ được copy từ `.build/wasm` vào `dist/<mode>/generated`. Không đặt source sinh tự động vào client/src. Không đổi cây thư mục resource gốc để khớp renderer.

## Debug / Release

```sh
npm --prefix web run build:wasm
npm --prefix web run build:debug
npm --prefix web run build:release
npm --prefix web test
npm --prefix web run test:wasm
```

Debug có overlay khung render/DPR, trạng thái WebGL2/WASM, FPS đo số lần vẽ thực tế theo requestAnimationFrame, và trạng thái ping server. Chưa có gateway/server connection nên ping phải ghi **chưa kết nối**, không hiển thị giá trị giả. FPS là của canvas kiểm tra, chưa phải benchmark cảnh game. Vòng đo/vẽ dừng khi tab ẩn; renderer không cấp phát lại buffer khi kích thước không đổi.

Release dùng entrypoint và allowlist riêng: không đóng gói thư mục diagnostics, self-test framing, overlay, FPS, ping hoặc CSS khung Debug. Không chỉ ẩn bằng CSS. Canvas full window; do chưa có scene game nên nó hiện nền trống. Lỗi khởi động thông thường vẫn có thông báo chung; validation packet và lỗi core vẫn giữ để đảm bảo hoạt động đúng, không log dữ liệu riêng.

WebGL surface dùng chung không import diagnostics. Debug và Release đều dùng cùng core WASM; bộ contract executable không được copy vào bản phát hành. Release chưa phải bản game hoàn chỉnh.

## Chạy thử

`npm --prefix web run serve` phục vụ riêng `dist/debug` trên 127.0.0.1:8080; `serve:release` phục vụ `dist/release` trên 127.0.0.1:8081. Không chạy cùng port nếu đã có preview khác. Dừng bằng Ctrl+C. Không dùng file:// cho ES module/WASM.

Trong W0010 đã có preview phục vụ cả thư mục dist trên port 8080: mở `/debug/` hoặc `/release/`. Đây là server static local, **không phải MU server hoặc database**. Đã dọn đúng các output sinh tự động cũ ở gốc dist từ W0009; chỉ dùng dist/debug hoặc dist/release để phát hành.

## Kiểm thử

Node test xác minh chính xác toàn bộ file của từng mode, nội dung khớp nguồn và không có diagnostics/marker FPS/ping trong Release. WASM test dùng lại self-test framing mà browser Debug chạy. M00 và các phần M02 còn thiếu vẫn mở; không có claim tối ưu 60 FPS trước khi có cảnh/game tải thật.

### Bố cục theo yêu cầu cuối W0010

Debug và Release đều có canvas 100vw × 100vh, không có trang giới thiệu hoặc panel. Chỉ Debug có overlay chữ 12px cố định top=8px, left=10px, pointer-events:none: FPS, ping, buffer/DPR và trạng thái core/render. Không có viền quanh canvas. Khi có renderer game sau này, giữ bố cục này và bổ sung số đo vào overlay thay vì làm trang riêng.
