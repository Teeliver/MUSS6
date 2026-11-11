// Item table: STR, DEX, ENERGY, VITALITY, COMMAND, LEVEL.
// HUD attributes: STR, DEX, VITALITY, ENERGY, COMMAND.
export const requirementAttributeIndex=i=>[0,1,3,2,4,5][i];
export function itemClassAllowed(meta,baseClass,tier){if(!Number.isInteger(baseClass)||baseClass<0||baseClass>6||!Number.isInteger(tier)||tier<0||tier>2)return false;const required=meta.classes?.[baseClass];return Number.isInteger(required)&&required>=1&&required<=tier+1;}
// IsRequireClassRenderItem. These exclusions suppress tooltip class lines only;
// they do not grant permission to equip an otherwise incompatible item.
export function showsClassRequirement(index){const g=index>>9,t=index&511;
 if(g===12&&[15,26,30,31,32,33,34].includes(t))return false;
 if(g===13&&([10,19,29,37,59,60,61,64,65,67,68,69,70,76,80,81,82,93,94,106,121,122].includes(t)||t>=39&&t<=48||t>=54&&t<=58||t>=62&&t<=63||t>=71&&t<=75||t>=83&&t<=93||t>=125&&t<=127))return false;
 if(g===14&&([53,54,91,94,95,96,133].includes(t)||t>=58&&t<=62||t>=70&&t<=82||t>=92&&t<=93||t>=97&&t<=98||t>=145&&t<=150))return false;return true;
}
const names=[[20,25,1669],[21,26,1668],[22,27,1670],[23,2305,1671],[24,2305,1672],[1687,1688,1689],[3150,2305,3151]];
export function itemClassLines(item,meta,{baseClass,tier=0,texts}){if(!showsClassRequirement(item.index)||meta.classes?.every(n=>n===1))return [];return (meta.classes??[]).flatMap((required,cls)=>{const id=names[cls]?.[required-1],name=texts.get(id);if(!required||!name)return [];const allowed=cls===baseClass&&required<=tier+1;return [{text:(texts.get(61)||'Can be equipped by %s').replace('%s',name),color:'#ffffff',background:allowed?undefined:'#a00000',bold:false}];});}
