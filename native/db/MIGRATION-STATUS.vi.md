# Trạng thái chuyển database EX603

Nguồn: `/Users/angelale/Downloads/MuServerEX603/DB/MuOnline.sql`.

Đã nhập vào container local `mu-native-mysql`, database `MuOnline`:

- 47 bảng, 11 dòng seed: GameServerInfo (1), DefaultClassType (7), WZ_CW_INFO (1), MEMB_INFO (2).
- 4 procedure trong `login-procedures.sql`: WZ_GetAccountLevel, WZ_SetAccountLevel, WZ_CONNECT_MEMB, WZ_DISCONNECT_MEMB.
- Schema đã nhập thử thành công vào MuOnlineNativeValidation2 trước khi nhập database chính.
- Kiểm thử procedure trên database thử: account không tồn tại trả NULL/NULL; connect ghi trạng thái 1; disconnect ghi trạng thái 0.

Đã thêm 5 procedure trong `character-procedures.sql`: WZ_CreateCharacter, WZ_GetResetInfo, WZ_GetMasterResetInfo, WZ_GetEventEntryInfo, WZ_GetItemSerial. Tổng cộng 9/63 procedure. Đã test login/list/create/select qua client native; account trở về offline, nhân vật lưu ở database.

Chưa hoàn tất: 54 procedure còn lại, kiểm thử account expiry đầy đủ, renderer và gameplay/save roundtrip.

Khác biệt có chủ đích: missing class template trả lỗi và rollback thay vì slot mồ côi; sửa lỗi gốc không ghi ITCount=0 khi reset ngày; tạo nhân vật có transaction/row lock chống tranh slot và tên trùng; item serial chỉ chạy khi có đúng một counter row (đúng seed hiện tại), từ chối cấu hình nhiều counter thay vì chọn số mơ hồ.

Giới hạn cần đối chiếu: HelperData.Data dùng VARBINARY(256) vì MySQL BINARY tối đa 255 byte; chưa mô phỏng tự động padding khi ghi dữ liệu ngắn. Collation latin1_swedish_ci và DATETIME chưa chứng minh tương đương collation và cách làm tròn smalldatetime của SQL Server gốc. Bảng MEMB_INFO gốc không có unique constraint trên tên account; procedure đọc một dòng nếu có tên trùng, cần xử lý dữ liệu trùng trước khi vận hành.

File schema/seed và cấu hình chứa mật khẩu nằm trong runtime gitignored, không đưa vào source control. Hai database kiểm thử được giữ lại để đối chiếu; database thử đầu tiên chỉ nhập được một phần trước khi phát hiện BINARY(256).

Client 0.2 đã kiểm thử đăng nhập, danh sách, tạo nhân vật, world-entry qua cả CLI và GUI. Chưa xác nhận gameplay hoặc parity 100%.

Bộ test `test_character_procedures.py` trên MuOnlineNativeValidation2 đã pass: tạo/duplicate/đủ 5 slot, missing-template rollback, binary templates, reset/event không chèn trùng, daily IT reset, concurrent name collision và item serial overflow rollback.

Đã thêm `guild-procedure.sql`: `WZ_GuildCreate(GuildName,MasterName)` trả 0 thành công / 1 guild tồn tại / 2 master đã ở guild khác; insert `Guild` + `GuildMember` với status 128. Tổng cộng 10/63 procedure. Đã test tạo Guild qua web và panel Guild Info.
