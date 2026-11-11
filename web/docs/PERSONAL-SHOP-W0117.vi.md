# W0117 — Cửa hàng cá nhân theo source

Ngày 2026-11-01. Phạm vi: protocol/state/UI cửa hàng cá nhân so với `Main5.2/source` và `GameServer/PersonalShop.h`.

## Trước khi sửa

Web chỉ hỗ trợ shop của chính mình:
- Gửi tạo shop `C1:3F:02`, huỷ `C1:3F:03`, đặt giá `C1:3F:01`.
- Nhận kết quả đặt giá / tạo / huỷ / bán được.
- **Thiếu** mở shop người khác, mua, rời shop, danh sách hàng, đổi tên, title trên đầu nhân vật, refresh list.

## Đã bổ sung

### Client → server
Tất cả gói gửi dùng **C3** vì source gọi `Send(TRUE)` (mã hoá), không phải C1.
- `C3:3F:05` mở shop người khác: `index[2] + name[10]`.
- `C3:3F:06` mua: `index[2] + name[10] + slot`.
- `C3:3F:07` rời shop: `index[2] + name[10]`.
- Cho phép gửi lại `C3:3F:02` khi shop đang mở để **cập nhật tên** (server `CGPShopOpenRecv` cập nhật `PShopText` rồi gửi text change).

### Server → client
- Gói đặt giá `C3:3F:01` dài **9 byte** cho profile 603 (`slot + price[4]`), không gửi JoB/JoS/JoC.
- `C2:3F:00` viewport shop titles → lưu `shop.titles[index]`.
- `C2:3F:05` / `C2:3F:13` danh sách hàng (result, index, name, title, count, từng món UPDATE603 là 20 byte: slot tại0, item12 tại1, padding3 byte, price DWORD tại16).
- `C1:3F:06` kết quả mua: result, index, item12, slot đích.
- `C1:3F:10` đổi tên shop.
- `C1:3F:12` thông báo rời/đóng shop.

### UI
- Panel cửa hàng cá nhân hiển thị shop người khác: tên người bán, title, danh sách món + giá + nút Mua, nút Đóng.
- Khi đang bán: cho sửa title và có nút **Cập nhật tên**.
- Tên nhân vật có shop mở hiển thị thêm dòng `[Store] <title>` (GlobalText[1104]).
- Phím `D` mở cửa sổ lệnh 190×429 như `NewUICommandWindow`, danh sách 11 lệnh theo `GlobalText[943/1124/944/945/946/1352/1321/1322/947/948/949]`; hiện chỉ bật **Mua hàng**.
- Chọn Mua hàng rồi click player để mở shop (đúng luồng source).
- Ctrl + click (Windows) / Control + click (Mac) vẫn là đường tắt phụ; chỉ dùng Control, không dùng Command.
- Khi giữ Control và hover player, con trỏ chuyển sang cursor xem hàng (CursorGet).
- Prefix `[Store]` lấy từ `Local/Eng/Text_eng.bmd` GlobalText[1104], không hardcode.

## Kiểm chứng

- Thêm `tests/personal-shop.test.mjs`: layout gói, viewport title, item list, mua, đổi tên, leave, frame lỗi.
- Toàn bộ **324/324 test pass**; build debug 148 file.

## Giới hạn

- Chưa hỗ trợ giá bằng Jewel (JoB/JoS/JoC) trên UI; server EX603 (<802) không yêu cầu 3 trường này.
- Chưa port search shop / search log (`C1:EC:31`, `C3:EC:33`, `C2:EC:32`).
- Chưa hiển thị title shop trên đầu nhân vật ngoài world; mới lưu `shop.titles`.
- Chưa xử lý tiền/hoa hồng khi mua; chờ packet money riêng từ server.
- Chưa port UI purchase shop native đầy đủ (grid 8x4, icon, tooltip); hiện là danh sách tối giản.

## Sửa luồng người mua — 2026-11-01

Đã xác định các lỗi khiến D → Buy → player không hiện cửa hàng:

- `personalShop` destructure tham số nhưng nhánh người mua lại đọc biến `payload` không tồn tại. Sửa nhận payload chung cho openOther/buy/leave.
- Lần đầu vào map chưa truyền callback `openShop`; trước đó chỉ có nhánh teleport. Dùng chung callback có báo lỗi, đồng thời giữ callback khi dựng lại scene.
- Nhận state shop.remote nhưng render bỏ qua panel đang hidden. Khi ACK thành công có danh sách hàng, mở panel và đóng cửa sổ D; khi remote state bị xoá thì ẩn panel.
- Chọn Buy trong menu phụ bị tự xoá trạng thái chọn. Sửa thứ tự đóng menu trước khi bật trạng thái.
- Bỏ yêu cầu cấp6 cho người xem/mua, giữ yêu cầu đó cho người đăng bán, khớp CGPShopItemListRecv/CGPShopBuyItemRecv.

Kiểm tra:29 test chọn lọc qua, gồm request/ACK của người mua cấp1, từ chối mua không mất hàng, event Buy chọn đúng player không gửi di chuyển/đánh. Thêm cảnh `?scene=ui-check&map=0&x=140&y=125&personalshop=1` để kiểm tra GUI bằng dữ liệu offline, không mua/bán đồ thật.

Đã sửa đóng shop: CGPShopLeaveRecv không gửi ACK nên gửi leave rồi đóng UI local, không giữ transaction lock. Test đóng/mở lại đã qua. Đợt này chưa chứng nhận mua/bán bằng hai account thật và chưa port grid shop native đầy đủ.

Trình duyệt đã xác nhận cảnh offline tự hiện GUI người bán (SellerTest), Kris+9, giá123,456Zen và nút Mua. Ảnh: `evidence/personal-shop/buyer-panel.png`. Build debug149file, profile/tài nguyên được giữ.

## Sửa packet thật và đối chiếu GUI/đặt giá (2026-11-01)

Phiên người mua đóng với lỗi `World packet 0x3f (75 bytes): Invalid personal shop item count`. Nguyên nhân chính xác: struct PMSG_PSHOP_ITEM_LIST của UPDATE603 có alignment DWORD, size20 (price offset16), không phải17/23byte như decoder trước. Header size55; một món tạo packet75byte. Đã bổ sung struct vào generator ABI, compile với clang++ từ header server thật, và decoder dùng artifact đó. Test dùng padding không-zero, một/hai món, refresh13 và count0; packed17byte bị từ chối.30test chọn lọc qua.

Đã xác nhận phiên thật DKAD mở shop MuOptions và hiển thị Ring of Wind giá1,111,111Zen với build35c8426f. Code mua7 nghĩa là thiếu Zen theo CGPShopBuyItemRecv, không phải lỗi packet. Tab trước đó còn giữ build8475928d nên cần refresh để nhận decoder mới. Không thực hiện mua đồ thay người dùng.

Source GUI: NewUIPurchaseShopInventory::Create là8x4ô; header GlobalText1102. Đã đổi UI người mua từ danh sách sang lưới8x4, icon thật, tooltip giá và hộp xác nhận mua GlobalText1100 trước khi gửi request.

Source đặt giá: NewUIMyShopInventory::UpdateMouseEvent gọi CPersonalShopItemValueMsgBoxLayout khi nhấc item vào shop hoặc nhấn chuột phải món có sẵn. NewUICustomMessageBox.cpp:4403 dùng hộp số,8ký tự, OK/Cancel/Enter; loại rỗng hoặc0; nếu giá thấp hơn ItemValue(item,2) có hộp cảnh báo riêng. Nếu đang bán thì đóng shop trước khi cập nhật giá/chuyển đồ. Request SendRequestSetSalePrice là C3:3F:01,9byte, DWORDlittle-endian; chờ ACK01result1.

Web đã thêm hộp nhập giá khi chuột phải món trong shop, chỉ nhận1..99,999,999Zen, giữ giá ACK làm chuẩn. Chưa port đầy đủ hộp cảnh báo giá thấp có model3D, cũng chưa gộp hộp giá vào thao tác thả item trước khi chuyển ô theo native. Không coi các phần đó là đã hoàn tất.

Trình duyệt offline bản GUI mới: lưới32ô, iconKris+9, tooltip123,456Zen; click mở xác nhận GlobalText1100, Hủy không gửi mua. Ảnh: `evidence/personal-shop/grid-confirm.png`.

## Tách shop của mình / shop đang xem — 2026-11-01

Sửa mọi nút đóng panel shop (X/footer/Esc) để gửi leave và xoá remote state, không chỉ hidden panel. Nút Cửa hàng cá nhân trong túi rời shop người khác trước rồi mở shop của mình; không lấy remote state làm nội dung own shop. Mở bảng lệnh hoặc nhân vật khi đóng shop cũng dùng cùng đường cleanup. Nếu có giao dịch đang chờ, báo chờ thay vì đổi shop giữa giao dịch. Không sửa giá/hàng của shop mình khi rời shop khác.

Đã bỏ selector Vật phẩm đặt giá, input giá và nút Đặt giá dưới lưới. Giữ hộp nhập giá khi chuột phải món; nút Bắt đầu/Ngừng bán đặt dưới lưới.

Trình duyệt fixture: SellerTest → mở túi → Cửa hàng cá nhân hiện chuẩn bị shop OFFLINE UI; không còn selector đặt giá (0element). Đóng/mở lại vẫn là own shop. Ảnh `evidence/personal-shop/own-shop-separated.png`. Đây là kiểm tra UI offline; chưa xác nhận thao tác đổi view trong hai phiên thật ở lượt sửa này.

## Chuẩn hoá hộp xác nhận mua và tự đóng shop hết hàng (2026-11-01)

Đối chiếu `Main5.2/source/NewUICommonMessageBox.cpp` (`CNewUI3DItemCommonMsgBox`) và `CPersonalShopItemBuyMsgBoxLayout`, đã thay hộp xác nhận mua dạng text bằng hộp native 230×132: khung `newui_msgbox_top/middle/bottom`, model vật phẩm 40×40 tại (30,30), dòng `GlobalText[1100]` canh giữa vùng text bắt đầu x=60,y=35, hai nút OK/Cancel 54×30 tại y=82. Nút OK gửi `C3:3F:06`, Hủy chỉ đóng hộp.

Lỗi hết hàng không đóng shop có hai nguyên nhân:
- Server gửi `GCPShopCloseSend` cho người bán rồi `MsgSendV2`, nên người mua nhận packet wide `C2:3F:03` (8 byte). Decoder cũ chỉ nhận C1 7 byte nên ném lỗi và không đóng panel. Đã đọc đúng offset theo header rộng, xoá title và đóng `shop.remote`.
- `3F:12` là `NotifyClosePersonalShop` (không phải leave ACK): khi đang xem shop người khác, client native ẩn cả hai cửa sổ và báo `GlobalText[1126]`. Web giờ đóng panel và hiện đúng câu này.

Bổ sung: list rỗng/refresh `3F:13` đóng panel thay vì hiện lưới trống; mua món cuối tự xoá `remote`; kết quả mua `6` (túi đầy) đóng panel; mã lỗi mua 6/7/8/0x23 hiện lần lượt `GlobalText[1166]/[423]/[375]` như source.

Canh lại GUI theo `MAIN_UPDATE 603`:
- Shop người bán: lưới 8×4 tại (15,110), ô tên (12,49), text tên tại x=50,y=55, nút mở/đóng (53/93,391), nút thoát (13,391), dòng `1103` khi đang bán và đủ bộ cảnh báo 370/1109/1111/1112/1113/1115/1134/1135.
- Shop người mua: lưới tại (16,90), tên shop xanh lá y=58, nút thoát (13,391), cảnh báo 370/1109/1111/1112/1114/1115/1134/1135.

Test: 332/332 pass, build debug thành công. Các test mới phủ packet close dạng C2, title change dạng C2, list rỗng, `3F:12` và kết quả mua 6.

## Lời rao UTF-8 và canh lề label shop (2026-11-01)

- Source client `MAX_SHOPTITLE=36` byte (`PShopText[36]` server). Web giờ cho phép tiếng Việt có dấu trong tên shop/lời rao, giới hạn **35 byte UTF-8** + null.
- Bỏ kiểm tra ASCII ở `world-session`; tên shop vẫn giới hạn 35 byte.
- `decodePersonalShopList` / `decodePersonalShopTitleChange` / viewport title decode title bằng UTF-8; tên nhân vật vẫn windows-1252.
- Label shop theo `PersonalShopTitleImp::CShopTitleDrawObj::Draw`:
  - dòng 1: `[Store]` + tên nhân vật, nền nâu, chữ vàng
  - dòng 2: lời rao/tên shop, chữ trắng
  - một nền nâu chung, canh trái; các dòng bắt đầu cùng mép trái.
