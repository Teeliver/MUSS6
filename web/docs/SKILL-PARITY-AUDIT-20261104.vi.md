# Audit parity Skill / Effect — 2026-11-04

Sinh bằng `web/tools/audit_skill_parity.py`. Chỉ đọc source/data, không sửa gameplay.

## Nguồn

- PC client: `Skill_eng.bmd` — **399** skill có tên (650 record).
- Server: `SkillList.txt` — **386** skill.
- Web catalog: **386** skill.
- PC effect dispatch: **157** tên `case` duy nhất, **601** call `Create*` trong `ZzzCharacter.cpp`.

## Metadata

- Client-only: **15** — `[203, 204, 314, 321, 323, 373, 374, 395, 409, 445, 475, 504, 548, 574, 592]`
- Server-only: **2** — `[75, 216]`
- Tên khác: **15** — `[(10, 'Hellfire', 'Hell Fire'), (48, 'Swell Life', 'Sweel Life'), (62, 'Earthshake', 'Earthquake'), (65, 'Electric Spike', 'Electric Spark'), (200, 'Summon Monster', 'Monster Summon'), (201, 'Magic Attack Immunity', 'Magic Damage Immunity'), (202, 'Physical Attack Immunity', 'Physical Damage Immunity'), (213, 'Shied-Burn', 'Shield-Burn'), (320, 'Strength Increase', 'AStrength Increase'), (415, 'Summoned Monster Strengthener (1', 'Summoned Monster Strengthener (1)'), (419, 'Summoned Monster Strengthener (2', 'Summoned Monster Strengthener (2)'), (428, 'Summoned Monster Strengthener (3', 'Summoned Monster Strengthener (3)'), (446, 'DimensionWings Defense PowUp', 'Dimension Wings Defense PowUp'), (447, 'DimensionWings Attack PowUp', 'Dimension Wings Attack PowUp'), (617, 'Increase Triple Damage Chance', 'Recovers SD Fully')]`
- Class khác: **19** — `[(9, [1, 0, 0, 1, 0, 0, 0], [1, 0, 0, 1, 0, 1, 0]), (18, [0, 1, 0, 1, 1, 0, 0], [0, 1, 0, 1, 1, 1, 0]), (19, [0, 1, 0, 1, 1, 0, 1], [0, 1, 0, 1, 1, 1, 1]), (20, [0, 1, 0, 1, 1, 0, 0], [0, 1, 0, 1, 1, 1, 0]), (21, [0, 1, 0, 1, 1, 0, 0], [0, 1, 0, 1, 1, 1, 0]), (22, [0, 1, 0, 1, 1, 0, 0], [0, 1, 0, 1, 1, 1, 0]), (23, [0, 1, 0, 1, 1, 0, 0], [0, 1, 0, 1, 1, 1, 0]), (39, [2, 0, 0, 0, 0, 0, 0], [2, 0, 0, 1, 0, 0, 0]), (67, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (68, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (69, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (70, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (71, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (77, [0, 0, 2, 0, 0, 0, 0], [0, 0, 1, 0, 0, 0, 0]), (210, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (211, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (212, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (213, [1, 1, 1, 1, 1, 1, 0], [1, 1, 1, 1, 1, 1, 1]), (239, [0, 0, 0, 1, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0])]`
- Range khác: **85** — `[(6, 6, 0, 0), (9, 6, 0, 6), (15, 6, 0, 0), (24, 6, 0, 6), (40, 6, 0, 6), (41, 2, 0, 3), (42, 3, 0, 4), (47, 3, 6, 6), (56, 5, 0, 5), (62, 10, 0, 5), (64, 0, 6, 6), (65, 10, 0, 6), (67, 2, 0, 5), (70, 0, 6, 0), (76, 6, 0, 5), (77, 6, 0, 0), (78, 6, 4, 6), (79, 2, 0, 0), (217, 5, 6, 0), (218, 5, 0, 0), (221, 6, 0, 5), (222, 6, 0, 5), (230, 6, 0, 5), (232, 5, 6, 3), (233, 6, 0, 0), (235, 6, 0, 6), (236, 3, 0, 5), (237, 6, 0, 6), (238, 6, 0, 6), (239, 3, 0, 3), (263, 5, 4, 3), (266, 3, 0, 0), (267, 7, 0, 5), (268, 7, 0, 5), (330, 2, 0, 3), (331, 3, 0, 4), (332, 2, 0, 3), (333, 3, 0, 4), (336, 3, 2, 2), (337, 5, 6, 3)]`

Các field `level`/`energy` của client và `ReqLevel`/`ReqEnergy` của server khác ngữ nghĩa; không dùng để kết luận lỗi nếu chưa xác minh.

## Effect coverage

- Active: **114**; có spec: **114**; missing: **0**.
- Exact: **3** `[4, 9, 17]`
- Source token: **106**
- Derived (không có token nguồn riêng): **0** — `[]`
- No visual (source xác nhận không có cast effect): **5** — `[212, 344, 495, 521, 565]`

Derived chỉ là họ hình ảnh gần đúng; chưa chứng minh hình học/vật liệu/timing/âm thanh.

### No-visual trong PC client
- `212`: AT_SKILL_CURSED_TEMPLE_TELEPORT chỉ SetAction(PLAYER_HIT_REMOVAL), không có Create*
- `344`: Skill_Blood_Storm chỉ có enum + xử lý server, không có case/Create* trong client
- `495`: Skill_Earth_Prison chỉ có buff icon EFFECT_EARTH_PRISON, không có cast effect client
- `521`: Skill_Iron_Defense chỉ có buff icon EFFECT_IRON_DEFENSE, không có cast effect client
- `565`: Skill_Blood_Howling chỉ có buff icon EFFECT_BLOOD_HOWLING, không có cast effect client

## Skill active chưa có spec

Không có.

## Việc cần làm

1. Chốt field metadata nào là chuẩn (PC client hay server custom).
2. Port effect thật cho 0 derived / các nhánh `Create*` chưa map.
3. Diff damage/range/cooldown/buff server ↔ PC; kiểm tra master skill tree và buff UI.
