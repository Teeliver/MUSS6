# Bộ Data EX603 đang dùng và đối chiếu bộ cũ

Runtime hiện dùng `/Users/angelale/Downloads/MuServerEX603/GameServer`, với Data tại `/Users/angelale/Downloads/MuServerEX603/Data`. Bộ này có đủ116 đường dẫn input được tham chiếu và61 file spawn; Item/Item.bmd là output, không bắt buộc startup. Đã test world initialization thành công, listener127.0.0.1:55900 và Join/Data TCP đều kết nối. Chưa xác nhận procedure, đăng nhập hoặc gameplay với client thật.

## Đối chiếu bộ MuServer cũ (tham khảo)

Đã đọc parser C++ và bộ `/Users/angelale/Downloads/MuServer/Data`. Đã thêm BlackList.txt đúng hai section IP/HID, chưa có mục bị chặn; bản cấu hình nằm trong native/config/game. Audit còn46/117 đường dẫn thiếu, cùng dependency glob Monster/Spawn. Chưa chuyển dữ liệu gameplay hoặc tạo file thay thế cho dữ liệu thiếu. Kết quả này kiểm tra định dạng, chưa xác nhận đầy đủ gameplay/cross-reference.

## Có thể copy nguyên bytes sang đường dẫn mới

| File hiện tại dưới Data | Đích dưới Data |
|---|---|
| Skill/Skill.txt | Skill/SkillList.txt |
| Monster/MonsterAIAutomata.txt | Monster/AI/MonsterAIAutomata.txt |
| Monster/MonsterAIElement.txt | Monster/AI/MonsterAIElement.txt |
| Monster/MonsterAIGroup.txt | Monster/AI/MonsterAIGroup.txt |
| Monster/MonsterAIRule.txt | Monster/AI/MonsterAIRule.txt |
| Monster/MonsterAIUnit.txt | Monster/AI/MonsterAIUnit.txt |
| Monster/MovePath/MovePath_35.dat | Monster/AI/MonsterAIMovePath.txt |
| Monster/MonsterSkill.txt | Monster/Skill/MonsterSkill.txt |
| Monster/MonsterSkillElement.txt | Monster/Skill/MonsterSkillElement.txt |
| Monster/MonsterSkillUnit.txt | Monster/Skill/MonsterSkillUnit.txt |

## Cần chuyển cả cấu trúc/nghiệp vụ

| Candidate | Khác biệt |
|---|---|
| Command.txt → CommandManager.txt | 31→28 cột, thứ tự khác và mã command khác: ID1 cũ là Post, ID1 mới là GuildWar, Post mới là4. |
| Monster/Monster.txt → Monster/MonsterList.txt | 28→31 cột, 4→7 resistance; Rate cũ trở thành Type mới. Giá trị1 của Bull Fighter sẽ làm nó thành NPC nếu copy mù. |
| Effect.txt → Skill/EffectList.txt | ItemIndex gộp phải tách Section/Index. |
| Custom/CustomCombo.txt → Skill/ComboSkill.txt | Danh sách chia section phải chuyển thành dòng Class/Index/Stage/Delay. |
| Custom/CustomStartItem.txt → Character/DefaultClassFreebies.txt | Khác section/item/socket/buff; thiếu dữ liệu tương ứng. |
| Hack/HackSkillCheck.txt → Hack/HackSkillSpeedCheck.txt | 5→10 cột, thêm class/level/physical/magic speed. |
| MapServerInfo.dat → MapServerInfo.txt | Cột/section khớp, nhưng IP có prefix S. Parser mới không bỏ prefix trước khi gửi endpoint cho client. |
| Event/MuCastleData.dat → Event/CastleSiege.dat | Đảo Side/DBSave; 14/24 NPC rows có hai giá trị khác nhau. Chỉ dùng với ServerType1. |

Monster/MonsterSetBase có29 file map top-level và38 file Event. Source mới lấy MapNumber từ tên file, nên dòng6/9 cột phải chuyển thành5/8. 29 file top-level có3761 records, map của mỗi record khớp tên file. Event có section4 dạng9 cột cần xử lý riêng; không flatten/copy nguyên trạng.

## Chưa có dữ liệu tương đương

- Base: ConnectMember.txt, MapRateInfo.txt. BlackList.txt đã được cấu hình riêng với danh sách IP/HID chưa có mục bị chặn.
- Custom: CustomAttack, CustomGift, CustomHealthBar, CustomLukeHelper, CustomMarriage, CustomMonsterKillCount.
- Event: DropEvent, EventEntryLevel, EventStageSpawn, Kalima.
- Item: ExcellentOptionRate, ItemLevel, ItemLoot, ItemMoneyDrop, WingOption.
- Monster: MonsterMove, MonsterSchedule, NpcTalk.
- Skill: HelperSkill, SkillEnabler, SkillUseArea.
- Util: DamageTable, OnlineReward, ResetLimit.
- Script/ScriptMain.lua: bộ hiện tại không có subtree Script; Lua loader báo lỗi và tiếp tục.

Item/Item.bmd là output của WriteItemInfo, không phải input bắt buộc startup. Nhiều file feature vẫn load vô điều kiện dù switch tắt. Sau khi thêm BlackList.txt, loader đã đọc qua nó và dừng tại Character/DefaultClassFreebies.txt. Copy10 mapping an toàn vẫn chưa đủ làm GameMainInit thành công. Cần chuyển/cấu hình có chủ đích từng feature, không thể xác nhận100% từ đổi tên file.
