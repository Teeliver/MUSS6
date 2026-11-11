# W0115 — Effect cho toàn bộ skill chủ động 7 class (2026-10-31)

## Kết quả

- **114/114 skill chủ động** có thể cast bởi player đã có effect tường minh khi cast.
- **3 exact**: Fire Ball (4), Evil Spirit (9), Energy Ball (17) — giữ nguyên hình học/prototype đã kiểm chứng.
- **60 source**: tìm thấy `CreateEffect` / `CreateJoint` / `CreateParticle` thật trong dispatch `switch (c->Skill)` của `Main5.2/source/ZzzCharacter.cpp`.
- **51 derived**: cùng họ hình ảnh với skill gốc của class, **chưa có token nguồn riêng** trong C++.
- **0 missing** trong phạm vi skill chủ động.
- 315/315 client test pass; 13/13 texture của catalog giải mã được; build debug 147 file.

Phạm vi "chủ động" được định nghĩa bằng dữ liệu, không bằng tên skill:

```text
id < 300, class != 0, (mana > 0 hoặc AG > 0)
hoặc
id >= 300 và có case trong CSkillManager::RunningSkill (GameServer/SkillManager.cpp)
```

Hai skill dùng AG thay mana là 62 Earthquake và 65 Electric Spark. Mười master skill chủ động có handler server là 344, 425, 426, 427, 430, 432, 461, 495, 521, 565; quét toàn bộ `case SKILL_*` trong `GameServer` cũng chỉ tìm thấy đúng 10 id ≥ 300 này. Các master-skill passive như 511/515/517 không có case trong `RunningSkill` nên không được gán effect. 79 Explosion cũng bị loại vì không có handler server; 50/200/201/202/239 có `classes` toàn 0 — player không cast được.

### 12 skill bổ sung so với bộ 102 cũ

| ID | Skill | Class | Căn cứ chủ động | Trạng thái |
|---:|---|---|---|---|
| 62 | Earthquake | DL | AG 50, delay 10s | `derived` |
| 65 | Electric Spark | DL | AG 100, delay 5s | `derived` |
| 344 | Blood Storm | DK, MG | `CSkillManager::RunningSkill` | `derived` |
| 425 | Cure | Elf | `RunningSkill` + `CreateEffect` | `source` |
| 426 | Party Healing | Elf | `RunningSkill` + `CreateEffect` | `source` |
| 427 | Poison Arrow | Elf | `RunningSkill` (`CreateArrows`) | `derived` |
| 430 | Bless | Elf | `RunningSkill` | `derived` |
| 432 | Summon Satyros | Elf | `RunningSkill` + `CreateEffect` | `source` |
| 461 | Blind | Summoner | `RunningSkill` | `derived` |
| 495 | Earth Prison | DW, MG | `RunningSkill` | `derived` |
| 521 | Iron Defense | DL | `RunningSkill` | `derived` |
| 565 | Blood Howling | RF | `RunningSkill` | `derived` |

## Phân loại trung thực

| Trạng thái | Nghĩa | Số lượng |
|---|---|---:|
| `exact` | Prototype gốc đã có hình học/asset kiểm chứng | 3 |
| `source` | Có token effect/joint/particle thật trong source C++ | 60 |
| `derived` | Cùng họ hình ảnh với skill gốc của class, chưa có token nguồn riêng | 51 |
| `missing` | Không có effect | 0 |

`derived` **không phải** bằng chứng parity. Nó chỉ nói rằng skill được gán một họ hình ảnh hợp lý theo class/nhóm kỹ năng; timing, vật liệu, scale và âm thanh chưa được đối chiếu từng skill.

## Theo class

Một skill đa class được đếm ở mọi class sở hữu, nên tổng theo cột lớn hơn 114.

| Class | Tổng skill chủ động | Exact | Source | Derived |
|---|---:|---:|---:|---:|
| DW | 33 | 3 | 21 | 9 |
| DK | 25 | 0 | 14 | 11 |
| Elf | 32 | 0 | 14 | 18 |
| MG | 41 | 3 | 25 | 13 |
| DL | 29 | 0 | 14 | 15 |
| Summoner | 35 | 2 | 18 | 15 |
| RF | 23 | 0 | 7 | 16 |

## Kiến trúc

| File | Vai trò |
|---|---|
| `client/src/world/skill-effect-catalog.js` | 13 họ hình ảnh + 114 spec (`family`, `mode`, `source`, `exact`) và `isActiveSkill`. Không suy effect từ tên skill. |
| `client/src/world/skill-effect-runtime.js` | Dựng hình học thuần dữ liệu cho 12 mode; không DOM/WebGL nên test được bằng Node. |
| `client/src/world/skill-effects.js` | Nối catalog/runtime vào scene; giữ legacy 4/9/17; giới hạn 12 effect đồng thời; dispose đúng. |
| `client/src/preview/skill-support.js` | Trả `status` (`prototype`/`source`/`derived`/`missing`), `family`, không giả lập fixture. |
| `tools/audit_skill_effects.mjs` | Audit 13 texture thật trong `Data/Effect` (đọc-only). |
| `tests/skill_effects_full.test.mjs` | Khóa coverage 114, token/ID, texture, topology và số mesh theo thời gian. |
| `tests/skill_support.test.mjs` | Khóa semantics prototype/source/derived/missing. |

### 13 họ hình ảnh

`fire`, `ice`, `lightning`, `energy`, `poison`, `holy`, `dark`, `wind`, `water`, `arc`, `magic`, `spark`, `force`.

Mỗi họ trỏ tới một texture gốc trong `Data/Effect`; audit xác nhận cả 13 texture mở được, kích thước hợp lệ.

### 12 mode hình học

| Mode | Hình học | Số skill |
|---|---|---:|
| `projectile` | Đầu đạn + vệt + impact tỏa | 13 |
| `meteor` | Rơi xiên + impact | 2 |
| `column` | Cột dâng theo phase | 3 |
| `ring` | Vòng đất lan + hạt | 12 |
| `burst` | Nổ tỏa đa hướng | 14 |
| `bolt` | Tia sét từ trên xuống | 4 |
| `arc` | Vệt chém theo hướng | 16 |
| `aura` | Vòng aura quanh mục tiêu | 26 |
| `beam` | Chùm nối from→to | 7 |
| `circle` | Vòng tròn ma thuật | 11 |
| `chain` | Chuỗi nối nhiều điểm | 1 |
| `shield` | 3 lớp khiên đồng tâm | 5 |

Mọi mode giữ **số mesh cố định theo thời gian** vì `createModelPreview.update()` ném lỗi nếu topology đổi. Test kiểm tra điều này ở age 0 / giữa / cuối cho từng mode.

### Toạ độ

Runtime dùng không gian source `(tileX, tileY, height)`. Shader world đổi sang world bằng `(x, z, -y)`, nên "up" của thế giới là trục `z`. `cameraSource()` tính `[5.82+dx, dy-4.82, height+8.5]` để khớp `worldCamera()`.

## Bảng 114 skill

| ID | Skill | Class | Trạng thái | Họ | Mode | Token nguồn |
|---:|---|---|---|---|---|---|
| 1 | Poison | DW, MG | source | poison | projectile | AT_SKILL_POISON |
| 2 | Meteorite | DW, MG, Summoner | source | fire | meteor | AT_SKILL_METEO |
| 3 | Lightning | DW, MG | derived | lightning | bolt | — |
| 4 | Fire Ball | DW, MG, Summoner | exact | fire | projectile | AT_SKILL_FIREBALL |
| 5 | Flame | DW, MG | source | fire | column | AT_SKILL_FLAME |
| 6 | Teleport | DW | derived | magic | circle | — |
| 7 | Ice | DW, MG, Summoner | source | ice | projectile | AT_SKILL_SLOW |
| 8 | Twister | DW, MG | source | wind | ring | AT_SKILL_STORM |
| 9 | Evil Spirit | DW, MG, Summoner | exact | dark | aura | Skill_Evil_Spirit |
| 10 | Hell Fire | DW, MG | source | fire | column | AT_SKILL_HELL |
| 11 | Power Wave | DW, MG, Summoner | source | force | beam | AT_SKILL_POWERWAVE |
| 12 | Aqua Beam | DW, MG | source | water | beam | AT_SKILL_FLASH |
| 13 | Cometfall | DW, MG | source | fire | meteor | Skill_Cometfall |
| 14 | Inferno | DW, MG | source | fire | column | AT_SKILL_INFERNO |
| 15 | Teleport Ally | DW | derived | magic | circle | — |
| 16 | Soul Barrier | DW | source | holy | shield | AT_SKILL_WIZARDDEFENSE |
| 17 | Energy Ball | DW, MG | exact | energy | projectile | AT_SKILL_ENERGYBALL |
| 18 | Defense | DK, MG, DL, Summoner | derived | holy | aura | — |
| 19 | Falling Slash | DK, MG, DL, Summoner, RF | derived | arc | arc | — |
| 20 | Lunge | DK, MG, DL, Summoner | derived | arc | arc | — |
| 21 | Uppercut | DK, MG, DL, Summoner | derived | arc | arc | — |
| 22 | Cyclone | DK, MG, DL, Summoner | derived | wind | ring | — |
| 23 | Slash | DK, MG, DL, Summoner | derived | arc | arc | — |
| 24 | Triple Shot | Elf | derived | spark | projectile | — |
| 26 | Heal | Elf | source | holy | aura | AT_SKILL_HEALING |
| 27 | Greater Defense | Elf | source | holy | shield | AT_SKILL_DEFENSE |
| 28 | Greater Damage | Elf | source | holy | aura | AT_SKILL_ATTACK |
| 30 | Summon Goblin | Elf | source | magic | circle | AT_SKILL_SUMMON |
| 31 | Summon Stone Golem | Elf | derived | magic | circle | — |
| 32 | Summon Assassin | Elf | derived | magic | circle | — |
| 33 | Summon Elite Yeti | Elf | derived | magic | circle | — |
| 34 | Summon Dark Knight | Elf | derived | magic | circle | — |
| 35 | Summon Bali | Elf | derived | magic | circle | — |
| 36 | Summon Soldier | Elf | derived | magic | circle | — |
| 38 | Decay | DW | source | poison | projectile | AT_SKILL_BLAST_POISON |
| 39 | Ice Storm | DW, MG | source | ice | ring | AT_SKILL_BLAST_FREEZE |
| 40 | Nova | DW | source | lightning | ring | AT_SKILL_BLAST_HELL |
| 41 | Twisting Slash | DK, MG | source | wind | ring | Skill_Twisting_Slash |
| 42 | Rageful Blow | DK | source | arc | burst | AT_SKILL_FURY_STRIKE |
| 43 | Death Stab | DK | source | dark | arc | Skill_Death_Stab |
| 44 | Crescent Moon Slash | DK | source | arc | arc | AT_SKILL_RUSH |
| 45 | Lance | DW, Summoner | source | force | beam | AT_SKILL_JAVELIN |
| 46 | Starfall | Elf | derived | holy | burst | — |
| 47 | Impale | DK, MG | source | force | projectile | Skill_Impale |
| 48 | Sweel Life | DK | source | holy | aura | AT_SKILL_VITALITY |
| 49 | Fire Breath | DK | source | fire | beam | AT_SKILL_RIDER |
| 51 | Ice Arrow | Elf | derived | ice | projectile | — |
| 52 | Penetration | Elf | source | spark | beam | AT_SKILL_PIERCING |
| 55 | Fire Slash | MG | source | fire | arc | Skill_Fire_Slash |
| 56 | Power Slash | MG | source | ice | arc | AT_SKILL_ICE_BLADE |
| 57 | Spiral Slash | MG | source | wind | arc | AT_SKILL_ONEFLASH |
| 60 | Force | DL | source | force | projectile | AT_SKILL_STRONG_PIER |
| 61 | Fire Burst | DL | source | fire | burst | Skill_Fire_Burst |
| 62 | Earthquake | DL | derived | force | ring | — |
| 63 | Summon | DL | source | magic | circle | AT_SKILL_PARTY_TELEPORT |
| 64 | Increase Critical Damage | DL | source | holy | aura | AT_SKILL_ADD_CRITICAL |
| 65 | Electric Spark | DL | derived | lightning | burst | — |
| 66 | Force Wave | DL | derived | force | ring | — |
| 67 | Stun | DW, DK, Elf, MG, DL, Summoner, RF | source | lightning | burst | AT_SKILL_STUN |
| 68 | Cancel Stun | DW, DK, Elf, MG, DL, Summoner, RF | source | holy | aura | AT_SKILL_REMOVAL_STUN |
| 69 | Swell Mana | DW, DK, Elf, MG, DL, Summoner, RF | source | holy | aura | AT_SKILL_MANA |
| 70 | Invisibility | DW, DK, Elf, MG, DL, Summoner, RF | source | magic | aura | AT_SKILL_INVISIBLE |
| 71 | Cancel Invisibility | DW, DK, Elf, MG, DL, Summoner, RF | source | holy | aura | AT_SKILL_REMOVAL_INVISIBLE |
| 72 | Abolish Magic | DL | source | holy | aura | AT_SKILL_REMOVAL_BUFF |
| 73 | Mana Rays | MG | source | energy | beam | AT_SKILL_DEATH_CANNON |
| 74 | Fire Blast | DL | source | fire | burst | Skill_Fire_Blast |
| 75 | Brand | DL | source | dark | burst | AT_SKILL_BRAND_OF_SKILL |
| 76 | Plasma Storm | DW, DK, Elf, MG, DL, Summoner, RF | source | lightning | bolt | AT_SKILL_PLASMA_STORM_FENRIR |
| 77 | Infinity Arrow | Elf | derived | spark | projectile | — |
| 78 | Fire Scream | DL | source | fire | burst | Skill_Fire_Scream |
| 210 | Spell of Protection | DW, DK, Elf, MG, DL, Summoner, RF | derived | holy | aura | — |
| 211 | Spell of Restriction | DW, DK, Elf, MG, DL, Summoner, RF | derived | dark | aura | — |
| 212 | Spell of Pursuit | DW, DK, Elf, MG, DL, Summoner, RF | derived | magic | aura | — |
| 213 | Shield-Burn | DW, DK, Elf, MG, DL, Summoner, RF | derived | fire | burst | — |
| 214 | Drain Life | Summoner | source | dark | beam | AT_SKILL_ALICE_DRAINLIFE |
| 215 | Chain Lightning | Summoner | derived | lightning | chain | — |
| 216 | Electric Surge | Summoner | source | lightning | bolt | AT_SKILL_ALICE_LIGHTNINGORB |
| 217 | Damage Reflection | Summoner | source | holy | shield | AT_SKILL_ALICE_THORNS |
| 218 | Berserker | Summoner | source | fire | aura | AT_SKILL_ALICE_BERSERKER |
| 219 | Sleep | Summoner | source | dark | aura | AT_SKILL_ALICE_SLEEP |
| 221 | Weakness | Summoner | source | poison | aura | AT_SKILL_ALICE_WEAKNESS |
| 222 | Innovation | Summoner | source | dark | aura | AT_SKILL_ALICE_ENERVATION |
| 223 | Explosion | Summoner | derived | fire | burst | — |
| 224 | Requiem | Summoner | derived | dark | ring | — |
| 225 | Pollution | Summoner | derived | poison | ring | — |
| 230 | Lightning Shock | Summoner | source | lightning | bolt | AT_SKILL_LIGHTNING_SHOCK |
| 232 | Strike of Destruction | DK | source | dark | burst | AT_SKILL_BLOW_OF_DESTRUCTION |
| 233 | Expansion of Wizardry | DW | derived | magic | aura | — |
| 234 | Recovery | Elf | derived | holy | aura | — |
| 235 | Multi-Shot | Elf | derived | spark | projectile | — |
| 236 | Flame Strike | MG | source | fire | arc | AT_SKILL_FLAME_STRIKE |
| 237 | Gigantic Storm | MG | source | wind | ring | AT_SKILL_GIGANTIC_STORM |
| 238 | Chaotic Diseier | DL | derived | dark | burst | — |
| 260 | Killing Blow | RF | derived | arc | arc | — |
| 261 | Beast Uppercut | RF | derived | arc | arc | — |
| 262 | Chain Drive | RF | derived | arc | arc | — |
| 263 | Dark Side | RF | derived | dark | arc | — |
| 264 | Dragon Roar | RF | derived | force | burst | — |
| 265 | Dragon Slasher | RF | derived | arc | arc | — |
| 266 | Ignore Defense | RF | derived | arc | arc | — |
| 267 | Increase Health | RF | derived | holy | aura | — |
| 268 | Increase Block | RF | derived | holy | shield | — |
| 269 | Charge | RF | source | wind | burst | AT_SKILL_OCCUPY |
| 270 | Phoenix Shot | RF | derived | fire | projectile | — |
| 344 | Blood Storm | DK, MG | derived | dark | ring | — |
| 425 | Cure | Elf | source | holy | aura | Skill_Cure |
| 426 | Party Healing | Elf | source | holy | aura | Skill_Party_Healing |
| 427 | Poison Arrow | Elf | derived | poison | projectile | — |
| 430 | Bless | Elf | derived | holy | aura | — |
| 432 | Summon Satyros | Elf | source | magic | circle | Skill_Summon_Satyros |
| 461 | Blind | Summoner | derived | dark | aura | — |
| 495 | Earth Prison | DW, MG | derived | force | ring | — |
| 521 | Iron Defense | DL | derived | holy | shield | — |
| 565 | Blood Howling | RF | derived | dark | aura | — |

## Bằng chứng chạy

```text
npm test
ℹ tests 315
ℹ pass 315
ℹ fail 0

node tools/audit_skill_effects.mjs /Users/angelale/Downloads/MuClientEX603/Data
→ 13 families, 114 skills, 13 texture rows decoded

npm run build:debug
→ Built 147 allowlisted files into web/dist/debug
→ dist/debug/src/world/skill-effect-catalog.js và skill-effect-runtime.js khớp source

npm run build:release
→ Built 38 allowlisted files into web/dist/release
```

Kiểm tra thêm: 63 token `source` (3 exact + 60 source) đều xuất hiện nguyên văn trong `Main5.2/source/ZzzCharacter.cpp` hoặc `SkillManager.h`. Riêng 425/426/432 nằm trong nhánh có `CreateEffect` thật; 427 dùng `CreateArrows` nên vẫn xếp `derived` theo tiêu chí hiện tại.

## Giới hạn — không báo hoàn tất parity

- 51 `derived` chưa có token nguồn riêng; đây là họ hình ảnh gần nhất, không phải trích xuất 1:1.
- Chưa port đầy đủ material, animation curve, scale, âm thanh, hit timing của từng skill như client Windows.
- `preview/menu.js` vẫn chỉ phát 3 prototype gốc (Fire Ball/Evil Spirit/Energy Ball); các skill khác hiển thị nhãn `source`/`derived` nhưng không dựng effect trong màn preview. Effect đầy đủ nằm ở world scene khi cast.
- Mỗi effect generic tạo một `createModelPreview` riêng (shader + texture riêng), giới hạn 12 effect đồng thời. Chưa đo FPS khi 12 effect cùng lúc; nếu cần tối ưu, bước tiếp theo là pool renderer dùng chung.
- Chưa smoke test browser cho từng skill; mới xác nhận bằng unit test hình học, audit texture và build.
- Damage/MP/cooldown/knockback vẫn do server quyết định; W0115 chỉ là lớp hình ảnh.

## Chạy lại

```bash
cd SOURCE/Source/Emulator/web
npm test
node tools/audit_skill_effects.mjs /Users/angelale/Downloads/MuClientEX603/Data
npm run build:debug
```

Scene có skill effect: `http://127.0.0.1:8080/?scene=servers` (online) hoặc `?scene=terrain-check&map=0` (offline, không có cast).

## Cập nhật 2026-11-04 — port effect source

- Coverage hiện tại: **3 exact / 106 source / 0 derived / 5 no-visual / 0 missing**.
- Đã port thêm BMD/texture source cho Teleport Ally, Earthquake, Force Wave, RF group, Summoner group, Infinity Arrow, Multi-Shot, Chaotic Diseier.
- 5 no-visual: `212, 344, 495, 521, 565` — source client xác nhận không có `Create*` cast effect (chỉ action, enum server hoặc buff icon).
- Audit: `docs/SKILL-PARITY-AUDIT-20261104.vi.md`; test **429/429**, build **166 file**.
