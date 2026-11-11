# W0106 — Set Item Option, luồng thả đồ và rà icon

Ngày: 2026-10-31.

## Các phần đã sửa
- `NewUIMyInventory.cpp:1014` RenderSetOption: nhãn GlobalText[989] tại y25 dưới Inventory, xám khi chưa có option kích hoạt, vàng khi có; pointer hover mở danh sách.
- `CSItemOption.cpp` CheckItemSetOptions/calcSetOptionList/getExplainText: đọc ItemSetType và ItemSetOption_eng từ Data. Type 8192×4 byte, Option bộ Data này 64×110 byte (không phải 256 hàng của client mới). Checksum E5F1/A2F1, XOR từng record FC CF AB. SHA256 lưu trong item-set-data.js. 168 liên kết và 49 set hợp lệ.
- Đếm đồ đang mặc, bỏ wing/helper, đồ hỏng, thiếu class/cấp/chỉ số. Hai vũ khí/nhẫn trùng type và ancient variant chỉ tính một. 2 món mới có standard bonus; full bonus theo count-1 >= byOptionCount-2. Dùng GlobalText cho từng option. Không tự cộng damage/stat, GameServer vẫn là nguồn chỉ số.
- Luồng thả trong `NewUIMyInventory.cpp:765` không có confirm dialog. Đồ quý bị chặn và thông báo GlobalText[269]; đồ thường gửi yêu cầu trực tiếp. Bỏ confirm tự dựng của web. Sửa highValueItem: không coi tất cả group12 là đồ quý; Orb of Summoning thường không thuộc danh sách cấm, Chaos/wing/bundle vẫn thuộc danh sách.

## Xác minh
- 97 test mục tiêu qua trước chỉnh bổ sung signature cập nhật theo attributes/classTier.
- UI offline fixture setreview=1 dựng Warrior 7 món từ bảng Data; trình duyệt hiện nhãn vàng và 9 dòng option thật, gồm Strength+10, skill+10, maxAG+20, AG recovery+5, defense+20, agility+10 và 3 full bonus (critical5%, excellent5%, strength25).
- Không thả/bán/chuyển item của nhân vật thật khi test. Fixture không kết nối GameServer.

## Hai icon chưa chốt
- Wings of Heaven đúng `Item/Wing02.bmd`, texture `angel_wing.tga`, action0 frame0, một mesh. W0105 mới đổi góc X riêng; người dùng đã báo hình vẫn chưa đúng. Lượt này chưa sửa thêm góc/tỷ lệ tùy ý. Cần bảng TRS riêng của đúng client hoặc đối chiếu icon native để xác định chính xác phần pose/projection còn sai.
- Violent Wind Helm 7:39 đúng `Player/HelmMale40.bmd`, native HideSkin chọn mesh2 `SkinClass106_headhelmet.jpg`, loại mesh đầu/tóc 0/1. Bounds sau pose Player0 và góc hiện tại xấp xỉ 12.812×7.360×9.082 model units. Renderer hiện auto-fit nên chưa có căn cứ kết luận tỷ lệ gốc; fallback .0039 cho footprint chỉ khoảng 6.87×3.95px tại UI480, không được tự thay bằng fallback rồi tuyên bố chuẩn. ItemTRSData.bmd chưa có trong bộ đã cung cấp.

## Giới hạn
- Các luật custom ItemAction/period/harmony và socket của client mở rộng chưa được port trọn vào drop policy; GameServer vẫn có quyền từ chối. Không khẳng định toàn bộ drop policy hay cổ vật có mọi option riêng đã đạt parity tuyệt đối.
- Extended set slots bất thường trên 7 món chưa được thực thi; native source có nhánh extended riêng cần bộ config có case thực tế. Data hiện dùng standard/full cho các set được kiểm tra.
