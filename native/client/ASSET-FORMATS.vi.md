# Bộ đọc asset EX603 và renderer 0.8

Code được viết cho native client trong assets.cpp và world_view.mm, đọc bộ Data hiện có mà không sửa hoặc chuyển đổi các file gốc.

| Asset | Phần đã kiểm chứng |
|---|---|
| World1/EncTerrain1.map | Giải mã rolling XOR; header version 0/world 1; 3 mảng 65.536 byte: base tile, overlay tile, alpha. |
| World1/EncTerrain1.att | Rolling XOR rồi mask 3 byte; header và lưới 256×256. Attributes được đọc, chưa dùng cho movement. |
| World1/TerrainHeight.OZB | Wrapper 4 byte + BMP 8-bit uncompressed 256×256; offset pixel lấy từ BMP header; chiều cao scale 1.5 unit/byte, quy đổi 100 world unit/tile. |
| World1/TerrainLight.OZJ, Tile*.OZJ | Bỏ 24 byte wrapper, ImageIO decode JPEG; kiểm tra dimensions/bounds. |
| *.OZT | Wrapper 4 byte + TGA truecolor uncompressed 24/32-bit; xử lý origin flags và BGRA/RGBA. Không hỗ trợ RLE ở phiên bản này. |
| Player/Player.bmd | BMD version 12, giải mã payload; skeleton và các clip được chọn (0: set, 1: idle nam, 15: walk nam); giới hạn 256 frame/clip và 262144 bone-frame lưu. |
| Player/{Helm,Armor,Pant,Glove,Boot}Class01.bmd | BMD version 10; vertex bone-local, UV, triangle indices, texture references; 640 triangles tổng cộng. Kiểm tra binding với các bone thực sự được mesh dùng. |

Đối chiếu format (không thay bộ source EX603 của người dùng bằng repo khác):

- [FileCryptor và rolling XOR](https://github.com/bernatvadell/muonline/blob/main/Client.Data/FileCryptor.cs).
- [Terrain mapping fields](https://github.com/bernatvadell/muonline/blob/main/Client.Data/MAP/TerrainMapping.cs), [ATT reader](https://github.com/bernatvadell/muonline/blob/main/Client.Data/ATT/ATTReader.cs).
- [BMD reader](https://github.com/bernatvadell/muonline/blob/main/Client.Data/BMD/BMDReader.cs) và các record vertex/normal/triangle trong cùng directory.
- [Terrain texture table](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzOpenData.cpp), [height scale](https://github.com/oXaa/muonline-client-sources/blob/master/ZzzLodTerrain.cpp).
- [SceneKit geometry API](https://developer.apple.com/documentation/scenekit/scngeometrysource).

Kiểm thử thực tế đọc 256×256 terrain, minimap 1024×1024, OZJ/OZT và BMD gốc; từ chối texture cắt ngắn, skeleton có vòng và binding không khớp. Renderer contract kiểm tra tọa độ sau chuyển từ CGFloat của macOS sang geometry source. Scene screenshot đã được kiểm tra bằng mắt; tests asset không thay thế visual QA.

Renderer hiện có terrain và nhân vật DW mặc định với sáu keyframe idle và bảy keyframe walk xương gốc được nội suy quaternion và GPU skinning. Vertex giữ tọa độ bone-local, trọng số 1, inverse bind identity; chuyển hệ trục và scale 0,01 ở rig root. Terrain, texture và pose được cache dùng chung trong tiến trình; movement chỉ cập nhật actor/camera sau xác nhận của server. Chưa đọc EncTerrain1.obj để dựng nhà/cây; các vùng tối trên terrain có thể là lightmap/shadow của các object chưa được dựng. Alpha blending atlas, skeleton pose, camera và shading chưa được đối chiếu frame-by-frame với executable Windows.

Bảng action tham khảo: [_enum.h](https://github.com/oXaa/muonline-client-sources/blob/master/_enum.h), PLAYER_SET=0, PLAYER_STOP_MALE=1, PLAYER_WALK_MALE=15. Source này dùng để đối chiếu tên action; chưa chứng minh parity với EX603 executable còn thiếu. Root X/Y của clip bị khóa tại idle để tránh cộng travel của animation vào tọa độ server. Geometry và hai bộ quaternion keyframe được tạo một lần cho mỗi viewer; mỗi frame slerp/normalize khớp, tính FK cho boot contact rồi GPU skinning. Chuyển idle/walk blend 0,12 giây, actor/camera đi cùng 0,4 giây mỗi ô. Không dùng CAAnimation blend quaternion.

Shadow01.bmd trong Data/Player được đọc bằng BMD reader hiện có, kiểm tra binding từng bone được mesh dùng với Player.bmd. Shadow pose lấy từ FK của cùng keyframe nội suy; vertex được chiếu theo hướng cố định lên terrain (nội suy đúng hai tam giác mỗi ô). Geometry source dùng ba Metal buffer để cập nhật vertex mà không tạo mesh mới mỗi frame. Đây là bóng chiếu model, không phải blob/circle. Chưa xác nhận visual parity với hình Windows; GUI cần được kiểm tra sau build.

## Sửa tương thích 0.14

TerrainHeight.OZB của bộ Data này dùng BMP pixel offset 1.078 (+4 wrapper). Engine tham chiếu OpenTerrainHeight bỏ 4 wrapper rồi đọc cố định header 1.080, tạo lệch +2 pixel bytes so với BMP chuẩn. Native dùng offset 1.084 để khớp cao độ và object placement; xác định giá trị đệm EOF thay vì đọc vượt file.

Terrain vẽ bằng texture OZJ gốc: UV = tọa độ tile ×64 / chiều texture, GL_REPEAT tương đương SCNWrapModeRepeat. Alpha/light theo đỉnh và đường chéo tam giác source. Không còn atlas giảm texture về 16 pixel/ô.

Mesh.textureIndex và action.LockPositions được giữ. Object1 áp dụng alpha/additive/hidden marker theo source tham chiếu; các hiệu ứng nước/lửa/cây động và type168 còn chưa hoàn thiện.
