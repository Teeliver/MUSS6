# Triển khai sau audit — 04-11-2026

## Đã triển khai

1. **Helper:** bộ điều khiển nhận cả kết quả đồng bộ và Promise; bắt lỗi potion/repair/stop, không chồng request đang chờ, thông báo có giới hạn 5 giây. Sau đổi map ngừng automation tới khi server dừng Helper, retry stop cách ít nhất 1 giây. Không thay packet/cooldown GameServer.
2. **Icon:** cache model vẫn serialize công việc GPU và tối đa 96 entry. Build lỗi tự gỡ entry để lần attach tiếp theo thử lại. Đóng cache thu hồi cả renderer đang build. Key tách index/level/Excellent/setOption. Không thêm cache HTTP/đĩa.
3. **Excellent:** lớp additive dùng texture gốc sau level passes; màu `[L,L*.3,1-L]`, `L=sin(seconds*2)*.5+.5`, đúng nhánh `RenderPartObjectEffect` của ZzzObject.cpp. Giữ ngoại lệ wings/cape, chọn mesh nón 39–44 và nón/áo 59–61. Giữ ordinal BMD qua decoder và giữ lựa chọn mesh giữa các frame animation. Bổ sung EXC từ appearance của người chơi khác.
4. **UI tải:** tối đa 3 worker fetch/decode, không còn đợi từng asset nối tiếp. Khi lỗi, chờ tất cả worker đang chạy kết thúc rồi thu hồi ảnh để không rò bitmap đến muộn.

Đính chính audit: useItem/toggleHelper của session hiện tại là hàm đồng bộ; repro Promise chỉ chứng minh giới hạn với adapter bất đồng bộ. Đã sửa diễn đạt ở báo cáo gốc.

## Kiểm tra

- Full web client: **394/394 qua**, chạy cuối ở cây cô lập; [log](remediation-20261104/client-tests.log).
- Nhóm test mới/liên quan: **33/33 qua**, gồm retry cache, eviction/disposal, Helper lỗi sync/async/đổi map, EXC ngoại lệ/mesh qua animation và cleanup UI khi decoder lỗi. [Log](remediation-20261104/targeted-tests.log).
- Build debug: 163 file allowlist, có module cache mới.
- Trình duyệt: cảnh DK +15/EXC render được, khoảng 60 FPS ở lần quan sát. Đã phát hiện và sửa lỗi precision uniform khi link shader; không dùng kết quả test Node để thay kiểm tra GPU.
- Inventory fixture sau khi mở I: 32/32 icon có model resource; khoảng 60 FPS, frame p95 17,2 ms ở lần quan sát. Không thay inventory/tài khoản thật.
- Ảnh bằng chứng: [EXC](remediation-20261104/excellent-preview.png), [inventory](remediation-20261104/inventory-preview.png).

Không coi lượt này là chứng nhận mọi item/class/buff đã giống PC. Chưa port Ancient pulse, các buff chặn effect và toàn bộ nhánh effect đặc thù từng item; lớp EXC bổ sung ở đây chưa bao phủ chúng.

## Sao lưu và dung lượng

- Chỉ giữ **một** snapshot code: `native/runtime/backups/mu-web-code-latest.tar.gz`, đã kiểm tra đọc archive trước khi thay bản cũ. Snapshot chứa các file có trước khi sửa; file module/test/doc mới có thể xóa khi rollback.
- Database: giữ bản không rỗng mới nhất `MuOnline-before-web-20261026-100511.sql`; xóa bản SQL cũ hơn và file rỗng cùng nhóm.
- Nhân vật: giữ snapshot MuOptions mới nhất `MuOptions-before-evil-20261018-093841.sql`; xóa hai snapshot MuOptions cũ hơn.
- Tổng 4 SQL cũ/rỗng đã xóa: 132.514 byte. Không xóa Data/source gốc.
- Log audit/test được giữ; thư mục build/test tạm của lượt audit được dọn sau khi test kết thúc.

## Các đợt còn lại

- [ ] Phân loại 792 placement theo nguồn map, bắt đầu 34/56; kiểm tra animation/effect theo map.
- [ ] Phân loại 22 item chưa mapping; đối chiếu hình/tỷ lệ từng họ item bằng Data/TRS.
- [ ] Hoàn thiện effect skill exact/source/derived theo từng class, gồm damage/âm thanh/animation.
- [ ] Đo load map/RAM trước khi chia vùng/instancing; không giảm chất lượng để lấy FPS.
- [ ] Audit DB sống và startup config; hoàn thiện tài liệu vận hành và gói release gameplay.

Đây là nhật ký tiến độ, các ô chưa đánh dấu vẫn chưa hoàn thành. Không restart backend hoặc thay database trong đợt này.

## Cập nhật đợt 2

Đã xử lý phần ưu tiên map34/56, khôi phục 515 placement, bổ sung các nhánh particle và sửa JPEG padding. Chưa đóng toàn bộ hạng mục map. Xem [báo cáo đợt 2](MAP-PHASE2-20261104.vi.md).
