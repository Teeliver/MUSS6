# W0048 — Fire01 fixture và kiểm tra 7 class

## Phạm vi thực tế

Đã ghép một effect thử Fire01 và kiểm tra pipeline render/animation trên đủ 7 class. **Chưa hoàn thành toàn bộ skill/hiệu ứng riêng của 7 class.** Cast thử trên một class không cấp quyền dùng Fire Ball cho class đó. Class eligibility, danh sách skill, packet, mana, cooldown, hit và damage chưa tích hợp.

## Đã triển khai

- WASM ModelAnimation cache BMD effect, sample clip0 theo clock6,25key/s giống ActorVisual native mặc định. BodyAnimation và effect giữ asset/GPU resources qua frame; không parse lại BMD trong render loop.
- Fire01 dùng hai mesh/texture gốc. Theo native, mesh1 additive và không ghi depth; mesh0 giữ opaque/discard policy. Renderer khôi phục blend/depth state sau khi vẽ. Không dùng ngưỡng RGB tự xóa nền.
- Projectile thử theo world_view.mm skill4: bắt đầu cao1,2world unit, đích cao1unit, tốc độ12,5unit/s, duration=max(0,04,distance/12,5). Đích giả đặt cách3unit theo trụcX; không có monster/target thật. Scale Fire0,01 và actor rig0,009, axis/yaw quy đổi về cùng tọa độ thân. Test analytic kiểm các điểm đầu/giữa, scale, normal và lifetime.
- Query `effect=fire` dùng panel nhân vật riêng, camera fit envelope toàn đường thử. `effect-time=0.12` dừng effect để xem material/hình tại thời điểm cụ thể; thân vẫn animation. Đây là chế độ kiểm tra, không phải skill tự phát/gameplay. Effect thử lặp2giây; khoảng nghỉ không phải cooldown nguồn.
- Chỉ thêm đúng Fire01.bmd và fire01/fire02.OZJ vào staging; tổng175asset. Path nguyên Data và SHA256 giữ nguyên. Skill folder chỉ chấp nhận một filename an toàn, không traversal/nested paths; không xuất toàn bộ Data.
- `class_render_audit.mjs` kiểm tier0 của DW, DK, Elf, MG, DL, Summoner, RF: body5part, idle/walk/run/attack và cast146/147, geometry Fire01, hữu hạn tọa độ. Cả7 qua. Không kiểm full equipment/tiers/skill sets.
- Bài đo mỗi class600mẫu sau30warmup gồm sampler/copy/Fire transform, không gồm WebGL/GPU. JSON lưu baseline W0048; không lấy DW đại diện hiệu năng tất cả class.

## Kết quả CPU theo class

| Class | Geometry/clip | CPU p95 (ms) | CPU max (ms) | Cảnh báo |
|---|---|---:|---:|---|
| DW | Qua | 0,315 | 0,635 | Skill set chưa xác minh |
| DK | Qua | 0,289 | 0,941 | Skill set chưa xác minh |
| Elf | Qua | 0,331 | 0,932 | Fire chỉ là fixture render |
| MG | Qua | 0,317 | 1,060 | Skill set chưa xác minh |
| DL | Qua | 1,227 | 16,175 | Spike vượt ngân sách120FPS, cần profile nguyên nhân |
| Summoner | Qua | 0,611 | 1,114 | Nhiều đỉnh hơn nhóm khác; skill set chưa xác minh |
| RF | Qua | 0,394 | 0,820 | Native male idle fallback chưa xác minh |

Số đo là một lần chạy CPU-only, không benchmark FPS/GPU và không bảo đảm performance trên mọi máy. Max DL được giữ nguyên, không loại bỏ để làm đẹp số liệu.

## Chưa hoàn thiện

- Fire chỉ là một prefab/prototype chuyển động theo đích giả; chưa nối skill selector/server, targeting/range, projectile hit, skill sound hoặc damage.
- Chưa tất cả skill của7class; chưa effect riêng DK/Elf/MG/DL/Summoner/RF hoặc cast policy riêng theo skill. Render generic clip không chứng minh skill hợp lệ.
- Chưa Windows visual parity, chưa full map/depth/occlusion/lighting context. Camera envelope là technical preview. Chưa GPU skinning/output pooling.
- Wing nền đen/hide, skin dưới mũ, RF idle và GUI/network vẫn còn mở. Không đánh dấu các lỗi này đã sửa bởi class audit.
- Không bật MU server/DB, không sửa Data/native production/config. M02/M03 vẫn PARTIAL.

## Validation và bằng chứng

Test/build/browser cuối ghi STATUS.vi.md. Baseline `.local-baselines/2026-10-25-w0048/` giữ JSON từng class và screenshot kiểm từng class. Bước sau cần lập matrix skill/class từ Skill.bmd và quy tắc server, rồi ghép từng effect có nguồn đủ, không mặc định chung Fire cho7class.

## Kiểm tra hình và điều kiện class từ server

Đã mở browser riêng từng class0..6 với Fire01 dừng ở0,12s; có đủ7ảnh `class-0.png` … `class-6.png`. Không thấy crash hay geometry vỡ trong các ảnh này. Đây không chứng nhận tất cả animation/trang bị/skill hoặc parity Windows.

| Class | Browser FPS quan sát | CPU frame p95 (ms) | rAF interval p95 (ms) |
|---|---:|---:|---:|
| DW | 60 | 1,10 | 17,60 |
| DK | 60 | 1,80 | 17,00 |
| Elf | 60 | 1,70 | 16,80 |
| MG | 60 | 2,20 | 17,30 |
| DL | 60 | 2,70 | 17,10 |
| Summoner | 60 | 2,50 | 17,10 |
| RF | 60 | 2,20 | 17,50 |

Cửa sổ mẫu khác nhau, không dùng bảng này để xếp hạng class. Các p95 vẫn vượt ngân sách16,67ms ở nhiều lần đo, chưa bảo đảm60FPS ổn định/120FPS. Fire01 có hình vệt lửa lớn theo scale/hướng adapter native; cần đối chiếu visual Windows để xác nhận kích thước/hướng, chưa tự sửa đẹp bằng hệ số đoán.

Đã thêm audit server `skill_class_audit.py`, đọc22cột đúng thứ tự SkillManager::Load và áp quy tắc CheckSkillRequireClass: RequireClass khác0 và ChangeUp+1>=RequireClass. SkillList.txt hiện có386mục. Không bật server, không sửa config.

| Class | Qua bộ lọc class ở tier0 / tier1 / tier2 | Fire Ball qua class filter tier0 |
|---|---|---|
| DW | 26 / 32 / 100 | Có |
| DK | 20 / 24 / 93 | Không |
| Elf | 23 / 27 / 97 | Không |
| MG | 39 / 39 / 110 | Có |
| DL | 29 / 29 / 100 | Không |
| Summoner | 33 / 34 / 97 | Có |
| RF | 22 / 22 / 84 | Không |

Các số này **không phải số skill đã ghép**, cũng không xác nhận đủ level/energy/mana/leadership/guild/kill count/equipment. Render Fire01 trên DK/Elf/DL/RF chỉ là diagnostic pipeline, không gán skill vào nhân vật. Hiệu ứng riêng đầy đủ của7class vẫn TODO. Rule/parser Python test qua; JSON giữ source path/hash và danh sách từng skill theo class/tier.
