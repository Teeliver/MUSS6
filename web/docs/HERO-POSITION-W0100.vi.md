# W0100 — thanh tên map/tọa độ và hai nút MU Helper

## Phạm vi đã chốt

Người dùng chọn **Chốt thanh tọa độ trước**. Chưa triển khai cấu hình/AI tự đánh–hồi máu–nhặt đồ của MU Helper. Hai icon gốc hiện đúng vị trí, tooltip ghi chưa khả dụng và aria-disabled; không giả trạng thái đang chạy, không gửi packet Helper khi click.

## Đúng code trong source

| Phần | Nguồn Main5.2/source | Ý nghĩa |
|---|---|---|
| Thanh chính | NewUIHeroPositionInfo.cpp:29–59,315–356 | CNewUIHeroPositionInfo, không phải Move hoặc minimap |
| Vị trí | NewUISystem.cpp:393 | Tạo tại (0,0) |
| Hai nút | NewUIHeroPositionInfo.cpp:79–133 | Setup ở WidenX+41, Start/Stop ở WidenX+59;18×13 |
| Chặn click xuyên | NewUIHeroPositionInfo.cpp:242–281 | Vùng UI chặn click xuống map |
| Cập nhật tọa độ | NewUIHeroPositionInfo.cpp:290–307 | Hero.PositionX/Y; không lấy vị trí con trỏ |
| Tên map | MapManager.cpp:1906–2070 | GetMapName, GlobalText và alias map sự kiện |
| Resource | NewUIHeroPositionInfo.cpp:467–489 | Nhánh 303<MAIN_UPDATE<=603; không dùng HUD_TEST của bản>603 |
| Setup | NewUIMacroMain.cpp, NewUIMacroSub.cpp | INTERFACE_MACRO_OFICIAL và các cấu hình phụ |
| Quy tắc Helper | CAIController.cpp | Điều kiện level/safe zone, controller tự động và trạng thái chạy |
| Bắt đầu/dừng | wsclientinline.h:2549 | C1 BF:51, status0/1; status2 cũng được controller sử dụng |
| Lưu cấu hình | wsclientinline.h:2558 | C2 AE:00 với buffer cấu hình |
| Phía server | GameServer/Protocol.cpp:440,674; Helper.cpp/.h | CGHelperDataRecv, CGHelperStartRecv, trả trạng thái/phí/điều kiện |

Nút Setup toggle bảng Helper và phát SOUND_CLICK01. Start kiểm tra CanUseAIController trước khi gửi; khi đang chạy thì nút đổi sang Stop. GUI và logic đó được ghi lại để làm bước sau, không thay bằng nút mở Move.

## Resource và bố cục

Đã audit/hash thêm sáu file gốc vào allowlist (tổng74 UI/table assets):

- Interface/Minimap_positionA.OZT
- Interface/Minimap_positionB.OZT
- Interface/MacroUI/Minimap_positionC.OZT
- Interface/MacroUI/MacroUI_Setup.OZT
- Interface/MacroUI/MacroUI_Start.OZT
- Interface/MacroUI/MacroUI_Stop.OZT

Nhánh không thêm custom menu: A22×25, B kéo ngang WidenX, C73×20. WidenX112 tại width800,96 ở nhánh else của nguồn. Text trắng, giữa vùng x+10 rộng WidenX+20, y+5; sprite nút18×13 theo dải đứng13pixel/state. Crop half-texel như NewUICommon. Dùng tỷ lệ panel chung của web đã có để cân với HUD/petHP.

Tên map dùng Text_eng.bmd thật; ví dụ map51→GlobalText1853=`Elveland`, khác nhãn server move `Elbeland`. Đã map các alias Blood Castle, Chaos Castle, Kalima, Barracks/Refuge, Imperial Guardian, v.v. Custom FindMap/textIndex chưa có bảng tương ứng trong phần source/Data hiện tại: map không xác định hiện `Map N`, không lấy nhầm các dòng Text không phải tên map.

## Tích hợp và kiểm tra

`world/hero-position.js` được tạo/hủy theo vòng đời map trong live-view. Theo dõi tile gần nhất của pose nhân vật, cập nhật khi đi/được điều chỉnh vị trí; đổi map tạo nhãn mới, bỏ nhãn cũ. Chạy trong offline walk và world thật. Không hiện trong màn đăng nhập/chọn nhân vật hoặc giữ lại sau reset.

39/39 test liên quan qua: tên map từ Data, alias sự kiện, tọa độ nguyên, crop/khung/nút theo nguồn, các hồi quy player/pet/cloth. Đã nạp lại riêng Identity asset helper để nhận allowlist mới; không restart GameServer hay thay nhân vật.

Kiểm tra trình duyệt ở cảnh offline riêng; không refresh hay thao tác phiên thật của người dùng. Bản offline có nhãn kiểm thử được dời xuống dưới thanh HP để không che thanh mới.

### QA cuối

Publish W0090-20261030-151302. Cảnh Elveland ở(52,225), click vào thanh giữ nguyên vị trí; đi sang phải cập nhật(54,225), dừng tại(55,225). Hai icon Helper hiển thị đúng, aria-disabled và tooltip chưa khả dụng. Thanh nằm trên HP pet, không bị nhãn OFFLINE che nữa. Ảnh `/tmp/mu-w100-elveland.png`, `/tmp/mu-w100-position-moving.png`, `/tmp/mu-w100-position-detail.png`.39/39 test liên quan qua; debug/release build thành công.
