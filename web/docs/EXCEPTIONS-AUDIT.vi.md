# Điều tra 12 trường hợp khác schema — W0005

Ngày 2026-10-23. Đã bổ sung `world_exceptions.py`, chạy trên đúng 12 file của W0004 và kiểm tra SHA-256 từng file trước khi đọc. Cả 12 khớp hash W0004. Không chỉnh Data, không nới parser production, không bật server/DB.

## Kết quả đã xác minh

| File / nhóm | Phát hiện | Trạng thái |
|---|---|---|
| World43, World64, World67 / TerrainHeight.OZB | BMP 256×256, 24-bit BGR, không nén; đủ 196.608 byte pixel, kích thước BMP khớp file | Audit decode được pixel; chưa có quy tắc chuyển thành height của MU |
| World42/TerrainHeight.OZB | BMP 8-bit, bảng màu 120 entry, pixel bắt đầu offset 538; đủ 65.536 mẫu, còn 546 byte phía sau | Audit decode được index pixel; kích thước BMP khai báo khác file thực, chưa xác nhận offset mà engine MU dùng |
| World11/TerrainHeight.OZB | Header khai báo 24-bit nhưng thiếu 130.046 byte để đọc đủ pixel theo BMP | Chưa có cách decode BMP đầy đủ; chưa sửa hoặc kết luận engine gốc không đọc được |
| World19/TerrainHeight.OZB | Toàn bộ 66.620 byte là zero | Không có header/pixel có thể xác thực theo BMP; ý nghĩa runtime chưa biết |
| World7, World55 / TerrainHeight.OZB | Header không mang BM tại offset wrapper; chưa phân loại format | Chưa decode, không tự coi là bitmap hợp lệ |
| World3/EncTerrain3.obj | 2 record có giá trị NaN | Đã định vị từng record; chưa biết xử lý của engine gốc |
| World8, World9, World32 / .obj | Tổng 4 record scale âm | Đã định vị từng record; chưa xác minh đây là phép phản chiếu hợp lệ hay dữ liệu lỗi |

Báo cáo [world-exceptions-W0005.json](world-exceptions-W0005.json) ghi hash input, thông số header, hash pixel đã decode, chỉ số/offset của mọi record có vấn đề. Float NaN được biểu diễn bằng chuỗi trong JSON; báo cáo không chứa literal NaN không hợp chuẩn.

Việc decode BGR/index chỉ xác minh cấu trúc pixel. Không lấy kênh màu bất kỳ làm độ cao, không thay NaN bằng 0, không clamp scale âm hay bỏ object để báo đạt. W0004 vẫn là bằng chứng cho schema native; không sửa báo cáo cũ thành “188/188 đạt”.

## Kiểm tra và tái lập

```sh
python3 web/tools/world_exceptions.py --data /Users/angelale/Downloads/MuClientEX603/Data --previous web/docs/world-structure-W0004.json --output /tmp/world-exceptions-new.json
python3 -m unittest discover -s web/tests -v
```

Output phải là file mới. Công cụ từ chối đường dẫn thoát Data, symlink file, file quá 64 MiB và input đã đổi hash so với W0004.

**14/14 test qua, không skip** trong máy hiện tại. Test mới kiểm tra hướng hàng BMP, 24-bit, palette 120, pixel bị cắt, file toàn zero, ghi hết record lỗi, JSON không NaN, Data đổi và đường dẫn thoát thư mục. Chưa test browser, render hoặc gameplay.

## Còn thiếu

M00 vẫn PARTIAL: mới giải thích được 4/8 heightmap ở mức pixel; quy tắc height MU và 4 placement vẫn chưa được xác nhận. Engine Windows EX603 đầy đủ chưa có trong nguồn được cung cấp. 140 BMD bảng, toàn bộ animation frame, font web và baseline visual/browser tiếp tục mở. Không đánh dấu các lỗi resource là đã sửa trong client.
