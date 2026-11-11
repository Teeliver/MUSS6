# W0086–W0087 — Move và animation cánh

2026-10-28. Người dùng xác nhận “ok hết rồi nhé” sau bản W0087.

## Nguyên nhân và sửa Move

Đã bắt lỗi thật trên browser: `Movement ACK timeout after 0 map transition`. Bản cũ đóng socket khi chờ ACK bước đi quá4giây. Bỏ đóng socket vì timeout đơn lẻ; dừng route ở tọa độ server đã xác nhận, chấp nhận ACK trễ cho đúng endpoint; hủy request/timer di chuyển cũ khi nhận teleport.

Sau đó người dùng xác nhận hết disconnect nhưng không đi được. Tìm đúng bước thiếu trong source `WSclient.cpp:ReceiveTeleport` → `wsclientinline.h:SendRequestFinishLoading`: C1,4,F3,12. `Protocol.cpp:CGCharacterMoveViewportEnableRecv` chuyển RegenOk1→2; ObjectSetStateCreate/Proc chuyển2→3→0. Nếu thiếu packet, CGMoveRecv return khi RegenOk>0.

Web hiện đợi view.world nạp cảnh rồi gửi FinishLoading cho teleport gate khác0. Có mapEpoch để không gửi ACK nạp cảnh cũ/khác revision; chỉ gửi một lần; chặn gửi bước đi khi scene chưa finish. Không thay đổi kiểm tra server, không restart GameServer.

## Cánh

Bản trước attachment_geometry dùng initial_geometry tĩnh cho cánh. Đã lấy pose clip0 theo sample_pose thay vì rest pose; sampling trong BodyAnimation mỗi frame và gắn bone47/offset15. Tốc độ0.25*25keys/s khi thường,1*25keys/s khi đang bay theo nhánh `ZzzCharacter.cpp:RenderLinkObject`/PART_t.PlaySpeed. Thêm Player action11(STOP_FLY),34(FLY) làm world mode8/9 cho nhân vật có cánh ngoài safe zone. Khác biệt bow/crossbow/robe, vật lý cloak, sprite hiệu ứng cánh custom chưa được chứng minh parity đầy đủ; không đánh dấu100% toàn bộ cánh.

## Kiểm chứng

236 test Node qua, WASM bridge/contracts rebuild emcc6.0.13. Thêm contract cánh có pose thay đổi theo thời gian, tốc độ bay; test movement timeout giữ socket/late ACK, teleport hủy route, FinishLoading exact bytes/stale epoch/once-only. Debug116/release32 file deploy.

User test phiên thật báo đã hết lỗi. Sau đó browser quan sát MuOptions Lv300, ACK move2.1ms, FPS60/p9517.4ms, cảnh Lost Tower với cánh; ảnh `/tmp/mu-w87-move-wing.png`. Console còn lưu warning từ bản cũ timestamp trước refresh, không tính là lỗi mới. Không tự nhận đã chạy hết mọi map: user ngay sau đó báo còn delay/thiếu resource và effect, chuyển sang audit map toàn bộ W0088.
