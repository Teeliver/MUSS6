# Audit cấu trúc toàn bộ world — W0004

Ngày 2026-10-23. Tiếp tục M00.3b bằng công cụ `web/tools/world_structure.py`, chỉ đọc Data. Không thay đổi parser production, resource, DB hoặc trạng thái server.

## Đối chiếu nguồn

Đã tìm lại trong cây source được cung cấp: phần Main_EX603 là extension; chưa có implementation loader model/terrain của engine Windows EX603. GameServer/Map.cpp đọc bản đồ server, không thay thế loader hình ảnh của client. Không có đủ bằng chứng để sửa tên Object44/Object166 hay tự coi type chưa ánh xạ là một model khác.

Schema của đợt này được ghi nguồn rõ ràng: `native/client/assets.cpp`, các hàm `decrypt_file`, `load_terrain`, `load_world_objects`. Công cụ kiểm tra schema trên tất cả thư mục WorldN, không áp giới hạn chỉ 5 world của API native. Đây là **audit cấu trúc**, không phải bổ sung hỗ trợ 42 map vào game hoặc xác nhận đúng engine Windows.

## Kết quả

| Loại | Qua schema | Cần đối chiếu |
|---|---:|---:|
| Mapping .map | 47 | 0 |
| Attribute .att | 47 | 0 |
| Placement .obj | 43 | 4 |
| Height .OZB | 39 | 8 |
| Tổng file | 176 | 12 |

35/47 world có cả bốn file qua schema. Các file có SHA-256 tại thời điểm đọc trong [báo cáo từng file](world-structure-W0004.json). Đây không phải lần hash lại toàn Data.

Các trường hợp chưa qua:

- World3/EncTerrain3.obj: giá trị float không qua giới hạn native.
- World8, World9, World32: scale placement vượt giới hạn native.
- Height của World11, World43, World64, World67: thông số bitmap khác schema 256×256 8-bit hiện tại.
- Height của World7, World19, World55: wrapper khác schema hiện tại.
- Height của World42: offset pixel không tương thích quy tắc legacy hiện tại.

Báo cáo ghi chỉ số record/giá trị hoặc thông số bitmap để điều tra tiếp. `schema_error` nghĩa là không qua quy tắc hiện tại, **không kết luận file gốc hỏng**. Không tự clamp scale/float, thay height hoặc dùng resource map khác để làm audit đạt.

Công cụ kiểm tra header/world, record hoàn chỉnh, toàn bộ float placement, scale, kích thước mapping/attribute, header/offset/độ dài pixel height. Nó ghi riêng chênh lệch số record khai báo và khả năng tương thích giới hạn count native. Tile index chỉ được thống kê, chưa ánh xạ tên texture; không tự áp bộ tile Lorencia lên map khác. Không kiểm tra model mapping của 42 world, visual, collision, blend, light hoặc animation.

Đối chiếu với W0003: số record, active placement và số mẫu terrain của 5 world native hỗ trợ khớp báo cáo cũ. Các lỗi model thiếu ở W0003 và lỗi texture/model W0002 vẫn chưa được sửa.

## Tái lập và kiểm thử

Từ Emulator, dùng đường dẫn output mới (công cụ từ chối ghi đè):

```sh
python3 web/tools/world_structure.py --data /Users/angelale/Downloads/MuClientEX603/Data --output /tmp/world-structure-new.json
python3 -m unittest discover -s web/tests -v
```

**10/10 test qua, không skip** sau khi hai CLI W0002/W0003 đã được build. Test mới kiểm tra record bị cắt, NaN, count khác header, 8/16-bit attribute, world header sai, offset/padding height và việc audit cấu trúc không tự bật hỗ trợ renderer. Chưa test browser hay gameplay.

## Trạng thái tiếp theo

M00 vẫn PARTIAL. Đã có bằng chứng cấu trúc cho 47 world, nhưng chưa xác minh hình ảnh/texture/model của 42 world ngoài phạm vi native. Còn 12 trường hợp schema, 140 BMD bảng chưa decode và animation chưa audit mọi frame. Không chờ parity tuyệt đối với engine chưa có rồi báo đã hoàn tất.

Bước kế tiếp: phân tích header/record của 12 trường hợp để tách khác format hợp lệ với giới hạn parser; chỉ thêm decoder khi có bằng chứng và fixture. Những mục thiếu engine gốc tiếp tục ghi BLOCKED parity trong SOURCE-MAP, không làm giả resource.
