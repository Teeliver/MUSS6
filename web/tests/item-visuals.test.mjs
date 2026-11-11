import test from 'node:test';import assert from 'node:assert/strict';import {readFile} from 'node:fs/promises';import {execFileSync} from 'node:child_process';
import {goldColor,maximumDurability,itemBackground,upgradeStyle,bindUpgradeMeshes,canonicalItemSize,hoverRadiansPerSecond,defaultItemAngles,equipmentItemOffset,equipmentItemStats,tooltipNameColor,inventoryItemAngles,inventoryItemAnchor,inventoryTRSScale,inventoryItemScale,inventoryItemOverride,inventoryOverrideAnchor,inventoryOverrideModelPath,inventoryFixedScale,hideInventoryMesh} from '../client/src/ui/item-visual-policy.js';
import {itemTRS,itemTRSHash} from '../client/src/ui/item-trs-data.js';
test('Zen colors use original four exact thresholds',()=>{for(const [value,color] of [[0,[255,220,150]],[99999,[255,220,150]],[100000,[0,201,24]],[999999,[0,201,24]],[1000000,[255,150,0]],[9999999,[255,150,0]],[10000000,[255,0,0]],[211330843,[255,0,0]]])assert.deepEqual(goldColor(value),color);});
test('item occupied-cell colors follow actual durability thresholds and level/excellent durability',()=>{const meta={baseDurability:100,magicDurability:80},item={index:0,level:0,durability:100,excellent:0,setOption:0};for(const [durability,color] of [[100,[.3,.5,.5,.6]],[50,[1,1,0,.4]],[30,[1,.66,0,.4]],[20,[1,.33,0,.4]],[0,[1,0,0,.4]]])assert.deepEqual(itemBackground({...item,durability},meta),color);assert.equal(maximumDurability({...item,level:15},meta),147);assert.equal(maximumDurability({...item,level:15,excellent:1},meta),162);assert.deepEqual(itemBackground({...item,index:14*512,durability:0},meta),[.3,.5,.5,.6]);});
test('maximum durability keeps calcMaxDurability staff36 exclusion, helper 4/5, lucky 255 and unclamped bonus',()=>{
 const meta={baseDurability:100,magicDurability:80};
 // ITEM_STAFF+36 (5:36) is on the source exclusion list for the +15 excellent bonus.
 assert.equal(maximumDurability({index:5*512+36,level:15,excellent:1,setOption:0},{baseDurability:100,magicDurability:100}),147);
 // Check_LuckyItem: groups 7..12, tab index 62..72 force 255 after every other rule.
 assert.equal(maximumDurability({index:7*512+62,level:0,excellent:0,setOption:0},{baseDurability:200}),255);
 assert.equal(maximumDurability({index:12*512+72,level:5,excellent:1,setOption:0},{baseDurability:200}),255);
 // HELPER+4/+5 force 255 before the bonus, so the bonus still applies (native order).
 assert.equal(maximumDurability({index:13*512+4,level:0,excellent:1,setOption:0},meta),270);
 // zzzInventory excludes wings 36..40, 42..43 and 49..50 from the +15 excellent bonus; wing41 is NOT excluded.
 assert.equal(maximumDurability({index:12*512+41,level:15,excellent:1,setOption:0},meta),162);
 for(const type of [36,40,42,43,49,50])assert.equal(maximumDurability({index:12*512+type,level:15,excellent:1,setOption:0},meta),147);
 // calcMaxDurability returns a WORD and does not clamp after the excellent/ancient bonus.
 assert.equal(maximumDurability({index:0,level:15,excellent:1,setOption:0},{baseDurability:240}),270);
 // CanChangeItemColorState lists helper 106 and 109..115 as endurance items.
 const item={index:13*512+109,level:0,durability:45,excellent:0,setOption:0};
 assert.deepEqual(itemBackground(item,meta),[1,1,0,.4]);
 assert.deepEqual(itemBackground({...item,index:13*512+106},meta),[1,1,0,.4]);
 assert.deepEqual(itemBackground({...item,index:13*512+108},meta),[.3,.5,.5,.6]);
});
test('equipment stats separate BOOK/scroll Energy, cepter and ItemConvert excellent exclusions',()=>{
 const base={requirements:[0,0,40,0,0,100]};
 assert.equal(equipmentItemStats({index:5*512+21,level:0,excellent:0,setOption:0,option:0},{...base,level:10,requirements:[0,0,40,0,0,0]}).requirements[2],32);
 assert.equal(equipmentItemStats({index:5*512+0,level:0,excellent:0,setOption:0,option:0},{...base,level:10,requirements:[0,0,40,0,0,0]}).requirements[2],36);
 assert.equal(equipmentItemStats({index:15*512+3,level:0,excellent:0,setOption:0,option:0},base).requirements[2],180);
 // Archangel/Sword+19 is excluded from is_excellent by ItemConvert; a normal sword is not.
 const sword={requirements:[0,0,0,0,0,0],level:20,damageMin:10,damageMax:20};
 assert.equal(equipmentItemStats({index:19,level:0,excellent:1,setOption:0,option:0},sword).damageMin,10);
 assert.equal(equipmentItemStats({index:0,level:0,excellent:1,setOption:0,option:0},sword).damageMin,27);
 // Scepters skip the +Level*2 MagicPower step; Frost Mace (2:16) is not a scepter.
 const power={requirements:[0,0,0,0,0,0],level:20,magicPower:40};
 assert.equal(equipmentItemStats({index:2*512+8,level:5,excellent:0,setOption:0,option:0},power).magicPower,27);
 assert.equal(equipmentItemStats({index:2*512+16,level:5,excellent:0,setOption:0,option:0},power).magicPower,37);
});
test('upgrade tiers follow source pass order instead of assigning one glow to every +level',()=>{const item={index:0};for(const [level,modes] of [[0,[]],[2,[]],[3,[]],[5,[]],[7,[1]],[8,[1]],[9,[1,2]],[10,[1,2]],[11,[3,2,1]],[12,[3,2,1]],[13,[4,2,1]],[15,[4,2,1]]])assert.deepEqual(upgradeStyle({...item,level}).passes.map(p=>p.mode),modes);assert.deepEqual(upgradeStyle({...item,level:3}).base,[1,.6,.6]);assert.deepEqual(upgradeStyle({...item,level:5}).base,[.5,.7,1]);assert.deepEqual(upgradeStyle({index:14*512,level:15}).passes.map(p=>p.mode),[4,2,1]);});
test('upgrade geometry reuses posed positions/normals and overlay never casts additional shadow',()=>{const p=new Float32Array(9),n=new Float32Array(9),base={meshes:[{positions:p,normals:n}]},images=new Map([['Effect/Chrome01.OZJ',{}],['Effect/Shiny01.OZJ',{}]]),binding=bindUpgradeMeshes([{index:0,level:9}],images),result=binding.apply(base);assert.equal(result.meshes.length,3);assert.equal(result.meshes[1].positions,p);assert.equal(result.meshes[2].normals,n);assert.equal(result.meshes[1].castsShadow,false);assert.equal(base.meshes.length,1);assert.deepEqual(canonicalItemSize({width:2,height:3}),[40,60]);assert.ok(Math.abs(hoverRadiansPerSecond-350*Math.PI/180)<1e-12);});
test('palette artifact reproduces supplied source functions compiled with explicit item constants',async()=>{const file=new URL('../client/src/ui/item-palette.js',import.meta.url),before=await readFile(file,'utf8');execFileSync(process.execPath,[new URL('../tools/generate_item_palette.mjs',import.meta.url).pathname]);assert.equal(await readFile(file,'utf8'),before);});

test("new source fallback distinguishes staffs and equipment armor offset",()=>{assert.deepEqual(defaultItemAngles(4*512),[180,270,15]);assert.deepEqual(defaultItemAngles(5*512),[270,-10,0]);assert.deepEqual(equipmentItemOffset(3,[75,87,46,66],{width:2,height:3}),[2,-9]);assert.deepEqual(equipmentItemOffset(12,[15,200,40,60],{width:2,height:3}),[0,0]);});

test('tooltip normal equipment includes upgrades and source integer requirement arithmetic',()=>{const a={level:20,damageMin:10,damageMax:20,defense:10,magicPower:40,blocking:0,requirements:[40,20,0,10,0,0]},item={index:2563,level:9,option:1,setOption:0,excellent:0};const s=equipmentItemStats(item,a);assert.equal(s.damageMin,37);assert.equal(s.damageMax,47);assert.equal(s.defense,37);assert.equal(s.magicPower,51);assert.deepEqual(s.requirements,[81,48,0,34,0,0]);assert.equal(tooltipNameColor(item),'#ffcc19');assert.equal(tooltipNameColor({...item,excellent:1}),'#19ff80');assert.equal(tooltipNameColor({...item,index:12*512,level:7,excellent:1,setOption:1}),'#ffcc19');assert.equal(tooltipNameColor({...item,index:12*512,level:6,excellent:1,setOption:1}),'#ffffff');});

test('ItemTRS artifact reproduces the supplied source table',async()=>{const {existsSync}=await import('node:fs');const data='/Users/angelale/Downloads/MuClientEX603/Data/Local/ItemTRSData.bmd';if(!existsSync(data))return;const file=new URL('../client/src/ui/item-trs-data.js',import.meta.url),before=await readFile(file,'utf8');execFileSync(process.execPath,[new URL('../tools/generate_item_trs.mjs',import.meta.url).pathname,'/Users/angelale/Downloads/MuClientEX603/Data']);assert.equal(await readFile(file,'utf8'),before);});
test('inventory hides skin/hair/light-texture meshes but retains actual helmet texture',()=>{for(const texture of ['skin_barbarian_01.jpg','Level01.jpg','HQskin3.jpg','HQhair_01.jpg','hid01.jpg','nude_01.jpg','cotton_01.jpg'])assert.equal(hideInventoryMesh(texture),true,texture);for(const texture of ['head_03.jpg','a9.jpg','shield11_a.jpg'])assert.equal(hideInventoryMesh(texture),false,texture);assert.deepEqual(inventoryItemAngles(5*512+5),[180,270,15]);assert.deepEqual(inventoryItemAngles(6*512),[270,-90,0]);});
test('ItemTRSData decodes per-item angles/anchors and source icon scale',()=>{
 assert.equal(itemTRSHash.length,64);assert.ok(Object.keys(itemTRS).length>3000);
 assert.deepEqual(itemTRS[0],[0.67,0.75,0,180,270,15,0.0025]);
 assert.deepEqual(inventoryItemAngles(0),[180,270,15]);assert.deepEqual(inventoryItemAnchor(0),[0.34,-0.5]);
 assert.deepEqual(inventoryItemAngles(12*512),[270,-10,0]);assert.deepEqual(inventoryItemAnchor(12*512),[0,-0.2]);
 assert.deepEqual(inventoryItemAngles(8191),[270,-10,0]);assert.equal(inventoryItemAnchor(8191),undefined);
 assert.ok(Math.abs(inventoryTRSScale(0,2)-0.01718875)<1e-9);
 assert.ok(Math.abs(inventoryTRSScale(4097,3)-0.0178763)<1e-6);
 assert.ok(Math.abs(inventoryTRSScale(14*512+13,1)-0.0481285)<1e-6);
 assert.equal(inventoryTRSScale(8191,1),undefined);
 // Orb/jewel/bundle families keep the calibrated native scale; raw TRS .002 makes them too small.
 assert.ok(Math.abs(inventoryFixedScale(12*512+7,1)-0.0536275)<1e-6);
 assert.ok(inventoryFixedScale(12*512+7,1)>inventoryTRSScale(12*512+7,1));
 // Rings/pendants have no RenderItem3D scale override; source uses TRS .0025.
 assert.equal(inventoryFixedScale(13*512+8,1),undefined);
 assert.ok(Math.abs(inventoryTRSScale(13*512+8,1)-0.0343775)<1e-7);
 assert.deepEqual(inventoryItemOverride(13*512+19,1),{scale:.001,angle:[180,270,15],pos:[.7,.8]});
 assert.deepEqual(inventoryItemOverride(13*512+19,2).angle,[90,180,20]);
 assert.deepEqual(inventoryItemOverride(14*512+11,3),{scale:.0039,angle:[270,90,0],pos:[.5,.5]});
 assert.equal(inventoryItemOverride(0,0),undefined);
 assert.ok(Math.abs(inventoryItemScale(13*512+19,1,1)-.013751)<1e-9);
 assert.equal(inventoryItemScale(0,0,1),undefined);
 assert.deepEqual(inventoryOverrideAnchor(13*512+19,1),[0.4,-0.6]);
 assert.deepEqual(inventoryOverrideAnchor(14*512+21,1),[-0.2,-0.6]);
 assert.equal(inventoryOverrideAnchor(0,0),undefined);
 assert.equal(inventoryOverrideModelPath(13*512+19,1),'Item/Sword20.bmd');
 assert.equal(inventoryOverrideModelPath(13*512+19,2),'Item/Bow19.bmd');
 assert.equal(inventoryOverrideModelPath(13*512+19,0),'Item/Staff11.bmd');
 assert.equal(inventoryOverrideModelPath(13*512+20,1),'Item/RingOfLordEvent00.bmd');
 assert.equal(inventoryOverrideModelPath(14*512+11,3),'Item/MagicBox05.bmd');
 assert.equal(inventoryOverrideModelPath(14*512+11,15),'Item/Beer02.bmd');
 assert.equal(inventoryOverrideModelPath(14*512+21,1),'Item/EventBloodCastle03.bmd');
 assert.equal(inventoryOverrideModelPath(0,0),undefined);
});

test('actual Staff06 retains its long projected shaft and Bone Helm skin can be removed',async()=>{const {existsSync}=await import('node:fs');const base='/Users/angelale/Downloads/MuClientEX603/Data/';if(!existsSync(base+'Item/Staff06.bmd'))return;const {default:create}=await import('../.build/wasm/frame-reader.mjs');const core=await create({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});const result=core.bmdGeometryResult(new Uint8Array(await readFile(base+'Item/Staff06.bmd')));assert.equal(result.ok,true);const [ax,ay,az]=inventoryItemAngles(2565).map(v=>v*Math.PI/180),lo=[Infinity,Infinity],hi=[-Infinity,-Infinity];for(const mesh of result.meshes)for(let i=0;i<mesh.positions.length;i+=3){const [x,y,z]=mesh.positions.slice(i,i+3),y1=Math.cos(ax)*y-Math.sin(ax)*z,z1=Math.sin(ax)*y+Math.cos(ax)*z,x2=Math.cos(ay)*x+Math.sin(ay)*z1;const p=[Math.cos(az)*x2-Math.sin(az)*y1,Math.sin(az)*x2+Math.cos(az)*y1];p.forEach((v,k)=>{lo[k]=Math.min(lo[k],v);hi[k]=Math.max(hi[k],v);});}assert.ok(hi[1]-lo[1]>240);assert.ok(hi[1]-lo[1]>3*(hi[0]-lo[0]));const helm=core.bmdGeometryResult(new Uint8Array(await readFile(base+'Player/HelmMale05.bmd')));assert.equal(helm.ok,true);assert.deepEqual(helm.meshes.filter(m=>!hideInventoryMesh(m.texture)).map(m=>m.texture),['head_06.jpg','head_a06.tga']);const shield=core.bmdGeometryResult(new Uint8Array(await readFile(base+'Item/Shield01.bmd'))),raw=[[],[],[]];for(const m of shield.meshes)for(let i=0;i<m.positions.length;i++)raw[i%3].push(m.positions[i]);const extent=raw.map(v=>Math.max(...v)-Math.min(...v));assert.ok(extent[1]>extent[0]*2&&extent[2]>extent[0]*2,'Shield01 must face the camera after rotating its X normal into Z');});

test('event and helper items use original effective levels rather than silently skipping their upgrades',async()=>{const {effectiveItemLevel}=await import('../client/src/ui/item-visual-policy.js');assert.equal(effectiveItemLevel({index:14*512+17,level:2}),4);assert.equal(effectiveItemLevel({index:14*512+17,level:7}),13);assert.equal(effectiveItemLevel({index:12*512+3,level:15}),0);assert.equal(upgradeStyle({index:13*512+16,level:9}).passes.length,2);assert.equal(upgradeStyle({index:14*512+17,level:7}).passes[0].mode,4);});
test('Armor of Guardsman and nested original model bindings resolve their actual texture directories',async()=>{const {itemTextureReference,textureReference}=await import('../client/src/platform/assets/decode-texture.js');const {itemModelBindings,itemTextureBindings}=await import('../client/src/protocol/item-model-bindings.js');assert.equal(itemTextureReference(13*512+29,'Item/EventChaosCastle.bmd','smith.jpg'),'Npc/smith.OZJ');assert.equal(itemTextureReference(6*512+17,'Item/Shield_18.bmd','flareRed.jpg'),'Effect/flareRed.OZJ');assert.ok(itemModelBindings[8*512+62].startsWith('Player/LuckyItem/62/'));assert.throws(()=>textureReference('Item/../Player/x.bmd','x.jpg'));assert.throws(()=>itemTextureReference(6685,'Item/EventChaosCastle.bmd','../x.jpg'));assert.ok(Object.keys(itemTextureBindings).length>90);});

test('Illusion Sorcerer Covenant retains source special Chrome2/Chrome4 even with forced level zero',()=>{assert.deepEqual(upgradeStyle({index:13*512+50,level:6}).passes.map(p=>p.mode),[3,4]);});
test('effective item levels reproduce the compiled original source switch',async()=>{const {existsSync}=await import('node:fs');const original='/Users/angelale/Downloads/SOURCE-2.0.7/Main5.2/source/ZzzObject.cpp';if(!existsSync(original))return;const file=new URL('../client/src/ui/item-effect-levels.js',import.meta.url),before=await readFile(file,'utf8');execFileSync(process.execPath,[new URL('../tools/generate_item_effect_levels.mjs',import.meta.url).pathname,original]);assert.equal(await readFile(file,'utf8'),before);});

test('inventory strips embedded face/hair body textures without stripping armor panels',()=>{for(const t of ['SkinClass106head_N.jpg','SkinClass306headhair01_N.tga','skinClass301.jpg','richface.jpg','cobraset_hair.tga'])assert.equal(hideInventoryMesh(t),true,t);for(const t of ['SkinClass106_headhelmet.jpg','SkinClass606_upper.jpg','Item3002_Armor.jpg'])assert.equal(hideInventoryMesh(t),false,t);});

test('native wing material passes remove black-key backgrounds and add MG chrome',async()=>{
 const {wingMeshMaterial,bindUpgradeMeshes}=await import('../client/src/ui/item-visual-policy.js');
 assert.equal(wingMeshMaterial(6144,0).additive,true);
 assert.equal(wingMeshMaterial(6147,0).additive,true);
 assert.equal(wingMeshMaterial(6180,0).additive,true);
 assert.equal(wingMeshMaterial(6180,2).additive,undefined);
 const chrome={kind:'test'},style=bindUpgradeMeshes([{index:6150,level:0}],new Map([['Effect/Chrome01.OZJ',chrome]]));
 assert.ok(style.paths.includes('Effect/Chrome01.OZJ'));
 const mesh=style.apply({meshes:[{textureIndex:0}]}).meshes.at(-1);
 assert.equal(mesh.image,chrome);assert.equal(mesh.additive,true);assert.deepEqual(mesh.materialTint,[.4,.3,.5]);
});

test('Wing of Storm draws opaque frame before both native light passes',async()=>{
 const {bindUpgradeMeshes}=await import('../client/src/ui/item-visual-policy.js');
 const items=Array.from({length:3},()=>({index:6180,level:0}));
 const result=bindUpgradeMeshes(items,new Map()).apply({meshes:[0,1,2].map(textureIndex=>({textureIndex}))});
 assert.deepEqual(result.meshes.map(m=>m.textureIndex),[2,0,1]);
 assert.equal(result.meshes[0].additive,undefined);assert.equal(result.meshes[1].additive,true);assert.equal(result.meshes[2].additive,true);
});
test('Excellent uses source time tint and original texture after level passes, excluding wings/cape',async()=>{
 const {excellentMaterial,excellentTint}=await import('../client/src/ui/item-visual-policy.js');assert.deepEqual(excellentTint(0),[.5,.15,.5]);assert.ok(excellentTint(Math.PI/4)[0]>.999);
 for(const index of [6144,6150,6180,6187,6193,6194,6274,6279,6686])assert.equal(excellentMaterial({index,excellent:63}),false);
 assert.equal(excellentMaterial({index:3585,excellent:1}),true);assert.equal(excellentMaterial({index:3585,excellent:64}),false);
 const image={},mesh={image,sourceMeshOrdinal:0,positions:new Float32Array(9),normals:new Float32Array(9)},binding=bindUpgradeMeshes([{index:3585,level:9,excellent:1}],new Map());const result=binding.apply({meshes:[mesh]});assert.equal(result.meshes.length,4);const pass=result.meshes.at(-1);assert.equal(pass.image,image);assert.equal(pass.excellentPulse,true);assert.equal(pass.additive,true);assert.equal(pass.castsShadow,false);assert.equal(pass.upgradeMode,undefined);
 const special=bindUpgradeMeshes([0,1,2].map(()=>({index:7*512+39,level:0,excellent:1})),new Map());const initial=special.apply({meshes:[0,1,2].map(sourceMeshOrdinal=>({...mesh,sourceMeshOrdinal}))});assert.equal(initial.meshes.length,4);assert.equal(initial.meshes.at(-1).sourceMeshOrdinal,2);assert.equal(special.apply({meshes:[mesh,mesh,mesh].map(({sourceMeshOrdinal,...rest})=>rest)}).meshes.length,4);
});
