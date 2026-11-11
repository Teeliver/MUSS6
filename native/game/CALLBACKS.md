# Callback coin shop trên native ARM64

Source Windows ghi function pointer và `gObj` pointer vào ba field DWORD để DataServer trả nguyên lại. Native dùng registry cục bộ: `CallbackFunc` là token32, `CallbackArg1` là nonce32, `CallbackArg2` vẫn là slot. Không đổi request/response layouts; các DataServer handlers vẫn chỉ echo ba field. Không chuyển pointer thành DWORD hay thực thi địa chỉ nhận từ network.

Registry giữ typed callback/object context trong process, kiểm tra buyer/target generation, account/name, channel và slot. Callback được consume trước khi chạy để reply trùng không mua lần hai. `gObjCharZeroSet`/`gObjDel` invalidates đời object, kể cả cùng account/name quay lại cùng index. Physical socket close của offline store không tự hủy character; cleanup gắn với object lifecycle thật.

Windows branch giữ callback cũ. Native send method trả kết quả `CConnection::DataSend`; gửi thất bại sẽ cancel và trả lỗi mua6 cho buyer còn đúng đời. Seller bị xóa/đổi đời trả lỗi3; buyer hết đời không nhận reply và không tác động user mới. Registry có TTL60s và tối đa4096 pending requests; hết hạn trả lỗi6, không giả success. Đây là cleanup native mới, vì source dùng raw pointer không có registry/TTL.

Cash callback inventory-full path trả lỗi8 và giải phóng PShopTransaction trên native, tránh giữ cửa hàng khóa mãi. Luật giá/coin, item insert/delete, save và các lời gọi gObjCoinAdd/Sub vẫn dùng callback C++ gốc; chưa biến nhiều lệnh persistence đó thành một transaction database mới.

Runtime phải gọi `mu::pruneCashCallbacks()` theo tick và `mu::cancelAllCashCallbacks()` khi DataServer disconnect/shutdown. Các callback/lifecycle operations chạy trên gameplay owner thread; mutex chỉ bảo vệ registry, không thay world ownership.

Tests offline: token nonce/channel/owner/slot mismatch, consume-once, buyer/seller reuse, send failure, expiry, capacity, disconnect, ready check, low-DWORD wrap; ABI32/44/36 bytes và callback fields4 bytes. Chưa xác nhận mua đồ với client/DataServer/VPS thật.
