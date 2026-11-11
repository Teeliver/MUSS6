# W0115 — Bao phủ skill 7 class (2026-10-31)

## Trạng thái hiện tại

**114/114 skill chủ động cast được bởi player đã có effect tường minh. 0 missing.**

| Trạng thái | Nghĩa | Số lượng |
|---|---|---:|
| `exact` | Prototype gốc đã kiểm chứng (Fire Ball 4, Evil Spirit 9, Energy Ball 17) | 3 |
| `source` | Có `CreateEffect` / `CreateJoint` / `CreateParticle` thật trong `ZzzCharacter.cpp` | 60 |
| `derived` | Cùng họ hình ảnh với skill gốc của class, chưa có token nguồn riêng | 51 |
| `missing` | Không có effect | 0 |

Phạm vi: `id < 300`, `classes` có ít nhất một class, và `mana > 0` hoặc `AG > 0`; cộng 10 master skill có handler thật trong `CSkillManager::RunningSkill`. Hai skill dùng AG là 62 Earthquake và 65 Electric Spark. Các master-skill passive như 511/515/517, 79 Explosion (không có handler server) và các skill có `classes` toàn 0 không được tính.

## Theo class

Một skill đa class được đếm ở mọi class sở hữu.

| Class | Tổng skill chủ động | Exact | Source | Derived |
|---|---:|---:|---:|---:|
| DW | 33 | 3 | 21 | 9 |
| DK | 25 | 0 | 14 | 11 |
| Elf | 32 | 0 | 14 | 18 |
| MG | 41 | 3 | 25 | 13 |
| DL | 29 | 0 | 14 | 15 |
| Summoner | 35 | 2 | 18 | 15 |
| RF | 23 | 0 | 7 | 16 |

## Đã kiểm

- Dispatch `switch (c->Skill)` trong `Main5.2/source/ZzzCharacter.cpp` được trích tự động; 63 token effect/joint/particle có thật (3 exact + 60 source). Mọi token `source` trong catalog đều xuất hiện nguyên văn trong C++.
- 10 master skill chủ động (344, 425, 426, 427, 430, 432, 461, 495, 521, 565) có case thật trong `CSkillManager::RunningSkill`; quét toàn bộ `case SKILL_*` trong `GameServer` cũng chỉ có đúng 10 id ≥ 300 này. 511/515/517 là passive nên không gán effect.
- 13 họ hình ảnh trỏ tới 13 texture gốc trong `Data/Effect`; audit giải mã đủ 13/13.
- 12 mode hình học giữ số mesh cố định theo thời gian; test kiểm tra positions/normals/uv hữu hạn và topology không đổi ở age 0 / giữa / cuối.
- 315/315 client test pass; build debug 147 file; file build khớp source.
- Không sửa dữ liệu server, không thêm asset ngoài allowlist.

## Ý nghĩa của `derived`

`derived` là họ hình ảnh gần nhất theo class/nhóm kỹ năng, **không phải** trích xuất 1:1 từ client Windows. 51 skill này chưa có token nguồn riêng, nên không được tính là parity.

## Giới hạn

- Chưa port đầy đủ material, animation curve, scale, âm thanh và hit timing từng skill.
- Preview menu vẫn chỉ dựng 3 prototype gốc; các skill khác hiển thị nhãn `source`/`derived`. Effect đầy đủ nằm ở world scene khi cast.
- Mỗi effect generic dùng một renderer riêng, giới hạn 12 effect đồng thời; chưa đo FPS khi đông effect.
- Chưa smoke test browser từng skill; mới có unit test hình học + audit texture + build.
- Damage/MP/cooldown/knockback vẫn do server quyết định.

Chi tiết kiến trúc, bảng 114 skill và bằng chứng: [SKILLS-FULL-W0115.vi.md](SKILLS-FULL-W0115.vi.md).

## Chạy lại

```bash
cd SOURCE/Source/Emulator/web
npm test
node tools/audit_skill_effects.mjs /Users/angelale/Downloads/MuClientEX603/Data
npm run build:debug
```

## Lịch sử W0053

Trước W0115, báo cáo cũ chỉ có 3 prototype và 383 skill chưa ghép, vì khi đó chưa tìm thấy `ZzzCharacter.cpp`/`ZzzEffect.cpp` trong cây source đang dùng. Bảng cũ được giữ ở đây để đối chiếu, không còn là trạng thái hiện tại:

| Class | Skill config | Effect prototype | Chưa ghép |
|---|---:|---:|---:|
| DW | 100 | 2 | 98 |
| DK | 93 | 0 | 93 |
| Elf | 97 | 0 | 97 |
| MG | 110 | 2 | 108 |
| DL | 100 | 0 | 100 |
| Summoner | 97 | 1 | 96 |
| RF | 84 | 0 | 84 |
