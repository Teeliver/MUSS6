# W0093 — Item, hướng cánh và pet/người chơi khác

2026-10-29. Đang kiểm tra hình ảnh; chưa parity100%.

## Item

Audit ITEM-RESOURCES-W0093.json đọc924 item có tên/kích thước trong Item_eng.bmd,902 item/903 model variant tìm đủ model và texture,22 chưa có mapping. Không tự chọn model tương tự cho22 item đó.

Bộ lọc icon trước chỉ nhận skin_ và các prefix cũ. Data có thêm skinClass301/302/305 (body), SkinClass*head_N (face), headhair_N, *_hair và *face. Đã ẩn các mesh này trong icon, giữ _headhelmet, _upper/_lower/_glove/_boot là texture giáp thực. Không áp bộ lọc icon vào model nhân vật.

Nhẫn/dây chuyền trước bị normalize riêng để lấp gần toàn ô khiến đồ nhỏ thành quá to. Đã chuyển nhóm trang sức sang tỷ lệ kích thước model chung, cố định theo đơn vị ô20px, giữ khác biệt kích thước giữa các model và giữa ô túi/ô trang bị. Tỷ lệ fallback suy ra từ nguồn: scale0.0039, FOV UI1độ, khoảng cách2000*0.1=200 ở canvas chuẩn480px => 240*0.0039/(200*tan(0.5độ)) pixel/đơn vị model; anchor(0.5,0.8) theo ScriptItem. Chưa có override từng item từ bảng ItemTRSData. Source ScriptItem.cpp:18 đọc Data/Local/ItemTRSData.bmd, file này vẫn chưa thấy trong Data hiện có; không tuyên bố tỷ lệ từng item trùng100%.

## Cánh

ZzzCharacter.cpp:14834–14858: cánh thường RenderLinkObject(Link=false), bone47, offset(0,0,15). Đọc sâu BMD::RotationPosition (ZzzBMD.cpp:466) cho thấy nó vẫn copy ma trận bone vào ParentMatrix. Vì vậy không được bỏ rotation bone chỉ dựa vào tên Link=false. Thử nghiệm bỏ rotation đã được gỡ. Wing40/50 là Link=true bone19; Wing50 dùng góc(0,90,0), vị trí(10,-15,0). Cập nhật W0094 ngày30/10: gỡ phép lật180° local của cánh thường vì không có căn cứ trong source; giữ bone47 nhân pose clip gốc. Sửa nhịp khi đứng bay(mode8) và di chuyển bay(mode9) đều25keys/s; Wing36 bay12.5keys/s; Wing39 luôn3.75keys/s. Lúc đứng đất6.25keys/s. Satan vẫn gắn bone34, không áp dụng phép lật của pet độc lập.


## Người chơi khác

Giải mã companion bits từ CharSet nguồn đã có nhưng remoteEquipment chỉ dùng7 phần thân/vũ khí. Đã bổ sung slot7(cánh),slot8(pet) từ companionsResult; override F3:13/C1:25/tháo đồ vẫn ưu tiên. Giữ model cũ đến khi thay mới sẵn sàng; nâng giới hạn viewport renderer từ32 lên400. Nhân vật có cánh ngoài safe zone chọn fly idle/move thay vì đứng/chạy bộ.

## Pet

Thêm pet-layer, hiện model/animation khi trang bị slot8, cập nhật theo owner và gỡ khi tháo/hết durability. Pet-layer áp dụng cho helper0/64/65/67/80/106/123; texture lấy itemTextureReference đúng thư mục model. Nối cho cả nhân vật chính và remote. Satan(helper1) được gỡ khỏi pet-layer và gắn trực tiếp body bone34, offset(20,0,0), animation12.5keys/s theo ZzzCharacter.cpp:14911–14931; attachment limit tăng4 để chứa2vũkhí+cánh+Satan. Ngoại lệ offset của Rage Fighter với armor59/60/61/73/83 chưa port. Điều khiển follow hiện là controller web giới hạn phạm vi, dùng model/clip gốc, chưa port toàn bộ GOBoid randomness, particle và hành vi custompet. Mount2/3/4/37 cần controller cưỡi/stance riêng và chưa render trong pet-layer này; không dùng pet đi theo thay thú cưỡi.

## Kiểm tra

256 test client qua, WASM rebuilt. Có test hướng cánh theo rotation bone, Wing50 source exception, remote companion/removal, pet policy và face/hair filtering. Fixture offline pettest/players/itemtest để xác minh resource thật; fixture không mở socket, không thay đồ tài khoản thật. Chưa xác nhận đầy đủ bằng hai phiên tài khoản online sau bản cuối.

Full suite W0093:256/256 qua. Sau đó sửa validator JS để chấp nhận attachment slot8, thêm regression gọi qua bridge cho Satan. W0094:3 test attachment qua, gồm trụcY/Z không bị lật, Wing36/39 đúng nhịp; WASM và debug/release rebuilt. Đã quan sát Satan nằm trên vai và cánh của remote bằng Data thật ở fixture; chưa xác nhận bằng hai account online sau bản cuối.

Người dùng xác nhận lỗi là lộn trên–dưới và Satan phải ở vai. Source xác nhận Satan bone34; đã chuyển đúng nhánh này và dùng itemTextureReference trong body attachments để không tìm nhầm Player/satan thay vì Item/satan. Không sửa account/DB trong đợt này.

Ảnh kiểm tra ngày30/10: `/tmp/mu-w94-wings-final.png`, `/tmp/mu-w93-items-final.png`. Hình cánh là Wings of Heaven từ Item/Wing02.bmd, không thay asset từ ảnh tham chiếu. Màu sáng/ánh sáng môi trường và mọi cánh custom chưa được chứng nhận giống100% ảnh gốc.

## W0094 — đối chiếu texture cánh ngày30/10

Source ZzzOpenData.cpp:1215–1217 nạp Wings of Heaven(6145) từ Item/Wing02.bmd. Texture trong model là angel_wing.tga, Data thực có Item/angel_wing.OZT(128x128). Player/Wing02.bmd là bản khác hash nhưng geometry/UV initial giống Item/Wing02, không tự đổi đường dẫn để sửa bằng cảm tính.

OZT decoder chuẩn hóa RGBA theo hàng trên trước; renderer materialUV lại lật V. Đã bù V cho attachment cánh dùng TGA để giữ chiều texture gốc, không sửa pixel/file Data, không đổi model. GlobalBitmap.cpp OpenTga của Main5.2 dùng stbi rồi upload trực tiếp; ZzzBMD RenderMesh dùng TexCoordV trực tiếp. Test kiểm tra V=.2 thành.8 ở output attachment, trong shader trở lại.2; vũ khí không bị thay đổi UV.

Sau bản gỡ lật local180, sửa nhịp bay và UV, người dùng xác nhận “mình thấy ok rồi”. Đây là xác nhận hình cánh đang kiểm tra, không phải chứng nhận tất cả cánh/pet/custom resource100%. Bộ256 test trước đó đã qua;3 test attachment sau sửa cũng qua.
