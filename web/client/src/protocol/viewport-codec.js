import {monsterHealthPacket} from './monster-health.js';
// GameServer/Viewport.h UPDATE603; normalized plaintext C1/C2.
export function createViewport(){
 const entities=new Map(),health=new Map(),id=(a,b)=>(a*256+b)&0x3fff;
 const check=(ok,message)=>{if(!ok)throw new Error(message);};
 return {clear(){entities.clear();health.clear();},snapshot(){return [...entities.values()].map(e=>({...e,effects:[...e.effects],...(e.health?{health:{...e.health}}:{}),...(e.charset?{charset:[...e.charset],equipment:(e.equipment??[]).map(i=>({...i}))}:{})}));},
 apply(b,self){
  check(b instanceof Uint8Array&&b.length>=3,'Invalid viewport input');const wide=b[0]===0xc2,off=wide?3:2;
  const hp=monsterHealthPacket(b);if(hp){health.clear();for(const e of entities.values())delete e.health;for(const row of hp){health.set(row.index,row);const e=entities.get(row.index);if(e)e.health={...row};}return true;}
  const head=b[off];if(!wide&&head===0x25){check(b[0]===0xc1&&b.length===17&&b[1]===17,'Invalid player item change');const slot=b[6]>>4,index=b[5]|((b[8]&128)<<1)|((b[10]&240)<<5);check(slot<12,'Invalid player equipment slot');const entity=entities.get(id(b[3],b[4]));if(entity?.kind==='player'){entity.equipment=(entity.equipment??[]).filter(i=>i.slot!==slot);entity.equipment.push({slot,index,level:[0,3,5,7,9,11,13,15][b[6]&15]??0,excellent:b[8]&63,setOption:b[9]});}return true;}
if(!wide&&head===0xf3&&b[3]===0x13){check(b[0]===0xc1&&b.length===24&&b[1]===24,'Invalid player equipment');const entity=entities.get(id(b[4],b[5]));if(entity?.kind==='player'){entity.charset=[...b.slice(6,24)];entity.equipment=[];}return true;}
if(!wide&&head===0x18){check(b.length===9&&b[1]===9&&b[5]<8,'Invalid viewport action');const e=entities.get(id(b[3],b[4]));if(e?.kind==='player'){e.restKind=({128:'sit',129:'lean',130:'healing'})[b[6]];e.direction=b[5];}return true;}
if(![0x12,0x13,0x14,0xd4,0x15,0x17].includes(head))return false;
  check((wide||b[0]===0xc1)&&(wide?b[1]*256+b[2]:b[1])===b.length,'Viewport frame mismatch');
  if(head===0x12){check(wide&&b.length>=5,'Invalid player spawn header');const next=[],seen=new Set();let p=5;
   for(let i=0;i<b[4];i++){check(p+36<=b.length,'Truncated player spawn');const index=id(b[p],b[p+1]),count=b[p+35];check(p+36+count<=b.length,'Truncated player effects');check(!seen.has(index),'Duplicate viewport index');seen.add(index);if(index!==self)next.push({index,kind:'player',restKind:({2:'sit',3:'lean',4:'healing'})[b[p+4]&7],charset:[...b.slice(p+4,p+22)],x:b[p+2],y:b[p+3],name:new TextDecoder('windows-1252').decode(b.slice(p+22,p+32)).split('\0')[0],tx:b[p+32],ty:b[p+33],direction:b[p+34]>>4,pkLevel:b[p+34]&15,effects:[...b.slice(p+36,p+36+count)],dead:false});p+=36+count;}
   check(p===b.length,'Trailing player spawn bytes');check(entities.size+next.filter(e=>!entities.has(e.index)).length<=1024,'Viewport entity cap');for(const e of next){e.health=health.get(e.index);entities.set(e.index,e);}return true;
  }
  if(head===0x13){check(wide&&b.length>=5,'Invalid NPC spawn header');const next=[],seen=new Set();let p=5;
   for(let i=0;i<b[4];i++){check(p+10<=b.length,'Truncated NPC spawn');const index=id(b[p],b[p+1]),count=b[p+9];check(p+10+count<=b.length,'Truncated viewport effects');check(!seen.has(index),'Duplicate viewport index');seen.add(index);if(index!==self)next.push({index,type:(b[p+2]*256+b[p+3])&0xfff,x:b[p+4],y:b[p+5],tx:b[p+6],ty:b[p+7],direction:b[p+8]>>4,effects:[...b.slice(p+10,p+10+count)],dead:false});p+=10+count;}
   check(p===b.length,'Trailing NPC spawn bytes');check(entities.size+next.filter(e=>!entities.has(e.index)).length<=1024,'Viewport entity cap');for(const e of next){e.health=health.get(e.index);entities.set(e.index,e);}return true;
  }
  if(head===0x14){check(!wide&&b.length>=4&&b.length===4+b[3]*2,'Invalid entity destroy');for(let p=4;p<b.length;p+=2){const key=id(b[p],b[p+1]);entities.delete(key);health.delete(key);}return true;}
  const size=head===0xd4?8:head===0x15?7:9;check(!wide&&b.length===size,'Invalid entity update');const e=entities.get(id(b[3],b[4]));if(!e||e.index===self)return true;
  if(head===0x17){e.dead=true;delete e.health;health.delete(e.index);}else{e.restKind=undefined;e.x=e.tx=b[5];e.y=e.ty=b[6];if(head===0xd4){check((b[7]>>4)<8,'Invalid entity direction');e.direction=b[7]>>4;}}return true;
 }
 };
}
