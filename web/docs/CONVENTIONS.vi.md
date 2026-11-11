# Quy ước dữ liệu web/core

Ngày 2026-10-23. Đây là hợp đồng kiến trúc M01, chưa phải codec hoặc core đã triển khai.

- Item ID giữ nguyên `section * 512 + index` theo GameServer/ItemManager.h. Không đổi sang chỉ số liên tục riêng cho web. Slot inventory là trường riêng với item ID; item instance/options không gộp vào ID.
- Base class theo DefaultClassInfo.h: DW=0, DK=1, Elf=2, MG=3, DL=4, Summoner=5, RF=6. DB class và packet class có encoding riêng; không truyền base class thay DB class. Việc ánh xạ cần fixture M02.
- Server map ID giữ nguyên; thư mục WorldN có N=serverMap+1 trong loader hiện có. Không suy ra model naming/tile naming mọi map từ phép cộng này.
- Gameplay state giữ tile X/Y theo server. Renderer dùng trục Y hướng lên; adapter native hiện tại đặt `(x,height,-y)`. Object vị trí source dùng 100 đơn vị/tile; phải chuyển đúng một lần tại ranh giới asset/render. Không thay đổi tọa độ authoritative để phù hợp camera. Height và góc model giữ metadata source; chuyển đơn vị/rotation cần fixture riêng, không mặc định mọi góc đều cùng đơn vị.
- Scalar packet nhiều byte phải đọc/ghi endian tường minh theo từng field. Length C2/C4 là big-endian (byte cao trước), C1/C3 là một byte; không dùng một quy tắc endian chung cho cả packet. Tham chiếu native/client/client.cpp, Socket::frame. Không assume một WebSocket message là một MU frame.
- Server sở hữu HP, damage, XP, drop, tiền, item và kết quả giao dịch. Browser giữ state phản chiếu và animation dự đoán; không tự xác nhận mua/nhặt/mặc đồ hoặc kết quả đánh.
- Core tương lai sở hữu codec/state thuần; transport cấp byte bất đồng bộ, renderer nhận snapshot/event. DOM, WebSocket, clock, audio, file/cache, UUID và Cocoa không được nhập ngược vào core.
- Mọi asset xuất sau này giữ path/ID gốc, schema version, hash và dependency; thiếu/không hỗ trợ phải ghi rõ. Không đổi resource bằng hình tự tạo để che lỗi.
- Không credential/config server trong frontend hoặc log packet thô có login. Gateway chỉ tới route allowlist; identity/ClientVerify browser còn chưa thiết kế, không tắt kiểm tra server để vượt qua.
