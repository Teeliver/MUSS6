# W0067 — bóng đổ trên terrain

2026-10-26. Bật bóng cho các mesh vật thể, nhân vật/trang bị và NPC/quái **đang render được** trong world. Không tạo hình hoặc bóng cho những resource/binding còn thiếu.

## Đối chiếu source

- Source bổ sung `file thieu/ZzzBMD.cpp:1956` CalcShadowPosition: trừ BodyOrigin; `x += z*(x+sx)/(z-sy)`; cộng origin; đặt Z bằng RequestTerrainHeight+5. RenderBodyShadow2087 dùng sx2000,sy4000; BattleCastle sx2500. AddMeshShadowTriangles2043 bỏ hidden mesh và BlendMesh.
- Web chuyển đơn vị100 thành tile: sx20 (map30:25),sy40,bias0.05; giữ Y; height field nhận đúng cao độ terrain. Có chặn mẫu số gần0 để tránh hình học vô hạn.
- `ZzzObject.cpp:11789+` có gate EnableShadow/m_bRenderShadow, map/Hellas/cloaking và lựa chọn mesh. Một số nhánh dùng đen opaque, map7 dùng alpha0.2. Web hiện dùng bóng alpha0.30 + stencil chống chồng đen; **không tuyên bố giống100% mọi gate/material/source branch**. Chưa đủ authoritative cloaking/visibility policy cho toàn bộ quái/map.
- Native viewer có Shadow01 riêng cho player; web dùng mesh thật đang pose (cả trang bị), không thêm blob tròn cố định. Chưa port riêng Shadow01 và cloth simulation.

## Cách tích hợp

- `world/shadow-field.js` tạo **một** texture R32F256×256/map (256KiB), dữ liệu từ terrain.height. Texture dùng chung cho world và mọi NPC renderer, dispose sau khi các renderer đã đóng; không tạo texture mỗi frame/NPC.
- Vertex shader dùng chính position buffer của object hoặc animation hiện tại; lấy terrain height theo cùng tam giác với terrainPatch bằng texelFetch. Không tạo bản sao toàn bộ shadow mesh trên CPU mỗi frame.
- Batch môi trường lưu origin mỗi vertex để nhiều object cùng texture/chunk vẫn chiếu theo origin riêng. Actor/NPC dùng origin cập nhật theo vị trí hình đang nội suy. Bóng actor theo walk/run/attack pose và vũ khí đã render.
- Render terrain trước, mọi bóng world/actor/NPC sau, rồi object/actor/NPC bình thường, mái và marker. Bóng chỉ nhận trên terrain; chưa shadow-map vật thể lên tường, tầng nhà hoặc self-shadow.
- WebGL context yêu cầu stencil. Stencil chỉ clear cùng đầu frame; toàn bộ shadow pass dùng chung coverage, mỗi pixel blend một lần, không bị đen dần vì nhiều triangle trùng. Không ghi depth trong shadow pass, giữ depth của terrain.
- Không cast terrain, mesh additive/multiply/marker/effect. Cutout texture dùng alpha discard để lá cây không thành tấm chữ nhật. Roof alpha giảm tới0 thì bóng mái cũng tắt. Mesh còn hidden/opacity0 không vẽ bóng.
- Culling tiếp tục theo bounds/chunk hiện tại; các map ngoài Lorencia còn nạp theo vùng cũ. Shadows không biến phần resource/gameplay thiếu thành hoàn chỉnh.

## Kiểm chứng

- 146/146 client tests pass, không skip, log `/tmp/mu-w67-tests.log`. Test mới kiểm projection theo công thức source, cao độ nhận bóng/bias, texture dùng chung/dispose đúng một lần, receiver/additive exclusion, depth/stencil và roof fade. Các test movement/combat cũ vẫn qua.
- Debug72file/Release31file; stage179preview+profile và swap output hoàn chỉnh, không xóa dist lúc đang phục vụ. Backup `web/.build/deploy-backups/W0067-20261026-153650`.
- Browser OFFLINE Lorencia: 2.787object đã mapping, đi quanh140,125→140,133; bóng dáng nhân vật thấy rõ trên nền trống, công trình/cây có bóng, khoảng60FPS. Ảnh `/tmp/mu-w67-player-shadows.png`. Đây là visual test renderer, không phải kiểm combat/network mới.
- Không thay Data gốc/server/DB; phiên đang mở cần refresh để WebGL context mới có stencil và nạp module mới.
- Browser NPC OFFLINE tại146,110:3NPCfixture hiển thị,0pending,~60FPS; không có lỗi shadow shader/height upload. Những NPC249/257 chưa có binding vẫn báo đúng cảnh báo cũ, không tạo bóng giả. Ảnh `/tmp/mu-w67-npc-shadows.png`.
