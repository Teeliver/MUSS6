const need=(v)=>{if(!v)throw Error('Invalid skill/item action');},byte=n=>Number.isInteger(n)&&n>=0&&n<=255,id=n=>Number.isInteger(n)&&n>0&&n<=65535,target=n=>Number.isInteger(n)&&n>=0&&n<0x4000;
export const multiSkills=new Set([5,8,9,10,12,13,14,24,47,49,55,56,65,74]);
export function itemUsePacket(slot,target=0){need(Number.isInteger(slot)&&slot>=12&&slot<204,'Invalid item slot');need(Number.isInteger(target)&&target>=0&&target<204,'Invalid item target');return Uint8Array.of(195,6,38,slot,target,0);}
export function skillPacket(skill,index){need(id(skill)&&target(index));return Uint8Array.of(195,8,25,skill>>8,skill&255,index>>8,index&255,0);}
export function durationSkillPacket(skill,x,y,angle,serial=0,index=65535){need(id(skill)&&[x,y,angle,serial].every(byte)&&(index===65535||target(index)));return Uint8Array.of(195,13,30,skill>>8,skill&255,x,y,angle,0,0,index>>8,index&255,serial);}
export function multiSkillPacket(skill,x,y,serial,targets){need(id(skill)&&[x,y,serial].every(byte)&&Array.isArray(targets)&&targets.length>0&&targets.length<=5&&targets.every(target)&&new Set(targets).size===targets.length);return Uint8Array.of(195,9+targets.length*3,0xDB,skill>>8,skill&255,x,y,serial,targets.length,...targets.flatMap(i=>[i>>8,i&255,0]));}
export const selfSkills=new Set([16,18,26,27,28,48,64]);
export function skillTarget(id){return id===6?'ground':selfSkills.has(id)?'self':[8,9,10,12,14].includes(id)?'area':'enemy';}
export function skillTeleportPacket(x,y){need(byte(x)&&byte(y));return Uint8Array.of(195,8,28,0,0,0,x,y);}

export function operatePacket(kind,direction){const action={sit:128,lean:129,healing:130}[kind];need(action!==undefined&&Number.isInteger(direction)&&direction>=0&&direction<8);return Uint8Array.of(193,7,24,direction,action,0,0);}
