# W0054 — Evil Spirit preview (2026-10-25)

## Đã xong

- Port thuật toán native/client/evil_spirit.hpp sang JavaScript: 4 hướng, tick25Hz, bước0,7world unit, tail6điểm, homing10độ/tick, xorshift32, tối đa50tick. Fixture seed1, nhân vật đứng ở gốc, chưa target/hit thật.
- So trực tiếp executable C++ dùng header native với JS: 3seed ×50tick ×4head ×7scalar =4200giá trị; sai số tối đa1,1368683772161603e-13. Đã sửa signed zero và quy tắc remainder ties-to-even. Công cụ: web/tools/spirit_reference.cpp và spirit_reference_check.mjs; kết quả baseline W0054.
- Ghép 4head Laser01.bmd/dragon02.OZJ và8ribbon JointSpirit01.OZJ theo adapter world_view.mm, scalehead0,013, tailwidth0,4/0,1; tái sử dụng output typed arrays, topology cố định, đoạn đuôi chưa có được collapse.
- Renderer thêm Multiply, không depthwrite, luminosity=(49-frame)*0,1 giảm dần, fade cuối đời theo adapter; khôi phục state sau vẽ. Nền xám riêng cho bài thử Multiply để nhìn rõ, không phải nền game gốc. Freeze camera fit frame hiện tại; animation body vẫn chạy.
- Menu skill9 mở spirit effect riêng;3prototype choDW (4/9/17). Staging179asset, không sửa Data nguồn. Release không có effect asset/debug module.
- 100/100 npm tests và buildDebug/Release qua. Test deterministic/tail/lifetime/topology/buffer, GPUblend/depth/brightness/opacity và audit menu. Browser mở DWtier2 skill9: có head/tail, không crash trong lượt thử; ảnh baseline đã lưu.

## Chưa hoàn thiện

- Chưa xác nhận orientation/scale/UV/độ tối và hình đầu/đuôi giống Windows. Ảnh hiện thấy đầu lớn, dải tối có dạng phẳng; cần đối chiếu reference trước khi gọi là chuẩn. Numeric simulation khớp native không chứng minh visual parity.
- Cast vẫn generic146/147, chưa timing riêng skill9, sound, gameplay authority/hit/damage/mana/cooldown/network. Không cấp skill thực.
- Prototype không phải full7class; menu master skill/skill khác vẫn missing. Nguồn adapter native đủ cho9 nhưng chưa có đầy đủ các nhánh Windows.
- Browser ảnh cuối quan sát60FPS, CPUframep951,50ms, intervalp9518,50ms; chưa bảo đảm60FPSổnđịnh/120FPS. GPUskinning/WASMsamplerpooling/DLspike/RFidle/skinwing cònTODO.
- Không bật MUserver/DB, không sửa native production/config. M02/M03 vẫnPARTIAL.

## Chạy so nguồn

`clang++ -std=c++17 web/tools/spirit_reference.cpp -o /tmp/mu-spirit-reference`

Chạy executable lưu stdout vào file rồi truyền file đó cho `node web/tools/spirit_reference_check.mjs FILE`.
