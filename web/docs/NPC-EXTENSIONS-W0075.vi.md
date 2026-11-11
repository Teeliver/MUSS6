# W0075 — NPC mở rộng (đang thực hiện)

Yêu cầu: các chức năng riêng theo từng NPC, toàn bộ NPC đang có trong source603. Không làm tối ưu/prewarm cache trong mốc này theo yêu cầu chờ của người dùng.

Kế hoạch triển khai: sinh registry từ dispatch NPC sau preprocess603; tích hợp các nhóm kho/transaction container, chaos/trainer/harmony/socket, jewel mix, guild/Gens/quest và NPC event/custom. Mỗi packet phải kiểm tra cả hàm gửi gốc (Send TRUE/FALSE), không dựa riêng chú thích header.

Nguồn: GameServer/NpcTalk.cpp; Warehouse/ChaosBox/JewelMix/Guild/GensSystem/Quest/QuestWorld; Main5.2 NewUI tương ứng và wsclientinline.h; cấu hình Data/Monster, ShopManager, CustomNpcCommand/Move. Đã phát hiện schema CustomNpcCommand.txt đang có Index/NpcTalk nhưng parser NpcCommand.cpp chờ5 trường — cần xác minh loader runtime trước khi thay cấu hình. Không tự click NPC reset/PK để test trên nhân vật.

Chưa đánh dấu hoàn tất. Ghi triển khai và kết quả theo từng nhóm bên dưới.

## Thu hồi bản test sau phản hồi lỗi giao diện

Đã tìm thấy `web/client/style.css` bị ghi nhầm toàn bộ JavaScript của world-session. Test protocol vẫn qua nhưng không kiểm tra nội dung CSS; giao diện bị mất định dạng toàn trang. Đã khôi phục CSS nguồn và cả debug/release đang phục vụ từ bản backup trước W0075 (`W0075-20261027-100328`). Giữ code NPC mở rộng ở nguồn để sửa/kiểm tra riêng; chưa coi là bản hoàn tất và không triển khai lại vào phiên người dùng. Build thêm chặn nội dung import/export JavaScript trong CSS trước khi thay output.

## Triển khai tiếp sau thu hồi

Đã tách server test giao diện tại localhost:8082, dùng bản sao file công khai trong allowlist, phục vụ tĩnh với SHA256. Không proxy helper, không sửa giới hạn Origin của helper. 8080 chỉ được cập nhật lại sau kiểm tra UI và test. Proxy giả Origin đã bị auto-review từ chối và không được chạy.

| Nhóm | NPC / chức năng đã nối |
|---|---|
| Kho | Safety Guardian: kho 8×15 và trang mở rộng, nhấc/đặt với túi/trang bị, gửi/rút Zen, mở khóa/đặt/xóa mật khẩu. Trạng thái khóa ban đầu chưa biết thì chặn chuyển đồ. |
| Mix | Chaos Goblin, Trainer, Elpis, Osbourne, Jerridon, Chaos Card Master, Cherry Blossom, Seed Master, Seed Researcher, David: container đúng flag, công thức theo enum603, hỏi tỷ lệ/phí server, kết hợp. Phải lấy hết đồ trước khi đóng. |
| Ngọc | Lahap/Raul: 10 loại ngọc đúng thứ tự JewelMix.cpp, gộp 10/20/30, tách bó, phí do server trừ. |
| Nhiệm vụ | F9/F6: lời thoại NPC, Gens/buff, danh sách/nhận/hoàn thành/hủy quest. A0/A1/A2/A4: Sebina, Marlon, Devin; hội thoại nguyên bản và bộ đếm quái. Werewolf/Gatekeeper: cửa nhiệm vụ D0:07/08. |
| Gens/Guild | Duprian/Vanert: gia nhập/rời/nhận thưởng, cập nhật theo ACK. Guild Master: form tên, vẽ huy hiệu 8×8 và gửi tạo guild. |
| Sự kiện | Charon/Blood Castle/Illusion Temple/Lugard/Jerry: chọn vé, gửi yêu cầu vào đúng format của từng event. Golden Archer nộp Rena; Delgado nộp/đổi Lucky Coin 10/20/30. Titus xem trận đấu. |
| Dịch chuyển/quà | Julia, Kanturu; Leo/Santa/Snowman; NPC Christmas/bàn thờ IT/chest BC được gửi theo hành vi trực tiếp của source, không chờ gói mở shop không tồn tại. |

Bộ Data gốc đã giải mã có hash nguồn: 2.288 đoạn QuestWords, 58 NPCDialogue, 915 QuestProgress; 7 quest đổi nghề và 200 slot Dialog (111 slot có lời thoại). Struct legacy quest744 byte, act bắt đầu38 (không phải40); request424. Quest detail251 byte là packed, phải đọc cả5 slot rồi bỏ slot type0. NextIndex của server chỉ cho lựa chọn1..3; reject option ngoài phạm vi.

Nút kho dùng newui_Bt_money01/02, newui_Bt_lock/lock02, newui_expansion_btn; nền tiền newui_item_money3; nút mix newui_Bt_mix. Tổng62 UI/table assets kiểm hash. Đã sửa specificity CSS làm nút hộp thoại bị trong suốt. Có regression test build thật: ghi JS vào CSS phải thất bại trước khi thay dist hợp lệ.

Audit từng NPC: [artifacts/npc-extensions-W0075.json](artifacts/npc-extensions-W0075.json). Phân loại64 loại NPC trong spawn catalog; shop thường đã có luồng riêng từ W0074. 12 loại thuộc công thành/Crywolf phụ thuộc server CS, không tuyên bố đã kiểm chứng tại GameServer thường. Danh mục hiện chưa có spawn501/502; config Reset/PK vẫn lệch schema parser, giữ nguyên và không thử reset/PK nhân vật.

Giới hạn: UI chọn công thức, form/hộp xác nhận và một số layout sự kiện đang được thích nghi cho web, chưa pixel parity100%. Nhiều NPC còn thiếu model trong resource catalog từ trước; chức năng packet không tự giải quyết thiếu model. Túi mở rộng nhân vật vẫn chỉ xem như trước; chuyển đồ NPC hiện dùng túi chính/trang bị. Agent không thực hiện mix tiêu hao, mua/bán, nhận thưởng, đổi Gens/tạo guild hay thay mật khẩu thật. Các hành vi đó cần người dùng kiểm tra trên nhân vật; test dùng packet/fixture giả lập.

## Kết quả bản cập nhật lại

Build debug93/release32, stage179 preview assets. Toàn bộ test client qua;4 test helper qua khi có quyền bind localhost. Đã kiểm tra mở27 nhóm/biến thể NPC trên server test riêng; không có console error. Sau đó cập nhật8080, reload helper để nạp62 UI/table assets với cùng giới hạn Origin/Host cũ. Mở kho từ8080 xác nhận103 CSS rules,0 icon error, không có lỗi console. Screenshot `/tmp/mu-w75-npc-storage-verified.png`. Chưa giao dịch tiêu hao trên nhân vật thật, chưa xác nhận toàn bộ event CS/gameplay trên server live.
