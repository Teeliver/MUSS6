import {effectLevels} from './item-effect-levels.js';
import {itemPalette} from './item-palette.js';
import {itemTRSAt} from './item-trs-data.js';
export function goldColor(value){return value>=10000000?[255,0,0]:value>=1000000?[255,150,0]:value>=100000?[0,201,24]:[255,220,150];}
// ZzzInventory.cpp::calcMaxDurability. Level>=13 caps at 255 during the loop; the
// ancient/excellent bonus is added afterwards and may exceed 255 (native WORD result).
export function maximumDurability(item,meta){const group=item.index>>9,type=item.index&511;let value=group===5?meta.magicDurability:meta.baseDurability;value??=0;for(let i=0;i<item.level&&item.index<13*512+51;i++){if(i<13)value+=i<4?1:i<9?2:i===9?3:i===10?4:i===11?5:6;else{const step=i===13?7:8;value=value+step<255?value+step:255;}}if(item.index===13*512+4||item.index===13*512+5)value=255;const ancient=item.setOption&3;if(ancient===1||ancient===2)value+=20;else if(item.excellent&&!(group===12&&type>=3&&type<=6)&&![19,4*512+18,5*512+10,13*512+30,2*512+13,5*512+36].includes(item.index)&&!(group===12&&((type>=36&&type<=40)||(type>=42&&type<=43)||(type>=49&&type<=50))))value+=15;if(group>=7&&group<=12&&type>=62&&type<=72)value=255;return value;}
export function itemBackground(item,meta){const normal=[.3,.5,.5,.6],group=item.index>>9,type=item.index&511,endurance=group<12||group===12&&(type<=6||type>=36&&type<=43||type===49||type===50||type>=130&&type<=135)||group===13&&(type<=5||[8,9,10,12,13,20,30,37,64,65,76,80,106,107,122,123].includes(type)||type>=21&&type<=28||type>=38&&type<=42||type>=109&&type<=115);if(!endurance||group===13&&type===20&&[1,2].includes(item.level))return normal;const max=maximumDurability(item,meta);if(!item.durability)return [1,0,0,.4];if(max&&item.durability<=max*.2)return [1,.33,0,.4];if(max&&item.durability<=max*.3)return [1,.66,0,.4];if(max&&item.durability<=max*.5)return [1,1,0,.4];return normal;}
export function effectiveItemLevel(item){return item?effectLevels[item.index]?.[item.level]??item.level:0;}
function levelUpgradeStyle(item){if(!item)return {base:[1,1,1],passes:[]};const level=effectiveItemLevel(item),palette=itemPalette[item.index],base=level<3?[1,1,1]:level<5?[1,.6,.6]:level<7?[.5,.7,1]:level<9?[.8,.8,.8]:[.9,.9,.9];if(level<7){const special=[13*512+50,13*512+38,14*512+42,13*512+43,13*512+93,13*512+44,13*512+94,13*512+116,13*512+45];if(special.includes(item.index)){const tint=palette[1];return {base:special.indexOf(item.index)>=3?[.9,.9,.9]:base,passes:[{mode:3,path:'Effect/Chrome02.OZJ',tint:tint.map(v=>v*1.5)},{mode:4,path:'Effect/Chrome02.OZJ',tint}]};}return {base,passes:[]};}const chrome={mode:1,path:'Effect/Chrome01.OZJ',tint:palette[0]},metal={mode:2,path:'Effect/Shiny01.OZJ',tint:palette[0]},extra={mode:level<13?3:4,path:'Effect/Chrome02.OZJ',tint:palette[1]};return {base,passes:level>=11?[extra,metal,chrome]:level>=9?[chrome,metal]:[chrome]};}
// ZzzObject.cpp::RenderPartObjectEffect, original-texture RENDER_BRIGHT pass.
export function excellentMaterial(item){if(!item||!(item.excellent&63))return false;const group=item.index>>9,type=item.index&511;return !(group===12&&(type<=6||type>=36&&type<=43||type>=49&&type<=50||type>=130&&type<=135)||group===13&&type===30);}
export function excellentTint(seconds){const luminosity=Math.sin(seconds*2)*.5+.5;return [luminosity,luminosity*.3,1-luminosity];}
export function upgradeStyle(item){return {...levelUpgradeStyle(item),excellent:excellentMaterial(item)};}
function excellentMesh(item,mesh){const group=item.index>>9,type=item.index&511,ordinal=mesh.sourceMeshOrdinal;if(group===7&&type>=39&&type<=44)return ordinal===[2,1,0,2,1,2][type-39];if((group===7||group===8)&&type>=59&&type<=61)return ordinal===(type===61?0:1);return true;}
// ZzzObject ItemObjectAttribute and RenderPartObject: native wing mesh passes.
export function wingMeshMaterial(index,textureIndex){if([6144,6147].includes(index)&&textureIndex===0)return {additive:true,castsShadow:false};if(index===6180){if(textureIndex===0)return {additive:true,materialTint:[1,.7,.5],castsShadow:false};if(textureIndex===1)return {additive:true,materialTint:[.9,.6,.3],castsShadow:false};if(textureIndex===2)return {materialTint:[1,.7,.5]};}if(index===6183&&textureIndex===0)return {additive:true,castsShadow:false};return {};}
export function bindUpgradeMeshes(meshItems,images){const styles=meshItems.map(upgradeStyle);let excellentSelection;return {apply(geometry){excellentSelection??=geometry.meshes.map((mesh,i)=>styles[i].excellent&&excellentMesh(meshItems[i],mesh));const base=geometry.meshes.map((mesh,i)=>({...mesh,materialTint:styles[i].base,...wingMeshMaterial(meshItems[i]?.index,mesh.textureIndex)})),overlays=[];for(let i=0;i<base.length;i++){if(meshItems[i]?.index===6150)overlays.push({...base[i],image:images.get('Effect/Chrome01.OZJ'),additive:true,upgradeMode:1,materialTint:[.4,.3,.5],castsShadow:false});for(const pass of styles[i].passes)overlays.push({...base[i],image:images.get(pass.path),additive:true,upgradeMode:pass.mode,materialTint:pass.tint,castsShadow:false});if(excellentSelection[i])overlays.push({...base[i],image:base[i].image,additive:true,multiply:false,alphaBlend:false,excellentPulse:true,materialTint:[1,1,1],castsShadow:false});}const ordered=[...base];const stormSlots=meshItems.flatMap((item,i)=>item?.index===6180?[i]:[]);if(stormSlots.length===3){const order=[2,0,1];for(let j=0;j<3;j++)ordered[stormSlots[j]]=base[stormSlots[order[j]]];}return {...geometry,meshes:[...ordered,...overlays]};},paths:[...new Set([...styles.flatMap(s=>s.passes.map(p=>p.path)),...(meshItems.some(i=>i?.index===6150)?['Effect/Chrome01.OZJ']:[])])]};}
export const hoverRadiansPerSecond=14*25*Math.PI/180;
export function canonicalItemSize(meta){return [meta.width*20,meta.height*20];}

// ScriptItem::FindItemRelative fallback; per-item ItemTRSData overrides remain unavailable.
export function defaultItemAngles(index){return (index>>9)<5?[180,270,15]:[270,-10,0];}
export function equipmentItemOffset(slot,rect,meta){const [w,h]=canonicalItemSize(meta);return slot<12?[1+(rect[2]-4-w)/2,(rect[3]-4-h)/2-(slot===3?10:0)]:[0,0];}

// CalculateItem arithmetic, shared with native/client/item_stats.cpp. Normal equipment only.
// ZzzInfomation.cpp::ItemConvert. Ancient items force is_excellent, so they take the
// +25 item-level/excellent branches; the +30 drop level is only used by ancient bonuses.
export function equipmentItemStats(item,a){
 const group=item.index>>9,type=item.index&511,level=item.level,upgrade=level*3+(level>=10?(level-9)*(level-8)/2:0);
 const ancient=[1,2].includes(item.setOption&3),baseLevel=a.level??0;
 // ItemConvert clears is_excellent for Archangel/wing/special families before ancient re-enables it.
 const noExcellent=[19,4*512+18,5*512+10,2*512+13,13*512+30,5*512+36].includes(item.index)||(group===12&&((type>=3&&type<=6)||(type>=36&&type<=40)||(type>=42&&type<=43)||(type>=49&&type<=50)||(type>=130&&type<=135)));
 const excellent=ancient||(!!item.excellent&&!noExcellent);
 const effective=baseLevel+(excellent?25:0),dropLevel=baseLevel+30,highLevel=baseLevel>=220,reqLevel=(a.requirements??[])[5]??0;
 const requirements=(a.requirements??[0,0,0,0,0,0]).map((n,i)=>{
  if(i===5)return n;
  if(!n)return 0;
  if(group===13&&type===5&&i===4)return 185+n*15;
  if(highLevel&&i!==4)return i===2?(excellent?Math.floor(n*2+n*0.035*level):Math.floor(n+n*0.035*level)):(excellent?Math.floor(n+n*0.75+n*0.02*level):Math.floor(n+n*0.02*level));
  // ItemConvert: BOOK uses Level+item_level, skill scroll/parchment uses RequireLevel.
  if(i===2&&group===5&&type>=21&&type<=23)return 20+Math.floor(3*(level+effective)*n/100);
  if(i===2&&group===15&&reqLevel>0)return 20+Math.floor(4*reqLevel*n/100);
  return 20+Math.floor(n*(effective+level*3)*(i===2?4:3)/100);
 });
 const chaos=item.index===2*512+6?15:item.index===5*512+7?25:item.index===4*512+6?30:0;
 const attack=base=>{
  if(!base)return 0;
  if(excellent&&baseLevel)base+=chaos||Math.floor(25*a.damageMin/baseLevel)+5;
  if(ancient)base+=Math.floor(dropLevel/40)+5;
  return base+upgrade;
 };
 let defense=a.defense??0,blocking=a.blocking??0,power=a.magicPower??0;
 if(defense){
  if(group===6){defense+=level;if(ancient)defense+=Math.floor(20*defense/dropLevel)+2;}
  else{if(excellent&&baseLevel)defense+=Math.floor(12*a.defense/baseLevel)+Math.floor(baseLevel/5)+4;if(ancient)defense+=Math.floor(3*defense/dropLevel)+Math.floor(dropLevel/30)+2;defense+=upgrade;}
 }
 // SuccessfulBlocking has no ancient branch in the source.
 if(blocking){if(excellent&&baseLevel)blocking+=Math.floor(25*a.blocking/baseLevel)+5;blocking+=upgrade;}
 if(power){
  if(excellent&&baseLevel)power+=Math.floor(25*a.magicPower/baseLevel)+5;
  if(ancient)power+=Math.floor(dropLevel/60)+5;
  power=Math.floor((power+upgrade)/2)+((group===2&&((type>=8&&type<=15)||(type>=17&&type<=18)))?0:level*2);
 }
 if(item.option&&group<=11)requirements[0]+=item.option*5;
 return {damageMin:attack(a.damageMin),damageMax:attack(a.damageMax),defense,blocking,magicPower:power,magicDefense:a.magicDefense?a.magicDefense+upgrade:0,requirements};
}
// NewUIItemTooltip name colour. Wings (IsEnableOptionWing) win over ancient/excellent:
// level<7 keeps the normal colour, level>=7 is always yellow.
export function tooltipNameColor(item){const g=item.index>>9,n=item.index&511;if(g===12||(g===13&&n===30))return item.level>=7?'#ffcc19':'#ffffff';if(item.setOption&3)return '#ffffff';if(item.excellent)return '#19ff80';if(item.level>=7)return '#ffcc19';if(item.luck||item.skill||item.option)return '#80b3ff';return '#ffffff';}

// Dedicated RF inventory meshes are authored in the frontal XY plane; their
// per-item TRS override remains unavailable (see W0102).
// Current Data uses the pre-W0069 staff orientation. Main5.2's fallback requires its missing ItemTRSData overrides.
// Native inventory pose comes from ScriptItem fallback; do not flip the wing root.
export function inventoryItemAngles(index){if((index>>9)===8&&[59,60,61,73,83].includes(index&511))return [0,0,0];return (index>>9)===5?[180,270,15]:(index>>9)===6?[270,-90,0]:defaultItemAngles(index);}
// TRS anchor retained for future renderer calibration; not applied to icons yet.
export function inventoryItemAnchor(index){const t=itemTRSAt(index);return t?[+((t[0]-.5)*2).toFixed(6),+((.5-t[1])*2).toFixed(6)]:undefined;}
// RenderItem3D per-type ItemScale/ItemAngle overrides (ZzzInventory.cpp:7975-8263).
// pos[]/modelType from the same switch are not applied yet; model substitutions for
// armor inventory meshes are handled by inventoryModelPath.
export function inventoryItemOverride(index,level=0){
 const g=index>>9,t=index&511;
 if(g===13&&t===19)return level===1?{scale:.001,angle:[180,270,15],pos:[.7,.8]}:level===2?{scale:.0015,angle:[90,180,20],pos:[.7,.7]}:level===0?{scale:.001,angle:[180,270,15],pos:[.5,.5]}:undefined;
 if(g===13&&t===20)return level>=1&&level<=3?{scale:.0028,angle:[255,160,0],pos:[.5,.8]}:level===0?{angle:[270,0,0]}:undefined;
 if(g===14&&t===9&&level===1)return {scale:.0035,angle:[270,-90,0]};
 if(g===14&&t===12&&level===1)return {scale:.0025,angle:[270,0,0],pos:[.5,.8]};
 if(g===14&&t===21)return level===1?{scale:.0015,angle:[-90,-20,-20],pos:[.4,.8]}:level===2?{scale:.0015,angle:[270,0,0],pos:[.4,.8]}:level===3?{scale:.002,angle:[-90,-10,0],pos:[.5,.5]}:undefined;
 if(g===14&&t===23&&level===1)return {scale:.0012,angle:[250,140,0]};
 if(g===14&&t===24&&level===1)return {angle:[270,-90,0],pos:[.5,.8]};
 if(g===14&&t===32&&level===1)return {pos:[.5,.45]};
 if(g===14&&t===33&&level===1)return {pos:[.5,.57]};
 if(g===14&&t===34&&level===1)return {pos:[.5,.57]};
 if(g===14&&t===11){
  if(level===1)return {scale:.0025,angle:[270,-10,0]};
  if(level===2)return {scale:.0025,angle:[270,0,0]};
  if(level===3)return {scale:.0039,angle:[270,90,0],pos:[.5,.5]};
  if(level===5)return {scale:.0015,angle:[270,-10,0]};
  if(level===6)return {scale:.0019,angle:[270,-10,0]};
  if(level===13)return {scale:.0039,angle:[270,90,0],pos:[.5,.5]};
  if(level>=14&&level<=15)return {scale:.0025,angle:[270,0,0],pos:[.5,.8]};
  if(level>=8&&level<=12)return {scale:.001,angle:[270,-10,0]};
 }
 if(g===13&&t===14&&level===1)return {scale:.002,angle:[-90,0,0],pos:[.55,.85]};
 return undefined;
}
export function inventoryOverrideAnchor(index,level=0){const o=inventoryItemOverride(index,level);return o?.pos?[+((o.pos[0]-.5)*2).toFixed(6),+((.5-o.pos[1])*2).toFixed(6)]:undefined;}
export function inventoryOverrideModelPath(index,level=0){
 const g=index>>9,t=index&511;
 if(g===13&&t===19)return level===1?'Item/Sword20.bmd':level===2?'Item/Bow19.bmd':level===0?'Item/Staff11.bmd':undefined;
 if(g===13&&t===20)return level>=1&&level<=3?'Item/RingOfLordEvent00.bmd':level===0?'Item/MagicRing00.bmd':undefined;
 if(g===13&&t===14&&level===1)return 'Item/DarkLordSleeve.bmd';
 if(g===14&&t===11){
  if(level===1)return 'Item/MagicBox02.bmd';
  if(level===2)return 'Item/MagicBox03.bmd';
  if(level===3||level===13)return 'Item/MagicBox05.bmd';
  if(level===5)return 'Item/MagicBox06.bmd';
  if(level===6)return 'Item/MagicBox07.bmd';
  if(level>=8&&level<=12)return 'Item/MagicBox08.bmd';
  if(level>=14&&level<=15)return 'Item/Beer02.bmd';
 }
 if(g===14&&t===12&&level===1)return 'Item/Beer02.bmd';
 if(g===14&&t===21&&(level===1||level===2))return 'Item/EventBloodCastle03.bmd';
 if(g===14&&t===23&&level===1)return 'Item/QuestItem3rd00.bmd';
 if(g===14&&t===24&&level===1)return 'Item/QuestItem3rd01.bmd';
 if(g===14&&t===32&&level===1)return 'Item/p03box.bmd';
 if(g===14&&t===33&&level===1)return 'Item/obox02.bmd';
 if(g===14&&t===34&&level===1)return 'Item/blue01.bmd';
}

export const itemViewPixelsPerUnit=137.51;
export function inventoryItemScale(index,level,height){const o=inventoryItemOverride(index,level);return o?.scale===undefined?undefined:o.scale*2*itemViewPixelsPerUnit/(Math.max(1,height)*20);}
export function inventoryTRSScale(index,height){const t=itemTRSAt(index);return t?t[6]*2*itemViewPixelsPerUnit/(Math.max(1,height)*20):undefined;}
export function inventorySourceScale(index){return itemTRSAt(index)?.[6];}
// CGMModelManager::OpenTexture + BMD::RenderMesh HideSkin for item-only views.
export function hideInventoryMesh(texture,index){if(index!==undefined&&((index>>9)<7||(index>>9)>11))return /^(?:skin_|level|hid|hair|hqskin|hqlevel_|hqhair|nude_|cotton_|cotton2_|k_nude)/i.test(texture);return /^(?:skinclass30[1-5]\.|skinclass\d+head(?:hair\d*)?_n\.|[^/]*hair[^/]*\.|[^/]*face\.|skin_|level|hid|hair|hqskin|hqlevel_|hqhair|nude_|cotton_|cotton2_|k_nude)/i.test(texture);}

export function jewelryIconScale(index,height){const type=index&511;return index>>9===13&&([8,9,10,12,13,20,21,22,23,24,25,26,27,28,38,39,40,41,42,76,107,122].includes(type))?(2*(240*.0039/(200*Math.tan(Math.PI/360))))/(height*20):undefined;}

// RenderItem3D substitutes dedicated inventory armor models. Never use these
// substitutions for the equipped character, whose geometry is different.
export function inventoryModelPath(index,path){if((index>>9)!==8)return path;const name=({59:'Armor_inventory60',60:'ArmorMale61_inventory',61:'ArmorMale62_inventory',73:'ArmorMale74_inven',83:'Armor_inventory60'})[index&511];return name?'Player/'+name+'.bmd':path;}
// ZzzObject RenderPartObject's HideSkin mesh passes (mesh ordinal, not texture ID).
export function inventoryMeshVisible(index,ordinal,texture){const group=index>>9,type=index&511;if(group===7){const selected=type>=39&&type<=44?[2,1,0,2,1,2][type-39]:[59,60,82,83].includes(type)?1:[61,73].includes(type)?0:undefined;if(selected!==undefined)return ordinal===selected;}
 if(group===8&&type===73)return ordinal===0;
 return !hideInventoryMesh(texture,index);
}
// Orb models are small native models, not thumbnails normalized to fill a cell.
// ScriptItem fallback .0039; UI FOV=1 degree, depth=200, native height=480.
export function inventoryFixedScale(index,height){const group=index>>9,type=index&511;return (group===12&&((type>=7&&type<=19)||[30,31].includes(type)||type>=136&&type<=143)||group===14&&[0,13,14,16,22,31,41,42,43,44].includes(type))?2*(240*.0039/(200*Math.tan(Math.PI/360)))/(height*20):undefined;}

export function itemRequirements(item,meta){if((item.index>>9)<=11)return equipmentItemStats(item,meta).requirements;const result=[...(meta.requirements??[])],g=item.index>>9,t=item.index&511;
 // CalculateItemRequireStat: first/second generation wings gain required level
 // with upgrades; skill orbs, higher wings and Fenrir retain the table level.
 const fixed=g===12&&(t>=7&&t<=40||t>=43&&t!==49)||g===13&&t===37;
 if(result[5]&&!fixed)result[5]+=(item.level??0)*(g===12&&(t>=3&&t<=6||t===42)?5:4);return result;
}

// RenderObjectScreen uses ScaleMA(.10/.07) for depth. ScaleMA multiplies by
// window height/480, cancelling the viewport's model magnification. The UI
// rectangle may grow, but the model must not grow again with the panel.
export function inventoryDisplayScale(panelScale,picked=false){if(!Number.isFinite(panelScale)||panelScale<=0)throw Error('Invalid item panel scale');return (picked?10/7:1)/panelScale;}
export function inventoryUsesWindowDepth(index){const type=index&511;return (index>>9)===12&&type>=7&&type<=19&&type!==15;}
export function inventoryFixedPlacement(index,lo,hi){return ([7168,7184,6280].includes(index)||jewelryIconScale(index,1)!==undefined)?{center:lo.map((v,i)=>(v+hi[i])/2),anchor:[0,0]}:{center:[0,0,0],anchor:[0,-.6]};}

// attachment_geometry.hpp uses the same OZT row convention for original wings.
// Model-preview applies 1-v globally; compensate the top-first OZT payload here.
export function prepareInventoryWingUv(index,model){const type=index&511,wing=(index>>9)===12&&(type<=6||type>=36&&type<=43||type===49||type===50||type>=130&&type<=135)||(index>>9)===13&&type===30;if(wing)for(const mesh of model.meshes)if(/\.tga$/i.test(mesh.texture))for(let i=1;i<mesh.uv.length;i+=2)mesh.uv[i]=1-mesh.uv[i];return model;}
