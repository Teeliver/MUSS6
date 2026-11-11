# Toolchain WASM — W0009, 2026-10-23

## Đã cài và chạy được

Emscripten Homebrew formula 6.0.13, compiler tự báo `6.0.13-git`; build script pin chính xác giá trị này. Dùng Python 3.14.6 và Node 26.11.1 hiện có, không nâng cấp hai runtime này.

Lần cài Homebrew đầy đủ thất bại vì hết dung lượng khi giải nén. Đã xóa riêng gói Emscripten 298 MB tải bởi lần thử đó; không dọn cache khác/source/Data. Sau khi người dùng giải phóng dung lượng, cài riêng formula bằng `brew install --ignore-dependencies emscripten`. Homebrew cảnh báo cách cài bỏ dependency không được hỗ trợ; post-install không hoàn tất vì launcher trỏ Python 3.15 chưa có.

Vì vậy **không gọi global `em++` là đã hoạt động đầy đủ**. Công cụ dự án gọi trực tiếp em++.py bằng Python đang có, tạo EM_CONFIG với LLVM/Binaryen trong SDK và Node hiện tại; EM_CACHE nằm trong thư mục output bị ignore. Cách này đã build thành công đúng phạm vi core/embind/contract đang dùng. Chưa xác minh Closure/Java hoặc các công cụ tùy chọn khác. Không sửa launcher Homebrew hoặc tạo symlink giả Python.

## Phạm vi implementation

`client-core/frame_reader_bindings.cpp` bọc core framing qua Embind. Input Uint8Array, output array Uint8Array có bản sao riêng; phải gọi `.delete()` khi hủy reader. Core không đổi semantics W0008. Đây là bridge nội bộ chưa nối vào trang diagnostics hay transport.

`web/tools/build_wasm.mjs` build hai target: module ES6 `frame-reader.mjs` + `.wasm`, và cùng `frame_reader_contract.cpp` sang `contract.cjs` + `.wasm`. C++ exception được bật vì core dùng throw/catch; memory growth có giới hạn 128 MiB. Output/cache/config đều nằm ở `web/.local-baselines/2026-10-23-m02-009/wasm`, không đưa vào dist tự động.

## Lệnh tái lập

```sh
npm --prefix web run build:wasm
npm --prefix web run test:wasm
npm --prefix web test
npm --prefix web run build
```

Script tìm SDK qua `brew --prefix emscripten`; có thể đặt `EMSCRIPTEN_ROOT` tới thư mục chứa em++.py/llvm/binaryen và `PYTHON` tới executable Python tương thích. Không chạy npm install. Không dùng trực tiếp global em++ đang thiếu Python 3.15.

Kết quả thực:

- WASM build thành công cả module bridge và contract.
- Cùng source C++ contract qua trong native (npm test) và WASM (Node). Đây là fixture tổng hợp, không phải capture login hoặc crypto thực.
- Bridge WASM qua kiểm tra mọi vị trí cắt stream, C1/C2/C3/C4 nối liền, copy ownership sau hủy reader, type/length lỗi, failed session và EOF giữa frame.
- npm test **3/3 qua**, build web tĩnh **4 file** vẫn qua.
- [Metadata và hash artifact](wasm-build-W0009.json).

## Chưa hoàn tất

Chưa kiểm tra Safari/Chrome hoặc WebGL2 thực. Chưa đưa module vào browser runtime, chưa có lifecycle/exception adapter hoàn chỉnh cho transport; bridge hiện dùng cho test. Crypto, ClientVerify, serial, state, gateway/login vẫn chưa triển khai. Không từ kết quả framing suy ra game đã kết nối hoặc đạt 60 FPS.

M02 vẫn PARTIAL. M00 và baseline visual/browser M01 vẫn mở. Không thay Data hoặc production; MU server/DB không bật.
