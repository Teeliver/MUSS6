# HUD, tên và thông báo — 0.27.1

Server source thực: GameServer/Notice.h, Notice.cpp, User.cpp, ObjectManager.cpp,
Protocol.h. Engine UI tham khảo: /tmp/muonline-client-sources-master. Main_EX603
người dùng là DLL mở rộng; chưa có đầy đủ engine Windows tương ứng để xác nhận
parity 100%. Không đổi Data hoặc thêm resource vẽ thay thế.

| Thành phần | Source đối chiếu | Resource / hành vi |
|---|---|---|
| Khung640×480 | NewUIMainFrameWindow::RenderFrame | Interface/newui_menu01,02,03.OZJ; y429; 256/128/256×51 |
| Nút | SetButtonInfo nhánh không partCharge1, NewUIButton | newui_menu_Bt01..04.OZJ, x488+38i,y429,38×42; 4 hàng trạng thái |
| HP/MP | RenderLifeMana | newui_menu_red/blue.OZJ,45×39,x158/437,y432 |
| SD/AG | RenderShield/RenderSkillMana | newui_menu_SD/AG.OZJ,16×39,x204/420,y431 |
| Số | NewUIRenderNumber | newui_number1.OZT; glyph12×14; hiển thị8.4×11.2,bước6.72 |
| EXP thường | RenderExperience | newui_Exbar.OZJ; chia10 chặng; công thức prior EXP gốc |
| Tên | ZzzInterface::SetPlayerColor/RenderChat | font qua source_font; màu PK0..6; nền bản thân60,100,0 alpha150; cao120+60MU units |
| Notice type1 | WSclient::ReceiveNotice, NewUIChatLogWindow | chat hệ thống trái; RGB100,150,255; nền đen alpha150 |
| Notice type0/2 | ZzzInterface::RenderNotices | giữa tại y300; cam/vàng hoặc xanh guild; chưa đủ blinking/stack |

## Lỗi chiều ảnh và DPI

decode_texture(OZJ) lật hàng cho UV mesh3D. HUD AppKit nạp JPEG payload nguyên
chiều, ép NSBitmapImageRep.size bằng pixelsWide/High. JPEG Data có DPI khác
nhau: AG/SD150DPI, red69DPI, menu01 71DPI. Không dùng point theo DPI để cắt atlas.
Không thay decoder map/model. Sprite OZT đã được chuẩn hóa hàng bởi decode_tga.

## Dữ liệu thật và đếm ngược

E0/E1 lấy ViewCur/ViewMax32bit HP/MP/SD/AG; E0 có level/EXP. Packet26/27 cập
nhật current/max theo typeFF/FE. Damage cập nhật HP/SD bản thân. Packet9C cập
nhật EXP/NextEXP. Chưa có Master EXP hay gauge xanh khi poisoned.

Notice C1:0D kiểm tra length/NUL, callback chạy cả trong requestLogout chờ
F1:02. User.cpp gửi Message289 theo CloseCount-1: 5→1. Không có timer đếm giả.
Server hiện dùng chung câu “You will quit game...” cho cả các kiểu thoát; client
không tự đổi thành câu chuyển nhân vật hoặc hardcode bản dịch.
Đóng interface NPC trên server trước logout từ menu để tránh Warehouse chặn.

## Hiện trạng

Đã có khung, gauge/số thật, EXP thường, nút Inventory và tên bản thân/peer Player
đã có model trong viewport. Timer60Hz project tên, chỉ redraw khi đổi vị trí;
ảnh cache, không đọc file mỗi frame. Timer dừng khi rời map và bỏ cập nhật khi
cửa sổ không hiển thị. Footer debug và nút camera macOS được bỏ khỏi game.

Các nút Character/Friend/WindowMenu hiện dùng atlas disabled; không gán nút
WindowMenu sang Esc. Chưa có đầy đủ kỹ năng/hotkey dùng thuốc, bảng stats,
friend/window menu, guild/GM/title, luật hover/Alt/cloaking/event cho tên.
Tên hiện luôn cho player có model. Slide notice10..15 hiện đưa tạm vào chat,
chưa phải slide gốc. Notice2 chưa bổ sung prefix GlobalText483.

Kiểm thử: mu_client_hud_contract (resource, Retina, nút release/passthrough,
teardown), mu_client_contract (notice malformed). MuClient-switch-test opt-in
đã xác nhận vitals và 5→1 notice khi đổi nhân vật rồi đổi server trên socket thật.
Ảnh HUD fixture ở /tmp/mu-hud-corrected.png, không ghi số fixture vào database.

## 0.27.2 — camera cố định khi mở panel

Theo yêu cầu người dùng: Inventory và Warehouse phủ lên cảnh, không bù tâm
camera theo số cột UI. Bỏ offset projection.m31 cũ; giữ nguyên vị trí, hướng,
viewport và projection khi đổi 0/1/2 cột. Chỉ đổi tỷ lệ projection khi cửa sổ
thật sự đổi kích thước. Đây là lựa chọn UX người dùng yêu cầu, không khẳng định
là hành vi tự dịch camera của mọi bản Windows.
