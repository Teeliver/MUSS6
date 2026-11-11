# W0081 — Sound và Music theo source gốc

## Bộ dữ liệu và rà soát

- `MuClientEX603/Data/Sound`: 911 WAV, 217.295.345 byte. Thư mục người dùng bổ sung `SOURCE-2.0.7/Music`: 47 MP3/OGG, 125.012.404 byte.
- `audit_audio.py` quét .cpp/.h của Main5.2/source: 946 lệnh LoadWaveFile, 1.529 lệnh PlayBuffer/StopBuffer/PlayMp3/StopMp3. Đọc các hằng SOUND/MUSIC gồm #define và enum, SetMonsterSound theo switch Type, bảng g_lpszMp3. Số liệu là tham chiếu source, không phải số trigger đã port.
- Bảng runtime `src/audio/catalog.js`: đường dẫn đúng hoa/thường, size/SHA256, 866 soundID có binding, 72 model monster có bảng sound; đủ47 mục trong bảng nhạc source. Full call-site, line number và mục chưa tìm được nằm trong `AUDIO-SOURCE-AUDIT.json`.
- Có17 lệnh nạp với đường dẫn không tìm được đúng file, gồm lỗi tên/thiếu extension và một đường dẫn không giải mã rõ từ encoding source. Có4 soundID nạp các file khác nhau tùy nhánh (160/326/442/698). Engine không tự chọn một file bất kỳ; trường hợp NPC dùng path rõ theo model. Không sửa/đổi tên Data gốc.

Tái sinh bảng:

```sh
python3 web/tools/audit_audio.py --source <Main5.2/source> --data <MuClientEX603/Data> --music <SOURCE-2.0.7/Music>
```

## Luồng đã nối

- Nút UI click, mở cửa sổ; ACK chuyển đồ, nhặt đồ, sửa đồ, mua đồ. Không phát nhặt thành công khi server từ chối, không phát nhặt Zen như vật phẩm thường. Item/Jewel/Money phân biệt theo CreateItem/ReceiveGetItem. Không thêm iButtonClick khi chỉ chọn item.
- Đồ rơi chỉ phát khi có CreateFlag và key mới, không phát lại vì snapshot tồn tại. Chỉ áp dụng vị trí/người nghe khi LoadWaveFile bật Enable3DSound; file2D không tự pan/giảm âm theo khoảng cách.
- Bước chân Hero theo loại terrain, map, safezone tại PlayWalkSound. WASM trả frame/mode thật từ MotionAnimation; hai chân kích ở1.5/4.5, một lần mỗi vòng, không còn timer360ms. Native contract kiểm tra frame ở60/120Hz và phase khi walk/run đổi nhau.
- Đánh thường chọn tiếng theo vũ khí source đã biết; barehand không tự phát sword. Tiếng bị đánh/chết của Hero theo lớp, tiếng quái theo bảng SetMonsterSound (attack/shock/death). Heartbeat khi0<HP/MaxHP<20% theo NewUIMainFrameWindow. Không nhân tiếng chân của các remote player vì source PlayWalkSound chỉ xét Hero.
- Chaos Goblin/Elf Lala theo xác suất source ở nhịp25Hz. Hanzo dùng animation frame5..10 (chỉ bắt đầu một lần ở vùng frame này), file nBlackSmith riêng, không nhầm nHarp cùng SOUND_NPC. Model NPC chưa dựng không bị phát tiếng giả.
- Môi trường map0/1/2/3/4/7/8/10/58/79/80/81 với điều kiện tile trong ZzzScene. Lorencia tile4 dừng gió; Devias chỉ tuyết ngoài các tile lát; Karutan2 tile12 đổi nền môi trường.
- Nhạc login và map thường theo g_lpszMp3: Lorencia safezone/bar tile4, nhà thờ Devias tọa độ205..214/13..31, Noria safezone, Dungeon/Atlans/Icarus/Tarkan/LostTower/Kalima/Aida/Kanturu/Balgass/Elbeland/Swamp/Raklion/Santa/Vulcanus/Duel/Imperial/Karutan. Lorencia/Devias/Noria ra ngoài safezone trong cùng map giữ track như nhánh source; vào world dừng nhạc login. Không tự gán nhạc cho Loren Market vì PlayBGM ở source đang comment.

## Phát trong browser và tài nguyên

- Web Audio chỉ khởi động/resume sau thao tác người dùng. Có nút Bật âm thanh/Âm thanh bật-tắt. Mute dừng tiếng và pause nhạc; tab ẩn suspend/pause; đổi world/scene dừng loop/track cũ, hủy kết quả phát async cũ; pagehide dispose.
- WAV nạp theo nhu cầu, kiểm size/hash trước decode. Cache RAM LRU tối đa64 buffer/64MiB decoded, tối đa32 voice và không vượt4 channel/ID. Không thêm cache lưu lâu, không cần clear browser khi sửa. Hash mismatch không phát.
- Music dùng HTMLAudio + Blob đã kiểm hash, chỉ một track tại một thời điểm, revoke URL khi đổi/hủy; MP3 dài không giải mã toàn bộ thành PCM trong RAM.
- `beep.wav` là Microsoft ADPCM mono; Chromium không decode trực tiếp. Bổ sung decoder đọc predictor coefficients từ fmt và số sample từ fact; không thay bằng âm tự tạo. Thuật toán định dạng đối chiếu [FFmpeg ADPCM decoder](https://www.ffmpeg.org/doxygen/trunk/adpcm_8c_source.html). `iChaosCastle.wav` là PCM8bit dài ~98giây, decode ở sample rate thiết bị vượt32MiB; budget64MiB xử lý được và vẫn có LRU.
- Stage958 file âm thanh gốc riêng ở `debug/audio-assets/Data`, qua exact manifest+hash+root containment. Không mở thêm quyền helper9001, không phục vụ config/credential, không sửa các file nguồn. Release hiện vẫn là build transport cơ sở, không đưa toàn bộ Data vào release.

## Kiểm tra

- 223 test client qua sau thay đổi engine/policy/frame/ADPCM; build WASM bridge và contracts bằng emscripten6.0.13 đã qua. Debug/release build tách riêng, deploy nguyên tử.
- Trình duyệt IAB: WAV911/911 hash+decode đạt sau sửa ADPCM/budget; trước đó909/911, hai lỗi trên đã xử lý. Các nút kiểm tra click/nhặt/rơi/sửa/chân/jewel phát được, data-played tăng, failed0, console không lỗi; Pub MP3 play thành công; Music47/47 kiểm hash/metadata đạt. Đi bộ fixture từ(140,125) tới(144,125): data-played5/failed0, nhạc tự chọn MUSIC_MAIN_THEME; mute dừng voice về0, unmute khôi phục track. Ảnh `/tmp/mu-w81-sound-audit.png`.
- Fixture `?scene=audio-check&soundcheck=1`: phát từng nhóm, audit toàn bộ WAV không phát hàng loạt, audit hash/metadata Music, mute/unmute. Fixture không mở GameServer socket hay giao dịch đồ thật.

## Giới hạn phải giữ rõ

Không coi 958 file được stage là 1.529 call-site đã đạt parity. Còn tiếng gắn skill chưa triển khai, boss/event theo stage, mount/pet/cloth/buff, object animation đặc thù, Crywolf/Kanturu3/CastleBattle/CursedTemple/Doppelganger theo event state và nhạc custommap ngoài81. Không đoán event state từ text notice. Các nhánh Music này đã có asset/binding nhưng chưa tự phát nếu client chưa có event state tương ứng. Aida/Elbeland/Kalima object ambience và một số actor-specific sound còn cần nối đúng object/frame gốc. RainCurrent Lorencia chưa có luồng thời tiết nên không tự phát mưa khi không biết có mưa. Bow custom cần Kind2 thật, không gán đại một tiếng.

Web attenuation/panning hiện là khoảng cách25.6tile + stereo, không tuyên bố tái tạo DirectSound3D/Hall chính xác. Thời điểm phát lần đầu phụ thuộc tải/giải mã; chưa preload toàn bộ342MB. Chưa xác nhận nghe toàn bộ trigger trên phiên nhân vật thật trong lượt này; người dùng cần refresh/vào lại nhận WASM và JS mới.
