# W0095 — Kiểm tra cánh cấp 1–3 theo đúng class

Đã bỏ fixture mặc một cánh cho mọi class. Danh sách lấy class/bậc và level từ Local/Eng/Item_eng.bmd; mỗi lần chỉ hiển thị một cặp hợp lệ. Không thay dữ liệu nhân vật online.

18 loại cánh/áo choàng,20 cặp hợp lệ,960 mẫu pose trong WASM;7class đều có cánh riêng. Model/texture đọc được, không lỗi tọa độ. ModelAnimation độc lập có chuyển động ở14 model cánh;4model áo choàng không có chuyển động mesh và cần physics cloth theo nguồn.

| Item | Class/bậc tối thiểu | Level | Kết quả |
|---|---|---|---|
| Wings of Elf | Fairy Elf bậc1 | 180 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Heaven | Dark Wizard bậc1, Magic Gladiator bậc1 | 180 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Satan | Dark Knight bậc1, Magic Gladiator bậc1 | 180 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wing of Curse | Summoner bậc1 | 180 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Spirits | Fairy Elf bậc2 | 215 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Soul | Dark Wizard bậc2 | 215 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Dragon | Dark Knight bậc2 | 215 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wings of Darkness | Magic Gladiator bậc1 | 215 | Cần làm tiếp: Missing native thunder/spirit joints and blue flare sprites between wing bones; chrome base pass added but full silhouette/effect parity remains incomplete. |
| Wind of Despair | Summoner bậc2 | 215 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Cape of Fighter | Rage Fighter bậc1 | 180 | Cần làm tiếp: Cape cloth simulation missing; native RenderLinkObject skips this item and uses physics cloth. |
| Wing of Storm | Dark Knight bậc3 | 400 | Cần làm tiếp: Native mesh order and additive light passes added; stepped lightning atlas and bone-attached particles remain incomplete. |
| Wing of Eternal | Dark Wizard bậc3 | 400 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wing of Illusion | Fairy Elf bậc3 | 400 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Wing of Ruin | Magic Gladiator bậc3 | 400 | Cần làm tiếp: Native additive mesh pass added; pulsing luminosity and BITMAP_3RDWING_LAYER overlay remain incomplete. |
| Cape of Emperor | Dark Lord bậc3 | 400 | Cần làm tiếp: Cape cloth simulation missing; static model does not reproduce source cape. |
| Wing of Dimension | Summoner bậc3 | 400 | Không thấy lỗi hình học rõ trong pose đã chụp; chưa chứng nhận toàn bộ FX |
| Cape of Overrule | Rage Fighter bậc3 | 400 | Cần làm tiếp: Cape cloth simulation and side strips missing. |
| Cape of Lord | Dark Lord bậc1 | 180 | Cần làm tiếp: Cape cloth simulation missing; native RenderLinkObject skips this item and uses physics cloth. |

## Sửa trong lượt kiểm tra

- Elf cấp1/cấp2: BlendMesh0 theo ZzzObject.cpp:6610–6630; loại nền đen bằng pass cộng sáng.
- MG cấp2: thêm chrome .5 ×(.8,.6,1) theo ZzzObject.cpp:8730; joint thunder/spirit vẫn thiếu.
- DK cấp3: RenderMesh2 thường trước0/1 cộng sáng theo ZzzObject.cpp:8740; atlas và particle chưa đủ.
- MG cấp3: mesh0 cộng sáng; chưa có pulse và BITMAP_3RDWING_LAYER đầy đủ.
- Depth LEQUAL cho lớp cộng sáng để không tự che lớp đồng phẳng.

## Giới hạn còn lại

Bốn áo choàng DL/RF chưa đúng: cần port CreateParts/RenderParts/CPhysicsCloth, bone19, collision spheres và texture vải. Không coi nạp model thành công là đã đạt. Cánh có sparkle/joint/flare riêng vẫn phải đối chiếu tiếp; báo cáo không khẳng định100%.

Bằng chứng từng cặp trong /tmp/mu-w95-{item}-{class}.png; dữ liệu máy đọc trong WINGS-CLASS-AUDIT-W0095.json. Cảnh xem: ?scene=terrain-check&map=0&x=140&y=125&walk=1&wingtest=6145; menu chỉ chứa20cặp đúng điều kiện source.

Kiểm tra code cuối:16 test item visuals và3 test attachment đã qua; build debug/release đã phát hành. Ảnh tổng thể sau sửa nền đen: /tmp/mu-w95-elf-fixed-full.png.
