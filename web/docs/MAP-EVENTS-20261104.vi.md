# Map — hiệu ứng có vòng đời và vật liệu Empire — 04-11-2026

Tiếp tục danh sách trong MAP-CONSOLIDATED, xử lý các nhánh chung trên69–74 bằng source và Data hiện có. Không đóng toàn bộ hiệu ứng47map.

## Đã thực hiện

### Marker85: hai tầng effect/particle

GMEmpireGuardian1/2/3/4::RenderObjectVisual: int(WorldTime)%200 nằm trong0…1 thì tạo rand4+4 effect BITMAP_FLAME subtype6. ZzzEffect::CreateEffect đặt lifetime40; mỗi MoveEffect phát một particle BITMAP_FLAME subtype12, vị tríXY lệch rand32−16world, sau đó giảm lifetime. Không thay bằng một particle mỗi200ms hoặc phát liên tục.

ZzzEffectParticle init subtype12:life10,velocityZ=(rand128+128)×0,15world/tick,scale=placementScale×(rand64+64)×0,01; RGB0,5 do marker truyền, không tự đổi màu. MovePosition xoay velocity theo AngleMatrix; subtype12 không có fade bổ sung. Texture đúng Effect/Flame01.OZJ; stream decode thật đã qua test. Chỉ nay mới ẩn12marker85 ởmap72; cácmapkhác hiệnkhôngcó placement85.

Web có effectpool riêng1000 và particlepool chung3000; capacity của family theo trường hợp cực đại chặn3000. Có created/live/peak/dropped diagnostics và dọn khi rời map.

**Đồng hồ:** cửa sổ nguồn chỉ2ms. Lấy performance.now tại điểm kiểm tra, không dùng timestamp dự báo vblank của RAF. Đã thử phục hồi cửa sổ bị bỏ qua nhưng đo thực tế gây over-emission, chạm3000particle và tụt20FPS; cách đó đã loại bỏ. Bản cuối giữ chính xác phép kiểm tra int(time)%200≤1, không phát bù backlog. Do điều kiện nguồn phụ thuộc pha thời gian, không cam kết luôn có một đợt mỗi200ms. Test inject clock để xác nhận việc lấy timer độc lập với RAF, lifetime và đường render thật.


### Type64: khói máy theo animation

BaCLOUD22chỉ phát khi frame>9,5&&<11,5 hoặc>23,5&&<25,5. Vịtrí lệch bằng AngleMatrix(0,0,rz+90)×(0,30,0), màu(0,04;0,03;0,02), scaleplacement. CLOUD22life80,alpha0,1,scatterXY±10world/Z−20…−1,scale(rand20+50)×0,003,velocity ngẫu nhiên theo góc. Giữ MovePosition của source trước cập nhật: tăngalpha0,1 vàZ0,5world nửatuổiđầu, giảmalpha0,025 vàZ−0,5nửasau,scale+0,001/tick.

BMD metadata nay trả period thực của clip theo cùng quy tắc locked-root với sampler. Dùng period này và cùng clock/rate với model; không đoán framecount. Type64map69/70/72/73/74=16keys/s;map71=11keys/s. Trước lượt này69/70/71còn fallback4keys/s. Data có14machine placement:70=3,71=1,72=8,74=2.

### Vật liệu và tốc độ dùng chung

- GMEmpireGuardian1::RenderObjectMesh được cảbốncontroller gọi: type37/41mesh1additive;type49/70mesh0additive. Type41mesh1pulse sin(WorldTime×0,002)×0,5+0,6.
- Type47mesh3tint1,2;type48mesh0tint1,1. Không áp vào mesh khác chung texture. Type96/97/100 trên72–74tint0,170382 theo controller4.
- Type115mesh1V+0,4/s,mesh3V−0,4/s. Tương đương modulo25000×0,0004 của PC do wrap tại10chu kỳUV.
- Type20=8keys/s;122/123/124=12;128=24, dựa velocity gốc0,16×25Hz và multiplier2/3/6. Chưa giả lập các thay đổi subtype/action do eventserver.
- Các RenderMesh branch phải dùng **ordinal mesh**, không lấy textureIndex làm ordinal. Environment đã áp đúng và tách batch theo tint/pulse/UV/additive. Test dùng4mesh cùngtextureIndex0 để bắt nhầm lẫn này.

## Kiểm chứng và giới hạn

WASM đã biên dịch lại; debug172file publish. **465/465 test web qua**;Có9test mới. Bao gồm fakeGPU integration xác nhận clock độc lập với timestamp RAF, đường phát particle và dispose;BMD period/texture dùng Data thật.

Browser bản cuối map72 tại32,68:60FPS,framep9517,7ms,CPU p958ms,buffer1582×1798;created54effect,Flame12Peak540,Cloud22Peak84,droppedparticle/effect/bone/sprite0. Có54effectđã phát và đi qua pipeline render, không chỉ test hàm policy. Tại thời điểm lấy mẫu effect đã hết tuổi nên live0 là đúng. Map71 tại110,195:Cloud22Peak45,60FPS,CPU p957,6ms; xác nhận controller tốc độ11keys/s. Map70 tại26,59:Cloud22Peak126,60FPS,CPU p957,1ms,dropped0. Đây là mẫu đơn lẻ, không phải benchmark dài hạn.

[Ảnh bản cuối](map-events-20261104/empire-final.png).
 Test và số đo browser xem các artifact cùng thư mục. Coverage chạy lại toàn47map vẫn254placement unresolved, không giấu các cảnh báo resource khác để làm giảm số đếm.

[Coverage](map-events-20261104/map-coverage.json) · [Readiness](map-events-20261104/readiness.json) · [Effect inventory](map-events-20261104/effects.json) · [Log test](map-events-20261104/client-tests.log)

Cập nhật: fire theo bone type50 đã có pipeline nhưng model Data không đủ bone; xem [kiểm tra bone](MAP-BONE-VALIDATION-20261104.vi.md). Chưaport joint spark20/51;chrome đỏ hai pass của64;effect/eventđặcbiệt và render-after-character. Bốnmapterrainthiếu texture trong báo cáo tổng hợp vẫn còn. Không chứng nhận pixelparity hoặc toàn maphoàn tất. Không thay Data/DB, không restart server.

Giữ một archive code mới nhất trước lượt này; không tạo backup theo từng lần sửa. Backup DB riêng giữ nguyên. Dọn cây test tạm sau lưu log.
