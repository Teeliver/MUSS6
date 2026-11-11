// Direct jewel use on an item (GameServer/ItemManager.cpp CGItemUseRecv).
// Source C1:26 carries SourceSlot (jewel), TargetSlot (item) and type 0.
export const JEWEL_OF_BLESS=14*512+13;
export const JEWEL_OF_SOUL=14*512+14;
export const JEWEL_OF_LIFE=14*512+16;
export const JEWEL_OF_HARMONY=14*512+42;
export const SMELTING_STONE_LOW=14*512+43;
export const SMELTING_STONE_HIGH=14*512+44;
export const JEWEL_OF_EXTENSION=14*512+160;
export const JEWEL_OF_ELEVATION=14*512+161;
const actions=new Map([
 [JEWEL_OF_BLESS,'bless'],
 [JEWEL_OF_SOUL,'soul'],
 [JEWEL_OF_LIFE,'life'],
 [JEWEL_OF_HARMONY,'harmony'],
 [SMELTING_STONE_LOW,'smelt1'],
 [SMELTING_STONE_HIGH,'smelt2'],
 [JEWEL_OF_EXTENSION,'extension'],
 [JEWEL_OF_ELEVATION,'elevation'],
]);
export function jewelAction(index){return actions.get(index);}
function targetAllowed(action,target){
 if(!target)return false;
 if(target.periodic)return false;
 if(!Number.isInteger(target.index))return true;
 const group=target.index>>9,type=target.index&511;
 if(group===12&&type>6){
  const wing=(type>=36&&type<=43)||type===49||type===50||(type>=262&&type<=265);
  return wing;
 }
 if(group===13)return type===30||type===37;
 return true;
}
// Jewels are only dragged onto another item; caps/restrictions follow the server checks.
export function jewelCanUse(action,target){
 if(!target)return false;
 if(!targetAllowed(action,target))return false;
 if(action==='bless')return (target.level??0)<6;
 if(action==='soul')return (target.level??0)<9;
 return true;
}
export function jewelActionText(action){
 return {
  bless:'Jewel of Bless: đập tăng cấp (tối đa +6).',
  soul:'Jewel of Soul: đập tăng cấp (tối đa +9, có thể tụt cấp).',
  life:'Jewel of Life: ép thêm option.',
  harmony:'Jewel of Harmony: ép Harmony option.',
  smelt1:'Smelting Stone: tạo option Harmony.',
  smelt2:'Smelting Stone cấp cao: tạo option Harmony.',
  extension:'Jewel of Extension: mở rộng Lucky Item.',
  elevation:'Jewel of Elevation: nâng Lucky Item.',
 }[action]??'';
}
