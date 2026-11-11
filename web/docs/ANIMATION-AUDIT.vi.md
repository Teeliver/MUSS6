# Audit mọi frame animation — W0006

Ngày 2026-10-23. Công cụ offline `web/tools/animation_audit.py` kiểm tra schema animation BMD10/BMD12 từ `native/client/assets.cpp`. Không sửa production hoặc Data. Mỗi model được xử lý riêng, không giữ toàn bộ pose của mọi model trong RAM.

## Kết quả thực tế

- Quét đúng 5.034 đường dẫn model của W0002; SHA-256 của cả 5.034 file khớp baseline W0001.
- 5.028 model qua kiểm tra số liệu mọi action/frame, gồm position, rotation và locked root-position. Tổng 10.396.461 float animation trong các model qua hoàn chỉnh.
- 6 file chưa qua: 4 model version 0 chưa hỗ trợ, 2 trường hợp float không qua kiểm tra. File lỗi dừng ở lỗi đầu tiên; không gọi phần còn lại của file đó là đã kiểm tra.
- 5.023 model cùng qua kiểm tra mesh của W0002 và số liệu animation W0006. Chưa suy ra render, skinning hay chuyển động giống bản gốc.

[Báo cáo tổng hợp](animation-audit-W0006.json) ghi từng lỗi, hash và offset. Báo cáo đầy đủ từng model nằm tại `web/.local-baselines/2026-10-23-m00-006/animations.json`, hash của báo cáo được lưu trong bản tổng hợp.

Hai phát hiện mới so với W0002:

1. `Item/dark_05.bmd`: NaN tại position của bone 3, action 0, payload offset 8653.
2. `Object79/Object05.bmd`: float khoảng 7,04e22 tại locked-position action 2, payload offset 113549; vượt ngưỡng native hiện tại 1e7.

Bốn file version 0: Object3/Object25, Object27, Object33, Object90. Chưa có decoder hoặc bằng chứng layout của version này. Không sửa version byte để ép đọc.

Các lỗi mesh W0002 ở Object35/Object36, Object52/Object90, Object57/Object36, Player/HelmMale20, Skill/CW_Bow_Skill **vẫn còn mở**, dù phần animation qua kiểm tra số liệu. Công cụ mới skip mesh sau khi kiểm tra count/bounds để đi tới action; không thay thế kiểm tra index/float vertex/normal của W0002.

## Phạm vi kiểm tra

Kiểm tra version/encrypted length, bounds từng block, action/key count, mọi float animation hữu hạn và nằm trong giới hạn số native, parent index và chu trình skeleton, byte thừa cuối payload. Locked root-position cũng được đọc thay vì bỏ qua. Không giữ clip để render hoặc thay đổi timing.

Chưa xác minh ý nghĩa action, tốc độ, interpolation, animation khi cầm trang bị, tính đúng của bind pose, hiệu ứng hoặc parity Windows. Có đủ dữ liệu số không đồng nghĩa nhân vật chạy mượt.

## Tái lập và test

```sh
python3 web/tools/animation_audit.py --data /Users/angelale/Downloads/MuClientEX603/Data --output /tmp/animations-new.json
python3 -m unittest discover -s web/tests -v
```

Output phải chưa tồn tại. **18/18 test qua, không skip** trên máy hiện tại sau khi các CLI W0002/W0003 đã build. Test mới kiểm tra BMD12 mã hóa, action không được giữ trong audit cũ có NaN, locked-position NaN, ngưỡng float, cắt payload, byte thừa và skeleton có chu trình kể cả khi không có mesh.

## Trạng thái

Phần kiểm tra số liệu mọi frame đã có coverage cho 5.028 model, 6 ngoại lệ được nêu rõ; chưa đóng toàn bộ M00.3b. Các mục terrain, mapping, table/font và baseline visual/browser còn thiếu vẫn giữ trong STATUS/KNOWN-GAPS. Server/DB không bật; chưa có thay đổi gameplay hoặc renderer.
