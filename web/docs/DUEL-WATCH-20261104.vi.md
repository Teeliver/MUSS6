# Duel Watch — 04/11/2026

Đã triển khai và kiểm tra trọn luồng khán giả: mở Titus → chọn phòng đang đấu → vào Duel Arena → nhận điểm/HP/SD/danh sách khán giả → rời về Lorencia → đi lại. Lượt này không mở rộng map hoặc effect map.

## Đối chiếu source và DATA

- Server: `GameServer/Duel.h`, `Duel.cpp`, `NpcTalk.cpp::NpcTitus`, `Protocol.cpp` nhánh `0xAA`.
- Client gốc: `Main5.2/source/NewUIDuelWatchWindow.cpp`, `NewUIDuelWatchMainFrameWindow.cpp`, `NewUIDuelWatchUserListWindow.cpp`, `NewUICustomMessageBox.cpp::CDuelResultMsgBox`, `WSclient.cpp` và `NewUIHotKey.cpp`.
- Titus type479 ở Vulcanus (122,135). Trả `C3:30/result33`, sau đó `C1:AA:06`, bốn phòng, mỗi phòng22byte. Chỉ phòng đang đấu và còn chỗ mới bấm Watch.
- Join/leave gửi **C3:AA:07/09**, không dùng C1. Join **result0 là thành công**. Join ACK30byte chứa hai tên và hai index theo HB/LB. Spectator list tối đa10 tên.
- Score `AA04`, HP/SD `AA05` ghép theo index hai đấu sĩ; không ghép theo index người xem. `AA08/0A/0B` cập nhật khán giả; `AA0C/0D` kết quả/reset vòng.
- Rời phòng bình thường **không có AA09 ACK**: xác nhận bằng teleport ra khỏi map64. `AA09/result0` vẫn xử lý khi server buộc thoát.
- Generator ABI đọc thêm14 struct Duel trực tiếp từ header UPDATE603, dùng clang tính sizeof/offsetof. Tổng81 layout, không đoán padding.

## Giao diện và vòng đời

- Titus dùng khung190×429, bốn hàng và nút Watch53×23 với ba trạng thái up/hover/pressed; divider từ DATA.
- Khán giả dùng frame640×51 ở giữa phía dưới, score bead, HP/SD có phần chuyển tiếp, tên đấu sĩ và danh sách khán giả57×17 phía trên bên phải. Giữ giới hạn scale HUD hiện có.
- Bảng kết quả230×222 dùng `newui_DuelWindow`, message box và OK gốc; chữ GlobalText2694–2697, notice2689, âm thanh `SOUND_OPEN_DUELWINDOW`.
- Thêm12 ảnh gốc vào manifest, kiểm tra hash/kích thước/case; tổng137 asset UI. Không thay ảnh DATA, không thêm cache đĩa/HTTP.
- Khi xem: ẩn HUD/panel thường, tên/self actor/pet và effect trang bị của chính người xem; chặn di chuyển, đánh, cast, dùng/nhặt đồ, tương tác và Helper. Chat vẫn dùng được.
- Khi rời/kick/đổi map/đóng phiên: xoá trạng thái khán giả, trả HUD/điều khiển, huỷ request deadline và vòng RAF của UI. Timeout không đóng toàn bộ session.
- Có fixture riêng, không socket/đồ thật: `http://127.0.0.1:8080/?scene=ui-check&map=0&x=140&y=125&duelwatch=1`. Các nút HP/SD, Score, Round, Win, Kick chỉ nằm ở fixture này.

## Lỗi server được bắt bằng phiên thật

`CGDuelButtonRecv` trước đây xoá hai buff và membership kể cả khi `gObjMoveGate(17)` thất bại vì scene còn đang regen. Người xem bị mắc ở map64 và không còn membership để rời lại.

Đã đổi thứ tự: chỉ xoá buff/broadcast/remove membership **sau khi gate thành công**. Không thêm opcode/result giả. Nếu server gửi lại map64 khi bấm rời, web huỷ pending leave, giữ chế độ xem và thông báo thử lại sau khi chuyển cảnh xong. Có test C++ trích chính handler đang build để kiểm tra cả gate thất bại/thành công và input không hợp lệ.

GameServer mới đã build và được nạp sau khi kiểm tra không còn tài khoản online. Chỉ helper resource được nạp lại để nhận manifest mới; Connect/Join/Data/Gateway giữ nguyên.

## Kết quả kiểm tra

- **485/485 web test qua**, cây test cô lập; bao gồm codec/ABI, ghép index, deepcopy, spectator cap/dedup, hai thứ tự warp/ACK, leave không ACK, kick, timeout/retry/disconnect, frame lỗi, chặn gameplay, gate refusal và RAF/disposal.
- **34/34 native CTest qua**. Bốn test Metal ban đầu không chạy trong sandbox; chạy lại với quyền GPU đã qua đủ34. Không sửa test để bỏ kiểm tra render.
- Debug build **173 file allowlist**, kiểm tra graph import trước publish.
- Browser: kiểm tra phòng trống/đầy bị khoá, join, score3–7/HP/SD, kết quả, OK và rời trả HUD. Quan sát khoảng60FPS ở viewport791×899. Ảnh [đang xem](reports/duel-watch-20261104/watch.png), [kết quả](reports/duel-watch-20261104/result.png), [rời phòng](reports/duel-watch-20261104/leave.png).
- Server thật: ba QA level100 tạo từ template, hai đấu sĩ challenge/accept ở Vulcanus; khán giả nói chuyện Titus, nhận bốn phòng, vào arena, nhận AA07/0B/05/04 thật. Bấm rời **ngay sau join** đã tái hiện gate refusal; bản mới giữ membership, retry rời về Lorencia và nhận ACK đi thêm một bước. [Packet/state trace](reports/duel-watch-20261104/live-server.json).
- Dữ liệu thật đã kiểm chứng điểm khởi tạo0–0, HP/SD100%; mức damage/score khác0, round reset và bảng thắng được kiểm tra qua codec/state/fixture. Chưa chứng nhận toàn bộ chiến đấu PvP/skill/effect hoặc mười spectator đồng thời.

## Dọn dữ liệu và tiếp tục

Đã đóng/xoá đủ3 QA account/character cùng dữ liệu phụ: tổng36 hàng (12 bảng ×3); kiểm tra còn0 hàng QA. Không sửa tài khoản có sẵn. [Biên bản](reports/duel-watch-20261104/qa-cleanup.json). Snapshot code duy nhất `native/runtime/backups/mu-web-code-latest.tar.gz` được thay bản cũ; giữ riêng hai bản DB/MuOptions gần nhất của từng nhóm. Cây test tạm/credential QA được dọn sau khi giữ log.

Duel Watch đã đóng trong phạm vi trên. Việc chức năng còn lại từ handoff: ngày tạo Guild/Edit Guild Mark và tooltip3D; PC Point UI vẫn thiếu source tương ứng, không tự dựng GUI thay thế. Các thiếu sót map giữ ở báo cáo map riêng, không trộn lại vào lượt Duel.
