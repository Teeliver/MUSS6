# W0091 — Mái nhà, animation chết và render quái

Ngày 2026-10-29. Đã sửa các lỗi xác định dưới đây; toàn bộ hiệu ứng đặc thù chưa đạt parity.

## Mái nhà

Source Main5.2 ZzzObject.cpp:4335–4365 có hai nhánh rõ ràng: Lorencia dùng HouseWall05/06 (type125/126), HeroTile4; Devias dùng type81/82/96/98/99, HeroTile3 hoặc >=10. Web trước chỉ gắn roof=true cho Lorencia và chỉ tính indoor Lorencia. Đã bổ sung cả phân loại object lẫn AlphaTarget Devias, giữ tốc độ Alpha gốc 0.1 mỗi tick25Hz và draw pass roofs sau nhân vật. Những map khác chưa tìm thấy luật tương ứng trong nhánh nguồn này; không tự làm mờ toàn bộ nhà/cây theo cảm tính.

## Animation và render quái

- sampleActionOnceResult trước kẹp theo số key đầy đủ, nhưng clip LockPositions có period giảm1. Cuối death có thể nội suy về key đầu. Đã kẹp đúng terminal key theo period thực để giữ tư thế chết.
- Phần opacity giảm của quái chưa bật alpha blend (trước chỉ roof có fade). Đã áp dụng blend cho material thường có opacity<1, giữ nguyên phép multiply/additive của hiệu ứng, tránh thay đổi Evil Spirit.
- Action126 AT_DAMAGE1 trong nguồn phải chạy MONSTER01_SHOCK5; trước tất cả action khác121 đều bị gán attack3. Đã nhận riêng120/121/126 và bỏ việc biến action không liên quan thành đòn đánh.
- Cập nhật thống kê actor khi corpse hết hạn hoặc Skeleton bị ẩn.

## Kiểm tra

252/252 test client qua; WASM rebuilt cùng các contract hiện hữu. Test Devias kiểm tra đủ type mái, tile trong/ngoài nhà và không áp luật sang map khác; test quái kiểm tra shock, death một lần, giữ xác sau viewport removal rồi giải phóng.

MONSTER-ANIMATION-W0091.json lấy mẫu bằng WASM thật trên207 model duy nhất/391 binding actor thường, tại ba thời điểm mỗi clip0/2/3/4/5/6 hiện có; tọa độ hữu hạn, corpse terminal ổn định,0 lỗi. Không bao gồm player-skinned Skeleton/Cursed Wizard hoặc actor chủ động ẩn. Đây là kiểm tra số học, không chứng minh mọi model/effect đẹp và đúng nguồn.

MONSTER-SPECIAL-FX-W0091.json ghi295 nhánh source có literal MODEL_MONSTER01+N, liên quan234 loại actor, với vị trí và lời gọi particle/sprite/joint/effect/render. Named constants, switch lồng nhau còn cần đối chiếu thủ công; không coi đây là danh sách đầy đủ hay bằng chứng đã port.

## Phần còn thiếu đã xác định

Cursed Lich model132 có4 particle BITMAP_FIRE_CURSEDLICH mỗi tick quanh BoneTransform30 (GMNewTown.cpp:1178), texture Effect/firehik02.OZJ; hiện chưa có hệ particle gắn bone này. Totem Golem model133 tạo nhiều model mảnh tại các bone khi chết; hiện chưa port các mảnh này. Một số boss còn material/render override và projectile riêng. Không thay các hiệu ứng bằng lửa/chớp giả dùng chung và không tuyên bố đã đủ100%.

Bản sửa đã publish sau252 test; cần refresh phiên game để tải WASM và code mới. Không sửa dữ liệu nhân vật, damage hoặc config spawn server trong đợt này.
