# Chạy MU Web trên Mac — hướng dẫn hiện tại

Mở Docker Desktop, chờ Docker chạy xong. Mở Terminal và dán:

```sh
cd "/Users/angelale/Downloads/SOURCE-2.0.7/SOURCE/Source/Emulator"
./native/MU-Web.command
```

Launcher dùng các bản đã build sẵn, bật MySQL → Connect → Join → Data → Game → Identity → Gateway → Web. Dịch vụ có cổng đang nghe được giữ nguyên, không mở thêm bản trùng. Các process mới chạy nền; đóng Terminal không dừng game. Log và PID vẫn ở `native/runtime/background/`. Không build lại, không xóa dữ liệu và không thay mật khẩu.

Mở game: http://127.0.0.1:8080/?scene=servers

Kiểm tra các cổng:

```sh
./native/MU-Web.command status
```

Nếu launcher dừng ở một server, đọc log được chỉ ra; ví dụ:

```sh
tail -n 50 native/runtime/background/GameServer.log
```

Sau khi xử lý lỗi, chạy lại cùng launcher. Không chạy đồng thời các launcher Start-... riêng lẻ. Không dùng `start-openmu.command` cho bộ MU Web này: file đó trỏ sang dự án OpenMU khác.

`status` chỉ xác nhận cổng đang nghe, chưa chứng minh đăng nhập/gameplay hoặc lưu dữ liệu thành công. Launcher không tự dừng hay khởi động lại dịch vụ đang chạy, tránh ngắt phiên chơi. Sau khi khởi động lại máy, mở Docker và chạy cùng hai lệnh ở trên.

Kiểm tra đủ file mà không mở dịch vụ: `./native/MU-Web.command check`.

## Lỗi “Không khởi tạo được phiên” — 2026-10-31

Đã xác định trường hợp Web thiếu `web/dist/debug/runtime/live-profile.json` sau build, dù helper9001/Gateway9000/Game55900 vẫn đang nghe. Đã khôi phục runtime, preview-assets và audio-assets từ bản deploy W0114-20261031-133023. Build hiện giữ lại năm thư mục tài nguyên runtime/preview/audio/ui/map-title và xuất code vào staging trước khi đổi thư mục. Launcher kiểm tra profile trước khi chạy.

Kiểm tra: hai lần build liên tiếp trong thư mục cô lập giữ nguyên profile và năm thư mục tài nguyên; trình duyệt chọn GameServer_0 đã mở phiên và hiện form đăng nhập. Chưa đăng nhập tài khoản trong lần kiểm tra này.
