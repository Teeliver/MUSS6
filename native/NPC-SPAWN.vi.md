# Thêm NPC trong bộ EX603 hiện tại

File Lorencia: /Users/angelale/Downloads/MuServerEX603/Data/Monster/Spawn/000 - Lorencia.txt. MonsterSetBase.cpp đọc các tên dạng `NNN - ...`, dùng ba số đầu làm map id.

Section0 dùng mỗi dòng `MonsterIndex SpawnRadius X Y Direction`, kết thúc bằng `end`. Ví dụ đang có `240 0 146 110 3`: Warehouse, đứng ở146,110, hướng3. Không đặt NPC trong ô chặn; kiểm tra ATT và model đúngClass trước. Section1 thêm RangeEndX/RangeEndY/MaxSpawnCount, dành cho spawn vùng và số lượng.

MonsterList.txt định nghĩa class; các chức năng shop/hội thoại còn phụ thuộc mapping và NpcTalk/GameServer, không phải chỉ tạo dòngspawn là có shop. Client nhận spawn từ server, chọn model qua actor catalog; class chưa hỗ trợ model/attachment sẽ chưa hiện trên bản native. Nên chỉnh một NPC thử trước, lưu bản sao file, dùng chức năng reload phù hợp hoặc restartGameServer để nạp lại. Chưa tự sửa runtime này.
