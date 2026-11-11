const allowed=new Set([0,1,2,3,4,5,6,7,8,9,10,20,35,36,37,38,39,40,46,47,48,49,50,70,71,78,79,80,81,82,94,85,86,87,133]);
export const canQuickItem=index=>Number.isInteger(index)&&(index>>9)===14&&allowed.has(index%512);
export function quickItems(items,binding){if(!binding)return [];const n=binding.index%512;const range=n<=3?[0,3]:n<=6?[4,6]:n>=35&&n<=37?[35,37]:n>=38&&n<=40?[38,40]:[n,n];return items.filter(i=>i.slot>=12&&i.slot<204&&(i.index>>9)===14&&i.index%512>=range[0]&&i.index%512<=range[1]&&(range[0]!==range[1]||(i.level??0)===(binding.level??0))&&i.durability>0).sort((a,b)=>b.index-a.index||b.slot-a.slot);}
export function bindSkill(keys,key,skill){return Object.fromEntries([...Object.entries(keys).filter(([k,v])=>String(k)!==String(key)&&v!==skill),[key,skill]]);}
