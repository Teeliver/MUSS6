# Resource map — 2026-10-16

Map dùng đường dẫn gốc, không copy hoặc đổi tên Data. Audit lúc nạp được ghi
vào `/tmp/mu-client-world-resource-audit.log`, không ghi log này mỗi frame.

| Map server | Data | Trạng thái hiện tại |
|---|---|---|
| 0 Lorencia | World1 / Object1 | Giữ terrain UV, height legacy và batching đã kiểm thử |
| 3 Noria | World4 / Object4 | 9345 object được vẽ; thiếu Object44.bmd và hai texture terrain |
| 51 Elbeland | World52 / Object52 | 4643 object tĩnh; Class06, idle 3, walk 16, run 25 |

Elbeland đối chiếu `ZzzOpenData.cpp`, `GMNewTown.cpp`, `ZzzCharacter.cpp`:
- Texture số 2 dùng AlphaTileGround01.OZT; tổng 14 texture nền.
- Các type 0,54,58,59,60,61,62 là marker/HiddenMesh=-2, không vẽ mesh tĩnh.
  Particle, ánh sáng động, chim và âm thanh tương ứng chưa port đầy đủ.
- Object90.bmd có chỉ số xương -8888 trên đỉnh được tham chiếu. Không nới
  validation để vẽ dữ liệu sai; tạm bỏ model và ghi audit.
- Object166.bmd chưa có trong bộ Data.
- Một số mesh thiếu texture: bottle.tga, shield02.jpg, shield03.jpg,
  sword08.jpg, sword02.jpg, medicine1.jpg, medicine2.jpg. Chỉ bỏ các mesh
  thiếu đó; giữ phần còn lại của model. Chưa tự thay bằng texture khác.

Noria còn thiếu TileGrass02.OZJ, TileGround02.OZJ: dùng World1 tạm thời,
được ghi rõ trong audit; không coi là tài nguyên Noria đã hoàn chỉnh.

Giữ mục tiêu render 60 FPS, cache model/texture và gộp mesh theo texture,
chunk 16x16. Chưa xác nhận FPS thực tế Elbeland trên phiên GUI online.

## Noria — 0.26.3 / 0.26.4 (2026-10-17)

Đối chiếu engine tham khảo `ZzzObject.cpp` (CreateObject, MoveObject),
`ZzzCharacter.cpp` (CreateMonster) và `ZzzOpenglUtil.cpp` (EnableAlphaBlend).
Đây là engine tham khảo; chưa khẳng định toàn bộ hiệu ứng trùng EX603 100%.

- Giữ các texture slot additive của type 1,9,17,18,19,37,39,41.
  Dùng phép cộng màu và tắt depth write giống GL_ONE/GL_ONE; không đổi Data.
- Type38 có HiddenMesh=-2: không vẽ mặt phẳng marker. Hiệu ứng operate/
  sprite của marker này chưa port, nên chưa coi như đã hoàn tất hiệu ứng.
- NPC238 và NPC242 dùng BlendMesh=1, khôi phục nền trong của cánh tiên.
- Máy Chaos là object type39, file Object4/Object40.bmd; giữ action0, xương,
  vị trí/góc/scale gốc. CreateObject đặt Velocity=.16, MoveObject gọi
  PlayAnimation theo tick25Hz: 4 key/giây. Nội suy vị trí/quaternion mỗi frame,
  giữ mesh trên GPU, chỉ cập nhật máy khi chunk hiển thị. Không xoay cả model
  bằng animation tự chế. Cây/địa hình vẫn trong batch tĩnh.
- Kiểm thử Noria đếm đủ đỉnh additive so với mesh gốc, kiểm tra vật liệu NPC,
  xương máy thay đổi theo thời gian và trở về đầu chu kỳ.

Còn thiếu: Object44.bmd và hai texture terrain đã nêu ở trên; hiệu ứng sprite,
UV scrolling type18/41/42/43 và animation các cảnh vật khác chưa port đầy đủ.
Lần kiểm tra GUI 0.26.3 ở Noria hiển thị60FPS; frame callback trung vị khoảng
16.7ms. Đây là số đo phiên thử, không phải cam kết mọi map luôn60FPS.

## Dây Noria tại X181 Y96 — kiểm tra 2026-10-17

Ảnh người dùng cho thấy chân nhân vật che dây. Đối chiếu đúng ô:
- EncTerrain4.att: attribute=1 (safe zone); không có TW_NOMOVE=4 hoặc
  TW_NOGROUND=8. Giữ nguyên quyền đi lại, không thêm collision từ mesh trang trí.
- Mesh cắt tia camera quanh chân thuộc type19: Object4/Object20.bmd,
  texture wd.jpg, slot0. CreateObject đặt BlendMesh=0 cho type19.
- BMD::RenderMesh chọn EnableAlphaBlend cho slot này; EnableAlphaBlend dùng
  GL_ONE/GL_ONE và DisableDepthMask. ZzzScene render objects trước
  RenderCharactersClient; Noria/type19 không được chuyển sang lượt sau nhân vật.
- Bản macOS đã dùng additive, depth write=false và lượt trước player.
  Vì vậy chồng hình trong ảnh phù hợp với engine tham khảo, không phải bằng
  chứng nhân vật đứng trên mesh dây. Tâm ô chân không giao mặt dây theo XY.

Không đổi resource, độ cao, vị trí dây hoặc ATT để làm mất hiện tượng này.
Engine đầy đủ đang đối chiếu là source tham khảo trong /tmp; bộ Main_EX603
của người dùng là DLL mở rộng, nên chưa có đối chiếu trực tiếp executable
Windows của đúng bộ này. Có thể mở lại kết luận nếu có bản Windows để so sánh.

Kiểm tra ảnh offline cùng đúng tọa độ:
`MU_NORIA_FOCUS=181,96 MU_NORIA_SNAPSHOT=/tmp/noria-rope.png`
khi chạy mu_client_render_contract CLIENT_ROOT --elf-only.
