# W0030 — Loader asset stream có giới hạn và kiểm hash

## Implementation

`platform/assets/load-resource.js` nhận manifest resolver đã validate/provision rõ ràng, fetch/WebCrypto/clock có thể inject. Không tự download asset lúc app mở. API load(canonical,{signal,onProgress}) chỉ trả Uint8Array khi stream đúng declared size và SHA-256 đúng manifest. Không tự bỏ hash để tiếp tục decode, không fallback model khác hoặc lưu cache ngầm.

Giới hạn: mỗi model tối đa64MiB, mặc định2 request đồng thời, tổng declared output reservations64MiB; vượt budget bị reject ngay (chưa queue scheduler). Output preallocate đúng size, copy từng chunk, không gom chunk list rồi nối lần nữa. Reservation không phải giới hạn toàn bộ heap browser: fetch internals và WebCrypto digest có thể tạo thêm bản sao. Chưa đo memory/performance game.

Fetch dùng redirect:error, credentials:omit, cache:no-store. Chỉ nhận status200, không redirected/changed URL, body stream bắt buộc. Content-Length identity nếu có phải khớp; với Content-Encoding, stream fetch đã giải nén nên kiểm decoded bytes thực thay vì nhầm compressed Content-Length. Thiếu header vẫn bị giới hạn theo entry size. Mỗi chunk được kiểm trước copy; stream thiếu/thừa và hash sai đều lỗi.

Một deadline mặc định15s bao fetch+read+digest, không gia hạn theo byte drip. AbortSignal và race deadline kết thúc promise kể cả mock fetch/read không tôn trọng signal. finally dọn timer/listener, abort request, cancel/release reader khi lỗi và giảm reservation. Output tạm bị zero khi thất bại; không cam kết xóa mọi bản sao fetch/crypto. Platform/provider bỏ qua signal có thể còn công việc nội bộ; không thể cưỡng chế giải phóng bộ nhớ ngoài loader.

onProgress báo loaded/total từ byte thực; loaded==total chưa phải checksum verified. Caller chỉ cho decoder dùng sau khi promise resolve. Callback lỗi cũng làm tải thất bại và cleanup. Chưa UI loading hoặc phần trăm tổng nhiều resource.

## Kiểm thử 23/10/2026

```sh
node --test web/tests/resource_loader.test.mjs
npm --prefix web test
```

4 test loader mới dùng WHATWG Response/ReadableStream và WebCrypto SHA-256 thật trong Node, fetch fixture: URL exact-case/options, byte progress, success,404, header mismatch, truncated/oversized/hash sai, compressed header semantics, stalled fetch/read deadline, AbortSignal trước/trong tải, budget/concurrency, callback lỗi và không cache bytes lỗi. Không có HTTP server thực hoặc browser asset load trong đợt này.

Kết quả full suite:54/54 test qua, không skip. Không đổi C++/WASM hoặc generator manifest; test:wasm riêng không chạy lại. Debug/Release build allowlist/cú pháp module kiểm trong full suite. Manifest real285 models từ W0029 chưa tự expose hoặc tải vào canvas.

## TODO / bước tiếp theo

- HTTP staging/route phục vụ đúng manifest và browser integration; verify asset thực qua network.
- Model/texture decoder portable, body class/tier resource mapping, renderer/animation/UI game.
- Scheduler/cache/version lifecycle và progress nhiều asset, performance/memory benchmark.
- Manifest trust/provisioning, identity/profile/keys và MU network chưa hoàn thiện; M02/M03 vẫn PARTIAL.
- Không bật MU server/DB, không đổi Data/config/native production.

Trang hiện vẫn chỉ canvas kiểm tra + overlay Debug, chưa có giao diện game. Bước sau: tách parser model BMD có bounds validation vào portable core, kiểm bằng fixture/asset thật trước upload WebGL.
