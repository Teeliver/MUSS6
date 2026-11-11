import {itemSetData} from './item-set-data.js';
import {itemClassAllowed,requirementAttributeIndex} from './item-class-policy.js';
import {itemRequirements} from './item-visual-policy.js';
export function setOptionText(code,value,text){const id=code<=6?950+code:code===7?632:code<=20?949+code:code===21?970:code===22?984:code===23?983:code<=35?971+code-24:undefined;return id===undefined?'':(text(id)||'').replace(/%[diu]/g,String(value)).replace(/%%/g,'%');}
// CSItemOption::CheckItemSetOptions + calcSetOptionList. Display only; server owns combat stats.
export function equippedSetOptions(items,catalog,attributes,tier,level){const groups=new Map(),equipped=items.filter(i=>i.slot<12),eligible=i=>{const m=catalog[i.index];return i.durability>0&&m&&itemClassAllowed(m,attributes.baseClass,tier)&&itemRequirements(i,m).every((r,n)=>r<=(n===5?level:(attributes.base?.[requirementAttributeIndex(n)]??0)+(attributes.bonus?.[requirementAttributeIndex(n)]??0)));};
 for(const i of equipped){if([7,8].includes(i.slot)||!eligible(i))continue;const kind=(i.setOption&3)-1,id=itemSetData.types[i.index]?.[kind],s=itemSetData.sets[id];if(!s)continue;const peer=i.slot===1?0:i.slot===11?10:undefined;if(peer!==undefined&&equipped.some(p=>p.slot===peer&&p.index===i.index&&(p.setOption&3)===(i.setOption&3)))continue;const g=groups.get(id)??{id,name:s.name,count:0,options:[]};g.count++;groups.set(id,g);}
 for(const g of groups.values()){const s=itemSetData.sets[g.id],count=g.count-1,cls=s.classes[attributes.baseClass],classAllowed=cls>0&&cls<=Math.min(1,tier)+1,add=([code,value])=>{if(code===255||code>=24&&!classAllowed)return;g.options.push({code,value,active:code<24||classAllowed});};if(count<1)continue;for(let j=0;j<Math.min(count,6);j++)s.standard[j].forEach(add);if(count>=s.count-2)s.full.forEach(add);}
 return [...groups.values()].filter(g=>g.options.length);
}
