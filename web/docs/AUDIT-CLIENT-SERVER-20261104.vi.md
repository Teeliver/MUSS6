# Rà soát MU Web / native server — 04-11-2026

## Kết luận hiện trạng

Dự án có nền tảng chạy được và bộ kiểm tra tương đối lớn; chưa đủ bằng chứng để gọi là client MU đầy đủ hoặc hiển thị giống bản PC 100%. Những khoảng trống đáng ưu tiên là material Excellent, độ phủ resource/effect map, skill effect, phục hồi lỗi tải icon và xử lý bất đồng bộ MU Helper. Backend nguyên bản biên dịch được, nhưng độ đầy đủ của migration database đang chạy chưa xác minh được trong lượt này.

Lượt này chỉ kiểm tra và bổ sung báo cáo; không sửa gameplay, database, tài khoản, trang bị hay restart các server đang chạy.

## Phạm vi và cách đọc kết quả

Đã lập danh mục/hash **2.317 tệp source, 844.876 dòng vật lý**: web client/gateway, client-core, native adapter, GameServer/DataServer/JoinServer/ConnectServer, Main5.2, Main_EX603, GetMainInfo_EX603, GetClientInfo, MHPClient/MHPServer và source bổ sung `file thieu`. Có cả header thư viện trong cây client gốc. Không cộng các bản client EX101/201/301/401/803 vào kết luận EX603; bỏ thư mục build/runtime/vendor/node_modules.

Đã quét **146 Markdown, 10.556 dòng**, trích tiêu đề/giới hạn và kiểm tra đường dẫn: không có link Markdown nội bộ bị đứt tại thời điểm kiểm kê. Danh mục chi tiết: [inventory.json](audit-20261104/inventory.json).

**Kiểm kê toàn bộ không đồng nghĩa đã đọc và chứng minh từng dòng.** Rà sâu tập trung vào renderer item/map/skill/pet, luồng session và packet, giao dịch trang bị/shop, MU Helper, build/startup, adapter server và SQL. Các nhánh sự kiện, tải nhiều người và từng hiệu ứng của từng map chưa được kiểm chứng toàn bộ bằng gameplay thật.

## Kết quả thực nghiệm

| Kiểm tra | Kết quả | Phạm vi kết luận |
|---|---|---|
| Web client, bản sao cô lập | **387/387 qua** | Không thay dist của phiên đang chơi |
| Gateway | **16/16 qua** | Chạy lại với quyền mở socket cục bộ |
| Python audit helpers | **24/24 qua** | Công cụ kiểm kê/kiểm tra |
| Native build sạch | **Biên dịch thành công, 454 bước** | Cả executable server và client native |
| Native test, cây build hiện có | 30 qua; 4 test GPU qua khi chạy lại ngoài sandbox | 34 test tổng; lỗi ban đầu do môi trường Metal |
| Native test, cây build sạch | **30/33 qua** | 3 auto-preflight không tìm thấy default config trong thư mục build mới; explicit-config preflight qua |
| Database metadata trực tiếp | **Chưa xác minh** | Docker info/exec timeout, không có kết quả metadata |
| Source → web đang phục vụ | Revision `dcc950c8f7e157b8` khớp | Không phải lỗi deploy đang giữ mã cũ ở cấp dist; tab đã mở vẫn có thể giữ module cũ |

Log gốc nằm trong [thư mục bằng chứng](audit-20261104/). Test qua không chứng minh pixel/animation giống PC hoặc mọi feature đã hoàn chỉnh.

## Các phát hiện cần xử lý

### 1. P1 — Render Excellent còn thiếu lớp màu riêng theo source

[Item visual policy](../client/src/ui/item-visual-policy.js), hàm `upgradeStyle` dòng 10 và `bindUpgradeMeshes` dòng 13, chỉ chọn màu/lớp theo level/index. Không phân biệt cờ Excellent trong đường material này. Tái hiện: cùng item 3585 +9, Excellent=0 và Excellent=1 trả về style giống hệt nhau.

Source PC `Main5.2/source/ZzzObject.cpp`, khoảng dòng 12770–12814, có nhánh `(Option1 & 63)>0`, màu biến thiên theo WorldTime và lớp additive dùng texture gốc, kèm ngoại lệ wing/cape. Tooltip xanh hoặc nền item không thay thế hiệu ứng material này.

Ngoài ra cache icon tại [item-icons.js](../client/src/ui/item-icons.js), dòng 16, chỉ dùng `index:level`. Khi bổ sung material theo Excellent/Ancient cần đưa những thuộc tính ảnh hưởng hình vào cache key. Không nên sửa bằng cách đổi màu chung mọi đồ + cao.

### 2. P2 — MU Helper cần hỗ trợ cả kết quả đồng bộ và Promise

[helper-controller.js](../client/src/world/helper-controller.js), dòng 10–11: `try { w.toggleHelper(...) } catch {}` và `try { w.useItem(...) } catch {}` chỉ bắt exception đồng bộ. Đính chính khi triển khai: `world-session.js::useItem` và `toggleHelper` hiện trả về đồng bộ; lỗi đồng bộ của chúng đã được catch. Mô phỏng adapter trả Promise reject tạo `unhandledRejection`, nên đây là khoảng trống khả năng xử lý adapter bất đồng bộ, không phải lỗi reject đã chứng minh trên session thật. Nhánh repair trả Promise cũng cần được xử lý nhất quán.

Cần bắt Promise, đồng bộ cooldown/trạng thái và hiển thị lỗi phù hợp; không kết luận đây là nguyên nhân disconnect của mọi phiên. Có thể chạy lại hai tái hiện trên bằng `node web/docs/audit-20261104/repro-findings.mjs`.

### 3. P2 — Icon tải lỗi bị lưu thành Promise lỗi trong cache

[item-icons.js](../client/src/ui/item-icons.js), dòng 16: lưu Promise trước khi build xong; khi thất bại không xóa entry. Những lần yêu cầu tiếp theo tiếp tục nhận Promise đã reject. Tải resource có retry ở lớp dưới nhưng khi hết retry thì icon vẫn có thể trống tới lúc eviction hoặc reload.

Cần xóa đúng entry khi reject, cho phép thử lại có giới hạn và giữ cleanup renderer khi view đã bị tháo. Đây là vấn đề cache RAM của phiên, không phải đề xuất bật cache đĩa/browser dài hạn.

### 4. P1 — Map còn khoảng trống mapping/resource/effect đáng kể

Audit mới trên Data hiện có: **47 map, 98.353 placement, 792 placement chưa giải quyết, 663 marker ẩn/effect, 0 lỗi chạy công cụ**. [Chi tiết từng map/type](audit-20261104/map-coverage.json).

| Map ID | Placement chưa giải quyết |
|---|---:|
| 34 | 294 |
| 56 | 226 |
| 57 | 65 |
| 8 | 49 |
| 38 | 26 |
| 65 | 23 |
| 51 | 21 |

**792 không đồng nghĩa 792 file bị thiếu**: có thể là marker điều khiển hiệu ứng, model chưa mapping, hoặc nhánh đặc thù map chưa port. Marker ẩn cũng không tự động là lỗi. Phải truy về switch/type tương ứng trong source PC để phân loại trước khi sửa.

Các bộ effect hiện tại có policy theo nhóm/type; chưa thể xác nhận mọi waterfall, fire, light, door, roof fade và controller animation của tất cả map đã tương đương PC. Tài liệu [W0088](MAP-RESOURCE-EFFECT-W0088.vi.md) cũng ghi giới hạn này. Không nên đóng hạng mục bằng việc chỉ đếm model tải thành công.

### 5. P1 — Độ phủ skill khác độ chính xác hình ảnh skill

Catalog có 114 active skill: 3 `exact`, 60 `source`, 51 `derived`. Theo chú thích [skill-effect-catalog.js](../client/src/world/skill-effect-catalog.js), `source` nghĩa đã tìm thấy token CreateEffect/CreateJoint/CreateParticle, không có nghĩa đã port trọn controller. Runtime dùng các mode/họ texture dùng chung; `derived` vẫn là cách dựng cùng họ hình ảnh.

Cần kiểm tra riêng packet, damage server, animation nhân vật, particle/joint, thời gian tồn tại và âm thanh. Không nên dùng con số 114 để kết luận toàn bộ hiệu ứng skill giống nguồn. Xem thêm [W0115](SKILLS-FULL-W0115.vi.md).

### 6. P2 — Còn 22 item chưa ánh xạ model

Audit Data: **924 item có tên; 902 item sẵn sàng; 903 biến thể model; 0 file thiếu trong các mapping đã giải quyết; 22 item chưa mapping**. [Danh sách đầy đủ](audit-20261104/item-resources.json).

Ví dụ: Divine Armor, Divine Helm, Grand Soul Pants, Gem of Secret, Chaos Card và Rage Fighter Character Card. Cần phân loại item hiển thị đặc biệt/không dùng trong phiên bản này với item thật sự thiếu mapping. Kiểm tra file tồn tại không chứng minh góc nhìn, tỷ lệ, pivot, mesh ẩn thân hoặc class condition đã đúng cho từng item.

`ItemTRSData.bmd` hiện đã được đưa vào bảng dữ liệu icon; những ghi chú lịch sử nói chưa có bảng này không còn mô tả đúng trạng thái hiện tại. Không nên tiếp tục thay tỷ lệ chung để chữa từng item riêng.

### 7. P2 — Dựng toàn bộ map gây áp lực CPU/RAM lúc chuyển map

[environment.js](../client/src/world/environment.js) mở rộng geometry placement vào các batch/typed array, có ngưỡng 5 triệu vertex và culling khi render nhưng chưa phải tải tài nguyên theo vùng không gian.

Map 4 khoảng **3,11 triệu vertex**, map 3 khoảng **2,93 triệu**. Với position/normal/UV/shadow origin tổng 11 Float32 mỗi vertex, riêng các mảng đó của map 4 xấp xỉ **137 MB thập phân**, chưa tính bản GPU, texture, dữ liệu nguồn và mảng trung gian. Đây là ước tính cấu trúc, **không phải số RAM/FPS đã đo**. Các map audit hiện chưa vượt ngưỡng 5 triệu; không có bằng chứng cap này đang cắt mất chúng.

Ưu tiên đo thời gian fetch/decode/build/upload và peak RAM, sau đó chia vùng, tái sử dụng geometry/instancing phù hợp, giảm bản sao và upload animation. Cache RAM có giới hạn hiện có là điểm tốt nhưng không giải quyết hết chi phí dựng scene.

### 8. P2 — UI assets tải tuần tự

[load-assets.js](../client/src/ui/load-assets.js), dòng 3, `for ... await queue(path)` khiến các request UI tuần tự dù queue có khả năng giới hạn song song. Có thể tăng độ trễ khởi tạo HUD/panel trên kết nối chậm.

Nên dùng nhóm tải song song có giới hạn, giữ cleanup khi lỗi; đo trước/sau. Không tải tất cả tính năng chưa mở nếu không cần. Không đề xuất preload toàn bộ Data hoặc thêm cache lâu dài trái với yêu cầu kiểm thử resource thường xuyên.

### 9. P1 về mức sẵn sàng — Release web chưa chứa game đầy đủ

[build-layout.mjs](../tools/build-layout.mjs) chỉ đưa release.html/css, app/release.js và shared core vào bản release. Luồng game/login/world/UI đang ở debug. Vì vậy bản local chạy được không đồng nghĩa có gói release gameplay sẵn sàng phát hành.

Đây là giới hạn gói sản phẩm hiện tại, không phải lỗi server gameplay. Cần chốt entrypoint/build phục vụ thật trước khi nói tới phát hành.

### 10. P2 — Startup và database cần kiểm chứng lại trên môi trường mới

Build sạch có 3 test auto-preflight thất bại do thiếu cấu hình mặc định cạnh executable; các test cấu hình chỉ định rõ qua. Hướng dẫn phải phân biệt build từ source với runtime đã được chuẩn bị config. Đây là một nguyên nhân có thể làm người dùng thấy “build được nhưng chạy không được”.

Trong `native/db` có **11 định nghĩa CREATE PROCEDURE**. Con số này không chứng minh database sống có đủ procedure/schema: lượt đọc metadata bị Docker timeout. Không dùng số “9/63” trong tài liệu cũ để kết luận hiện tại còn thiếu chính xác bao nhiêu. Cần audit schema/procedure/version ở DB đang chạy trước khi chốt event/guild/reset/persistence đầy đủ.

Adapter bảo toàn gameplay C++ gốc và có kiểm tra framing/ABI; điều đó chưa tương đương kiểm chứng độ dài/count của mọi payload C++ hoặc load test nhiều account. Cần bổ sung kiểm tra theo packet và fuzz/sanitizer tại các luồng giao dịch trước khi mở rộng triển khai.

### 11. P2 — Tài liệu tổng quan chưa theo kịp mã

- `web/README.vi.md` còn mô tả giai đoạn M00, không phản ánh gameplay hiện có.
- `native/RUN-AND-TEST.vi.md` còn số test cũ; cần cập nhật cách build/config/runtime và kết quả mới.
- `web/docs/STATUS.vi.md` đầu file dừng W0117 trong khi đã có các hạng mục tiếp theo và bộ web lên 387 test.
- Các tài liệu migration/ItemTRS có thông tin lịch sử; cần ghi rõ ngày và liên kết trạng thái hiện hành.

Không coi ghi chú giới hạn cũ là bằng chứng lỗi vẫn tồn tại. Nên có một trang “hiện trạng + lệnh chạy đã xác minh” làm điểm vào, còn các Wxxxx giữ vai trò nhật ký.

## Những điểm đang tốt

- Backend tiếp tục dùng gameplay C++ nguyên bản, các adapter native xây dựng được từ source hiện tại.
- ABI/packet có test và dữ liệu sinh từ header; luồng trang bị/shop đã có ACK/state handling, không chỉ dựng UI mô phỏng.
- Có giới hạn RAM/queue/icon, kiểm soát đường dẫn asset, và revision asset để giảm nhầm mã nguồn với dist.
- Resource audit phân biệt model mapping, file thiếu và marker; có thể dùng làm baseline cho lần sửa tiếp theo.
- Bộ test tăng đáng kể; tuy nhiên hai repro mới cho thấy vẫn cần test lỗi bất đồng bộ và các tổ hợp material.

## Thứ tự nên làm tiếp

1. Chữa Promise Helper và cache icon lỗi, thêm test hồi quy có ý nghĩa.
2. Port material Excellent đúng nhánh source; cập nhật cache key và kiểm tra +/EXC/Ancient riêng.
3. Phân loại toàn bộ unresolved map type; xử lý nhóm nhiều nhất 34/56 trước, kèm checklist effect/animation thực tế.
4. Hoàn thành mapping item còn thiếu; kiểm tra visual từng họ bằng Data/TRS, không scale chung cảm tính.
5. Profile chuyển map và tải UI; tối ưu trên số đo.
6. Xác minh DB/config, cập nhật hướng dẫn chạy và gói release.

## Giới hạn xác nhận

Chưa thực hiện walkthrough tất cả map/class/item/pet/wing trên hai phiên thật, chưa đo concurrency/peak GPU, chưa xác minh schema DB sống, chưa chứng nhận mọi nhánh source/hiệu ứng đã port. Các dịch vụ local đang chạy không bị khởi động lại. Báo cáo này là baseline kỹ thuật có bằng chứng, không phải chứng nhận “chuẩn gốc 100%”.
