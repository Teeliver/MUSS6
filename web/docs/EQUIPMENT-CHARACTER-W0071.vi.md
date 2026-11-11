# W0071 — Small Shield, nút nhấn, thông số và tháo/mặc trang bị

## Source đối chiếu

- Main5.2/source/NewUIButton.cpp: ChangeButtonImgState overflg=false dùng UP frame0 và DOWN frame1. NewUIMyInventory.cpp và NewUICharacterInfoWindow.cpp dùng chế độ hai frame cho exit/stat. Data exit 36×58 (hai frame 36×29), stat 16×30 (hai frame 16×15).
- Small Shield: Item 6:0, Item/Shield01.bmd, texture du_shield1.jpg. Plane mặt nằm trên YZ; bounds thô X21.65 / Y53.98 / Z53.98. Góc cũ nhìn nghiêng; inventory dùng 270/-90/0 để đưa pháp tuyến X về camera Z, giữ mặt khiên. Đây là fallback tương thích Data khi chưa có ItemTRSData, không giả là đã đọc được bảng TRS.
- native/client/item_placement.cpp và GameServer/ItemManager.cpp: CheckItemMoveToInventory, MoveItemToInventoryFromInventory, CGItemMoveRecv 0x24. Ô trang bị 0..11 và túi chính12..75 cùng container0; server kiểm class, stage, yêu cầu chỉ số, hai tay và item bị chặn theo map.
- Main5.2/source/NewUICharacterInfoWindow.cpp và PMSG_NEW_CHARACTER_CALC_SEND/CHARACTER_INFO/LEVEL_UP trong server: thêm chỉ số đang có packet; không tự tạo account expiration/VIP/quest/master stats chưa nhận.

## Đã làm

- Icon shield quay về mặt trước. Không đổi góc khiên đang cầm ngoài map.
- Footer exit và nút tăng điểm dùng đúng hai frame resource, có pointerdown/up/leave/cancel và keyboard; HUD/header button có phản hồi nhấn. Không gửi cộng điểm trong kiểm tra.
- Click nhấc trang bị rồi đặt vào ô túi trống để tháo. Nhấc item trong túi và chọn ô trang bị trống để mặc. Kiểm footprint, slot/class/stage/chỉ số và xung đột hai tay ở UI; packet vẫn do server quyết định. Ô có đồ không tự swap, yêu cầu tháo trước. Túi mở rộng vẫn chỉ xem.
- Dùng cùng packet C3:24, giữ wire12byte gốc; không đổi inventory trước ACK. ACK thất bại giữ món ở chỗ cũ; timeout giữ khóa chờ phản hồi hoặc vào lại, không retry mù. Không bán/drop/sửa item thật.
- Snapshot inventory sau ACK thay roster CharSet cũ cho armor/weapon. Dựng lại body/attachment theo state mới; boot level dùng số thật. Wing slot7 được đưa qua attachment core đã có; helper/mount/pet world riêng còn chưa đầy đủ.
- Bảng C bổ sung reset/fruit, attack rate, SD/AG/casting speed. Trang Chi tiết đọc đủ các trường CALC đang có: PvP attack/defense rate, magic rate, curse cho Summoner, Dark Spirit cho DL, RF multipliers, base/bonus từng chỉ số. Không bỏ các trường32bit về16bit; không hiển thị dữ liệu bịa khi packet chưa tới.

## Kiểm tra

166/166 client test qua; thêm test tháo/mặc cả hai hướng chỉ commit sau ACK, từ chối giữ đồ, luật class/stage/twoHand, state reset/fruit/full CALC, roster cũ không ghi đè trang bị đã xác nhận. Asset thật Staff06/Bone Helm/Shield01 được kiểm tra. Debug82/release31 file build + stage179asset thành công; backup W0071-20261026-220237. Test browser/online đang thực hiện, người dùng tự kiểm tra tháo/mặc trên nhân vật.

## Giới hạn

Chưa tích hợp toàn bộ ItemTRSData/custom tooltip; panel chi tiết là bản UI để xem các trường server có cung cấp, chưa port mọi tab VIP/account/quest/master của source bổ sung. Rebuild cảnh khi đổi equip có thể khựng trong thời gian tải/dựng; chưa chuyển sang GPU attachment hot-swap. Không xác nhận wing/pet/mount mọi loại đã đầy đủ chỉ vì server chấp nhận trang bị.

Kiểm tra mặt trước Shield01: nhóm normal +X có 12 tam giác, tổng diện tích UV ~0.7606 (mặt gỗ/kim loại), normal −X có 6 tam giác UV ~0.0154 (mặt sau màu nâu). Dùng −90 ở Y để +X đi về +Z gần camera; +90 sẽ nhìn mặt sau. Texture gốc du_shield1.OZJ 32×32 có tâm kim loại.
