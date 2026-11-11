# W0069 — item, màu nâng cấp và source bổ sung

Ngày 2026-10-26. Đang tiếp tục kiểm tra giao diện; không xác nhận parity 100%.

## Hai folder mới

- `/Users/angelale/Downloads/SOURCE-2.0.7/Main5.2/source` có mã cần dùng: `ScriptItem.cpp/.h`, `NewUIInventoryCtrl.cpp`, `NewUIMyInventory.cpp`, `ZzzInventory.cpp`, `ZzzObject.cpp`, `ZzzBMD.cpp`.
- `Release2/ScriptItem.obj` là bản biên dịch, không thay thế bảng cấu hình runtime.
- `ScriptItem::OpenData` đọc `Data/Local/ItemTRSData.bmd`, record 32 byte: index, X/Y/Z, angle[3], scale; khóa 0xE2F1. Tìm tên file toàn Downloads chưa thấy bảng này. Không suy diễn các giá trị riêng từng item từ file obj.

## Đã triển khai

- Nền từng ô có item và màu độ bền theo ngưỡng source; màu Zen tại 100.000, 1.000.000, 10.000.000.
- Canvas item 3D, hover quay; click nhấc có item theo con trỏ, Esc hủy nhấc. Đặt đồ vẫn chờ ACK server; trang bị/túi mở rộng vẫn chỉ xem.
- Kích thước canvas Width×20, Height×20 dùng chung túi/trang bị. Ô trang bị căn theo width/height trừ 4, x cộng 1; armor dịch lên 10 theo `RenderItemEquipment`.
- Góc mặc định từ `FindItemRelative`: nhóm 0–4 dùng 180/270/15 (bow -90 tương đương 270), nhóm 5 trở đi dùng 270/-10/0. Sửa staff trước đây bị gom với kiếm.
- Các lớp màu/texture nâng cấp theo tier +3/+5/+7/+9/+11/+13; Chrome01, Chrome02, Shiny01 từ Data. Áp dụng cho icon và mesh nhân vật có ánh xạ item.
- Bảng màu trích từ ba hàm PartObjectColor trong source bổ sung, có script tái tạo và test. Chưa có custom GMProtect overrides hoặc monk remap.

## Giới hạn cần giữ mở

Kích thước ô đã theo source; renderer vẫn auto-fit geometry. Chưa có ItemTRSData nên vị trí/scale riêng từng item và phép chiếu RenderObjectScreen chưa khớp hoàn toàn. Các ngoại lệ model, hiệu ứng particle, custom màu, glow riêng từng mesh chưa port đủ. Không coi toàn bộ +level effect đã hoàn chỉnh chỉ vì đã có lớp chrome/metal.

## Kiểm tra

159 test client qua sau chỉnh, gồm góc staff và offset armor. Build cách ly debug 82 file / release 31 file thành công; stage 179 asset debug. Đã cập nhật server nền, backup W0069-20261026-211029.

Browser cảnh ui-check OFFLINE: quan sát nền ô, model trang bị và Kris +9 có lớp màu; click nhấc Kris hiển thị model tại con trỏ; Esc phục hồi ô gốc với thông báo không gửi packet. Ảnh: /tmp/mu-w69-offline-items.png và /tmp/mu-w69-offline-picked.png. Chưa xác minh lại ACK online trong đợt này; chưa đo hình ảnh hover ở hai thời điểm hay đối chiếu Windows cùng item.


## Sửa regression trong W0070

Fallback staff 270/-10/0 từ Main5.2 không phù hợp với Staff06 của Data hiện tại khi thiếu bảng ItemTRSData: trục dài bị xoay vào chiều sâu. Đã khôi phục hướng staff trước W0069 trong inventory, áp HideSkin theo source để không hiện đầu người trong Bone Helm, và bỏ cắt canvas theo khung ô. Xem ITEM-TOOLTIPS-W0070.vi.md; chưa thay tỷ lệ vũ khí ngoài map.
