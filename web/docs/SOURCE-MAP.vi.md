# Nguồn chuẩn và khả năng tái sử dụng

| Phần | Nguồn hiện có | Phân loại | Cách dùng cho web / chưa xác nhận |
|---|---|---|---|
| Packet và authoritative gameplay | Emulator/GameServer/Protocol.*, SkillManager.*, ItemManager.*, Monster.*, ObjectManager.* | Source EX603 được cung cấp | Chuẩn packet và quyết định server; không chứng minh visual client |
| Cipher và packet extension | Main_EX603/Main/Protocol.*, PacketManager.*, Offset.h | Extension EX603 được cung cấp | Main.vcxproj khai báo DynamicLibrary; không có engine tại các địa chỉ hook |
| Transport/state native | native/client/client.cpp, viewport.cpp, inventory.cpp | Implementation mới đã có contract native | Tách POSIX Socket, CommonCrypto, gethostuuid và callbacks trước WASM; chưa compile web |
| Identity/ClientVerify | client.cpp constructor/session greeting, GameServer/HackCheck.* và Main_EX603 | Protocol gốc + adapter Mac mới | Browser không có gethostuuid. Cần thiết kế identity/session cho web và kiểm tra chính sách server; không tự tắt kiểm tra để login được |
| Item stats và placement | native/client/item_stats.cpp, item_placement.cpp, ItemAttribute parser; GameServer/Item.* | Native dựa trên source/data | Có thể dùng chung phần C++; Harmony/socket/custom còn thiếu, xem ITEM-UI.vi.md |
| Terrain/model/parser | native/client/assets.cpp, assets.hpp, item_models.cpp, entity_assets.mm | Parser/adapter mới | BMD vừa có bảng vừa có model. Cần audit magic/version và dependency, không phân loại chỉ bằng extension |
| Movement | motion.hpp, pathfinding.hpp; server Move/User logic | Native + reference và server constraints | Giữ test về tick/độ dài quãng đường; chưa xác nhận toàn bộ curve/animation trùng engine Windows |
| Render/HUD/effects | world_view.mm, entity_view.mm, item_upgrade.mm, game_hud.mm | SceneKit/Cocoa implementation mới | Viết renderer web; shader/material/màu/alpha/sort có bằng chứng riêng, không port nguyên .mm |
| Scene/action/material tham khảo | oXaa/muonline-client-sources; sven-n/MuMain; ghi nguồn trong audit native | Reference khác build/version | Chỉ dùng khi gắn nhãn reference, không gọi là source đầy đủ của EX603 hiện tại |
| Texture/model/terrain/audio/text | MuClientEX603/Data | Resource bộ client hiện có | Có inventory byte-level; chưa xác minh decode/dependency toàn bộ |
| Font | source_font.mm đọc Text[0]/Config.ini; fallback đăng ký Gulim từ ứng dụng Office đã cài | Data chỉ tên font; font ngoài Data | Không có TTF/TTC/WOFF trong inventory Data. Chưa có asset font web hợp lệ; không tự đổi font hoặc copy font Office vào bundle |
| Audio | Data/Sound; sound_bank.hpp | Resource thật + mapping native chưa hoàn chỉnh | 911 WAV, không có MP3 trong Data đã quét. Mapping/timing native không phải chuẩn gốc đã xác nhận |
| Engine Windows EX603 đầy đủ | Chưa tìm thấy trong nguồn đã audit | BLOCKED parity | Thiếu implementation được Offset.h gọi; không có bằng chứng frame-by-frame với executable Windows gốc |

Phạm vi đã kiểm tra dựa trên file thực tại ngày 2026-10-23, kế thừa `native/client/SOURCE-AUDIT.vi.md` nhưng không coi tài liệu cũ là trạng thái tự động đúng mãi. Bằng chứng trực tiếp: Main.vcxproj có ConfigurationType=DynamicLibrary; client.cpp còn include CommonCrypto/HidManager/uuid và Socket; CMake viewer/item UI link Cocoa/SceneKit/Metal/CoreText.

Quy tắc ticket: ghi đường dẫn + hàm hoặc schema Data + baseline hash. Khi thiếu source, ghi rõ unknown/reference; không đóng ticket parity chỉ vì implementation mới chạy được.


## Bổ sung W0059 — 2026-10-26

Đã nhận `file thieu` gồm39file C++/header (3.648.890byte), gồm movement, pathfinding, character và các hệ Effect/Joint/Particle/Sprite. Phân loại **source bổ sung được cung cấp, version chưa xác nhận**, không còn coi mọi nhánh effect là chưa có source. Chưa có bằng chứng cùng build engineEX603. [Inventory có hash](supplemental-source-audit.json) và [đối chiếu movement/effect](MOVEMENT-SOURCE-AUDIT.vi.md). Đây là nguồn tham khảo để đối chiếu từng nhánh, chưa được ghép vào web và không tự đóng các gap visual/skill.


## W0062 — Height loader và menu map

Đã đối chiếu OpenTerrainHeight/OpenTerrainHeightNew trong source bổ sung; parser hỗ trợ raw8/extended24, đọc được47World. Catalog38map có texture theo adapter,9map thiếu mapping; chưa object/scene parity. Move.txt/Gate.txt và GameServer/Move.h/.cpp là nguồn cho39điểm chuyển online, ABI có test compile từ struct gốc. UIweb/M mới chưa được xác nhận giống GUIgốc; còn thiếu MoveCommandData/NewUIMove/MapManager::LoadWorld. Chi tiết [MAP-MENU](MAP-MENU.vi.md).


## W0064 — nguồn NPC và texture

Đã đọc riêng AccessModel/OpenTexture, renderer ZzzObject và nhánh Player helper. Model folder không đồng nghĩa texture folder (Quarrel dùngMonster). Raul có shared-asset adapter được ghi/pin hash riêng, không coi là source-proven engine lookup. Helper371/414 dùng skeletonPlayer/action1 và giáp+9 từ nguồn. [Chi tiết và giới hạn](WORLD-RESOURCES.vi.md).
