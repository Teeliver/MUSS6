# Rà soát OpenMU — 23/10/2026

Repository: `/Users/angelale/Projects/OpenMU`, HEAD `f883f4a8b`.

## Kết luận

Có lỗi lớn cần xử lý trước khi vận hành lâu dài: trade có thể làm Zen thành số âm. Đã tái hiện bằng chính GameLogic với player giả trong RAM. Ngoài ra, luồng trade chưa lưu item và Zen trong cùng một transaction, và cấu hình launcher hiện dùng demo/in-memory. Chưa đủ bằng chứng để chứng nhận server sẵn sàng production hoặc không có lỗi nhân đôi item.

Lượt này chỉ review và chạy kiểm thử; không sửa source nghiệp vụ, không restart server và không thay dữ liệu nhân vật đang chơi.

## Phát hiện ưu tiên

### 1. P1 — Trade vượt giới hạn Zen vẫn thành công và làm tiền âm — đã tái hiện

Vị trí: `/Users/angelale/Projects/OpenMU/src/GameLogic/PlayerActions/Trade/TradeButtonAction.cs:112`.

Kết thúc trade cộng trực tiếp `Money += TradingMoney`, không kiểm tra giới hạn cấu hình hoặc tính tổng bằng kiểu đủ lớn. `Player.Money` ghi thẳng vào Inventory.Money; đường này bỏ qua `TryAddMoney`.

Mô phỏng: A có 2.000.000.000 Zen, B đưa A 500.000.000 Zen. Cả hai xác nhận trade. Kết quả thực tế:

```text
TRADE requested=True; receiverZen=-1794967296; senderZen=0;
receiverState=Entered World; senderState=Entered World
```

Đối chứng gọi `TryAddMoney(500_000_000)` với ví 2 tỷ trả false và giữ nguyên tiền. Vì vậy đây là lỗi riêng của đường hoàn tất trade, không phải giới hạn packet ở client.

Cần kiểm tra số dư cuối của cả hai bên trước khi chuyển item, từ chối toàn bộ giao dịch khi vượt giới hạn, và đưa ca này vào test chính thức.

### 2. P1 — Item và Zen của trade chưa commit nguyên tử — xác nhận từ code, chưa thử crash PostgreSQL

Vị trí: `TradeButtonAction.cs:109` đến phần trả Success; `/Users/angelale/Projects/OpenMU/src/Persistence/EntityFramework/TradeContext.cs:31`.

TradeContext lưu thay đổi item trước, đồng thời Ignore<ItemStorage>. Zen được cập nhật sau SaveChangesAsync trong context của từng người; hàm hoàn tất không lưu hai số dư cùng transaction item. Periodic save mặc định một phút, và chỉ chọn player ở EnteredWorld.

Nếu process dừng giữa commit item và lưu số dư, item đã sang người nhận nhưng tiền có thể còn ở trạng thái cũ. Nếu hai context người chơi được lưu ở hai thời điểm khác nhau rồi process dừng, số dư hai bên cũng có thể không đồng nhất. Đây là cửa sổ rủi ro, chưa phải kết luận đã khai thác được nhân đôi item trên database hiện tại.

Cùng vùng code, try/catch bao gồm cả gửi view và gọi plugin sau khi item đã commit. Exception sau commit trả Cancelled, khiến caller gọi CancelTradeAsync và khôi phục BackupInventory trong RAM dù commit item đã xảy ra. Cần phân biệt lỗi trước/sau commit; rollback trong bộ nhớ sau commit không tương đương rollback database.

Cần một giao dịch durable bao gồm quyền sở hữu/slot item và số dư hai bên, cùng kiểm thử lỗi ở từng mốc commit, gửi view và plugin.

### 3. P1 cho triển khai — Launcher chạy demo nên dữ liệu không bền qua restart

Vị trí: `/Users/angelale/Projects/start-openmu.command:20`; `/Users/angelale/Projects/OpenMU/src/Startup/Program.cs:646`.

Launcher truyền `-demo`; Program chọn InMemoryPersistenceContextProvider. Đây là cấu hình của launcher đã đọc, không phải xác nhận command line của mọi process đang chạy. Phù hợp thử nghiệm nhưng không phù hợp lưu tiến trình chơi lâu dài. Không tự chuyển chế độ vì cần kế hoạch giữ dữ liệu thử nghiệm và cấu hình database.

### 4. P2 — Lưu thất bại khi rời game được ghi log rồi tiếp tục teardown

Vị trí: `/Users/angelale/Projects/OpenMU/src/GameLogic/Player.cs:1359`.

RemoveFromGameAsync bắt exception SaveProgressAsync và chỉ LogError. Vì exception không truyền ra ngoài, đường DisconnectAsync không biết lần save thông thường đã thất bại để dùng nhánh save dự phòng dành cho teardown exception. Khi database lỗi đủ lâu, người chơi có thể thoát bình thường nhưng lần đăng nhập sau mất tiến trình từ lần lưu thành công cuối.

Đã xác nhận cách xử lý từ source; chưa cố ý làm database thật mất kết nối. Cần chính sách retry/bảo toàn trạng thái chưa lưu và cảnh báo vận hành rõ ràng; không thể bảo đảm dữ liệu chỉ bằng log.

## Các phần đã đọc và đối chiếu

- LoginAction: authenticate, account reservation, load account, template account, nhả reservation khi lỗi.
- PlayerPersistence, RemotePlayer: khóa packet mutation so với save; đường disconnect và lưu định kỳ.
- TradeRequest/Accept/Money/Button/Cancel; BackupInventory, TradeContext và EntityFrameworkContextBase.
- MoveItemAction: kiểm tra storage, điều kiện, rollback khi AddItem thất bại.
- PlayerMovement: kiểm tra trạng thái, đường bị chặn và đồng bộ lại vị trí.
- PacketHandlerPlugInContainer và network parser/test: dispatch/version/encryption, xử lý kết nối.
- AdminPanel Startup/Auth và endpoint mới ClientServersController: controllers mặc định RequireAuthorization, endpoint danh sách server AllowAnonymous chỉ trả ID/mô tả. Chưa thấy endpoint này trả bí mật trong đoạn đã đọc.

Các cơ chế reservation trước load, khóa save với packet, rollback move item và teardown cô lập lỗi đã có trong source. Không ghi chúng là lỗi còn tồn tại chỉ dựa trên comment lịch sử.

## Kiểm thử thực tế

1. GameLogic/general: `dotnet test tests/MUnique.OpenMU.Tests/MUnique.OpenMU.Tests.csproj -p:ci=true`: **1.898 passed, 0 failed, 0 skipped**.
2. Network: `dotnet test tests/MUnique.OpenMU.Network.Tests/MUnique.OpenMU.Network.Tests.csproj -p:ci=true`: **73 passed, 0 failed, 4 skipped**. Bốn test real socket có Ignore sẵn; không tính là đã chạy.
3. Harness ngoài repository `/private/tmp/openmu-server-audit/Program.cs`: dùng PlayerTestHelper, TradeRequestAction, TradeAcceptAction, TradeMoneyAction, TradeButtonAction. Tái hiện trade tràn Zen ở trên. Không truy cập database hay tài khoản thật.
4. Build có warning sẵn về style/obsolete/threading; test pass không chứng minh không có lỗi nghiệp vụ ngoài coverage.

Lần gọi đầu với --no-restore không thực thi test khi chưa có assets; không tính vào kết quả. Kết quả trên là lần đã restore, build và có tổng số test thực tế.

## Giới hạn và thứ tự tiếp theo

Chưa thực hiện soak/load test nhiều giờ, chaos test database, khôi phục backup thật, kiểm thử PostgreSQL cho transaction trade, kiểm thử mọi event/quest/cash shop hay kiểm toán bảo mật toàn bộ code. Không suy ra sức chứa người chơi hoặc 60 FPS từ test unit.

Ưu tiên: sửa tràn Zen → thiết kế transaction trade và xử lý lỗi sau commit → chốt persistence/backup production → thử lỗi database/logout/reconnect → load test theo lượng người chơi mục tiêu. Giữ client PC làm tham chiếu protocol/hành vi; tính toàn vẹn database phải được bảo đảm ở server dù client gửi dữ liệu sai.
