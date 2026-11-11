# Luồng vào game native 0.22

## Quy trình thực thi

Boot (nạp Data thật, tiến trình theo tác vụ hoàn tất) → Servers (ConnectServer discovery) → Connecting (lấy endpoint và nhận greeting GameServer) → Login → Authenticating → Characters → Loading → World.

Các trạng thái chỉ chuyển tiếp theo kết quả mạng thật. Sai mật khẩu giữ màn Login để nhập lại; lỗi kết nối trở về chọn server. Chọn nhân vật gửi F3:03, nhận world-entry rồi mới tải map. Tải dữ liệu/texture và xử lý socket trên networkQueue; UI trên main queue. Thanh tiến trình không chạy theo timer. Sau khi hoàn tất Data, giữ ở 100% cho đủ tối thiểu 1,5 giây tính từ lúc bắt đầu nạp để màn không biến mất quá nhanh; đây chỉ là thời gian trình bày, không phải công việc tải thêm.

Toàn bộ nằm trong NSWindow chính. World viewer có API nhận cửa sổ có sẵn; stop dừng renderer và cursor timer, bỏ delegate/callback trước trả về lobby. Menu MU Native Client → Select server đóng phiên rồi trở về chọn server. IP/port giữ trong client.ini, không trộn cấu hình kỹ thuật vào ô đăng nhập game.

## Nguồn đối chiếu

- Bộ người dùng: Main_EX603/Main/Main.cpp EntryProc (ServerInfo.sse, endpoint/version/serial); Emulator/ConnectServer/ConnectServerProtocol.cpp (F4:03, F4:06); Emulator/GameServer/Protocol.cpp (F1:01, F3:00, F3:03).
- Engine tham chiếu ngoài bộ: ZzzScene.cpp (Scene, StartGame, LoadingScene), WSclient.cpp (ReceiveServerConnect, ReceiveJoinServer); LoginWin.cpp (329×245, hai input 156×23, offset input và OK/Cancel); LoadingScene.cpp (800×600, bốn phần 400×512/400×88); UIMng.cpp CreateTitleSceneUI (kích thước/vị trí sáu mảnh title); ZzzOpenData.cpp (tên resource).
- Tài nguyên thực: MuClientEX603/Data/Interface/LSBg01–04.OZJ, login_back.OZT, login_me.OZT, message_ok_b_all.OZT, loding_cancel_b_all.OZT, New_lo_back_01–02.OZJ, lo_back_s5_03–04.OZJ, lo_back_s5_im01–06.OZJ, New_lo_mu_logo.OZT.
- Font dùng source_font: tên trong GlobalText[0]/Config.ini; chuỗi Account/Password/Connect/Select Server/Create/OK/Cancel dùng GlobalText ID 450/451/452/382/1412/228/229. Sprite OK/Cancel vẫn chứa chữ của bộ texture, muốn dịch hình phải có asset tương ứng ngôn ngữ.

## Phạm vi chưa hoàn tất

Đã chuyển quy trình mạng và cửa sổ. Chọn server đã dùng cha_bt/server_b2_all/server_deco_all và gauge server_b2_loding: ban đầu không chọn cụm, click mở subserver bên phải, click lại đóng; nút cụm giữ nguyên vị trí khi đóng/mở. Cụm/subserver đặt giữa theo tổng chiều rộng. Chọn nhân vật còn dùng danh sách AppKit. Đã dựng cảnh chọn server World74/Object74 bằng terrain, model và camera tour CWS thật. Camera chạy theo waypoint 25 Hz, nội suy khi render với mục tiêu 60 FPS. Chưa port đầy đủ blend đường camera, animation xương/particle, âm thanh login, intro movie, model/trang bị của các slot chọn nhân vật, UI xóa nhân vật và các hộp lỗi nguyên bản. Không coi nguồn engine tham chiếu là bằng chứng các bản EX603 có cấu hình scene giống nhau.

## Kiểm chứng

- GUI: chọn Server 0 → login tài khoản test → chọn MuOptions → loading → Lorencia X147 Y110, cùng một cửa sổ; thống kê tại lúc kiểm tra khoảng 60 FPS.
- Render contract: kiểm tra hiển thị control đúng từng stage, tỷ lệ canvas/không vượt cửa sổ tại 800×600, 1280×720, 900×900; language table và sprite login thật. Kế thừa các test protocol, asset, inventory và tooltip.
- FPS tại một thời điểm không chứng minh mọi frame đạt 16,67ms. Đã xác nhận GUI menu Select server trả về danh sách server và kết nối lại đến màn Login; phiên map cũ được đóng.

## Đối chiếu loading và giới hạn Data (sửa sau phản hồi người dùng)

Người dùng xác nhận “update Data” là nạp tài nguyên local khi mở game, không yêu cầu launcher tải patch qua mạng. Không cần URL update. Engine tham chiếu WebzenScene → OpenBasicData gọi RenderTitleSceneUI với các mốc 0..11 rồi LOG_IN_SCENE. Gauge ở (72,540), giới hạn 656×15 và lấy lo_lo.jpg; các mảnh title chỉ thuộc Boot. LSBg01–04 chỉ dùng khi vào map sau chọn nhân vật.

Native hiện báo 10 tác vụ thực: catalog item, tài nguyên panel, actor+terrain, tile/light/cursor, geometry terrain, skeleton, body, shadow, placements, environment. Không coi đây là đã port đủ OpenBasicData: các resource/class/effect/skill/sound chưa được triển khai vẫn chưa được nạp đầy đủ như gốc. Các tác vụ chỉ báo sau khi thành công; lỗi dừng Boot. Log thời gian ở /tmp/mu-client-bootstrap.log. Cache chỉ có trong tiến trình; lần khởi động mới đọc lại Data. Không có timer tự tăng phần trăm.

Sửa chiều vẽ JPEG riêng trong lobby (decode dùng tọa độ GPU khác AppKit), tránh đảo từng mảnh và đứt hình ở đường nối; không sửa decoder dùng cho world renderer.

## Nền chọn server 0.22 và giới hạn đối chiếu

Source tham chiếu CreateLogInScene dùng World 73 hoặc 77 theo PJH_NEW_SERVER_SELECT_MAP: World74/Object74/CWScript74.cws hoặc World78/Object78/CWScript78.cws. Character selection tương ứng World75 hoặc World79. OpenWorldModels ánh xạ Object(type+1).bmd; camera tour đọc WAYPOINT 28 byte (size_t Windows là 4 byte). Cần chuyển cả object/terrain/material, camera script và hiệu ứng/animation theo world tương ứng; không thay bằng ảnh chụp, spinner hoặc pan ảnh để gọi là cảnh gốc. Chưa xác định nhánh compile chính xác của executable EX603 từ DLL hook hiện có. Ảnh người dùng có nhãn Season 2 Pro là tham chiếu bố cục/hành vi, chưa chứng minh khớp cảnh EX603 trong Data.

Bản 0.22 nạp 375 placement, 45 loại model có sẵn; ghi rõ các model thiếu vào /tmp/mu-client-bootstrap.log. Marker hiệu ứng không vẽ thành hộp hình học; particle tương ứng chưa triển khai. Terrain dùng layer/alpha/lightmap World74, texture dùng chung giữa các chunk. Scene dừng render khi vào World hoặc cửa sổ bị che. Các model nền hiện dùng pose đầu, không coi camera chuyển động là đã có animation model đầy đủ. Nút cụm 108×26 và subserver 193×26 giữ kích thước UI, không phóng theo ảnh nền; khung chọn rộng 465 được căn giữa độc lập.

Sau phản hồi GUI: bỏ nút Refresh và lời nhắc chọn server; thêm Credit góc phải bằng server_credit_b_all.OZT (54×30). Nội dung đọc BuxConvert từ Local/Credit.bmd, 400 record × 33 byte theo CreditWin::LoadText. Cửa sổ Credit hiện là danh sách text trong game, chưa port slideshow/fade của CreditWin. Kiểm tra GUI đã phát hiện tour có góc nhìn xuyên terrain ở một số đoạn; kiểm thử một snapshot không đủ để xác nhận toàn vòng camera đúng. Chưa coi phần nền là hoàn tất hoặc tương đương executable gốc.
