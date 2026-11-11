// Guild packets for UPDATE603. Source: Guild.h / Guild.cpp and Main5.2 WSclient.cpp.
const latin1=new TextDecoder('latin1');
const readName=(b,offset,length)=>latin1.decode(b.slice(offset,offset+length)).split('\0')[0];
const check=(ok,message)=>{if(!ok)throw Error(message);};
export const GUILD_STATUS={MEMBER:0,BATTLE_MASTER:32,OFFICE_MASTER:64,MASTER:128};
export const guildListRequestPacket=()=>Uint8Array.of(0xc1,3,0x52);
export function guildViewportRequestPacket(key){
 check(Number.isInteger(key)&&key>=0&&key<=0xffffffff,'Invalid guild key');
 const b=new Uint8Array(8);b.set([0xc1,8,0x66,0]);new DataView(b.buffer).setUint32(4,key,true);return b;
}
export function decodeGuildViewport(b){
 if(!(b instanceof Uint8Array)||b.length<5||b[0]!==0xc2||b[3]!==0x65)return undefined;
 const count=b[4];check(b.length===5+count*12,'Invalid guild viewport size');
 const entries=[];
 for(let i=0;i<count;i++){
  const p=5+i*12,d=new DataView(b.buffer,b.byteOffset,b.byteLength);
  entries.push({key:d.getUint32(p,true),status:b[p+4],type:b[p+5],relationship:b[p+6],index:((b[p+7]&0x7f)<<8)|b[p+8],owner:b[p+9]});
 }
 return {entries};
}
export function decodeGuildInfo(b){
 if(!(b instanceof Uint8Array)||b[2]!==0x66)return undefined;
 check(b[0]===0xc1&&b.length>=57,'Invalid guild info size');
 const d=new DataView(b.buffer,b.byteOffset,b.byteLength);
 return {key:d.getUint32(4,true),type:b[8],unionName:readName(b,9,8),name:readName(b,17,8),mark:[...b.slice(25,57)]};
}
export function decodeGuildList(b){
 if(!(b instanceof Uint8Array)||b[3]!==0x52)return undefined;
 check(b[0]===0xc2&&b.length>=24,'Invalid guild list size');
 const result=b[4],count=b[5],d=new DataView(b.buffer,b.byteOffset,b.byteLength);
 check(b.length===24+count*13,'Invalid guild member list size');
 const members=[];
 for(let i=0;i<count;i++){
  const p=24+i*13,connected=b[p+11],rawServer=connected&0x7f,online=connected!==0x7f&&connected!==0xff&&(connected&0x80)!==0;
  members.push({name:readName(b,p,10),number:b[p+10],connected,status:b[p+12],server:online?rawServer:-1,online});
 }
 return {result,count,totalScore:d.getUint32(8,true),score:b[12],rival:readName(b,13,9),members};
}
export function guildStatusTextId(status){return status===GUILD_STATUS.MASTER?1300:status===GUILD_STATUS.OFFICE_MASTER?1301:status===GUILD_STATUS.BATTLE_MASTER?1302:0;}
export function guildStatusBackground(status){return status===GUILD_STATUS.MASTER?'rgba(255,100,50,.5)':status===GUILD_STATUS.OFFICE_MASTER?'rgba(255,150,80,.5)':status===GUILD_STATUS.BATTLE_MASTER?'rgba(255,200,100,.5)':undefined;}
export const guildUnionListRequestPacket=()=>Uint8Array.of(0xc1,3,0xe9);
export function decodeGuildUnionList(b){
 if(!(b instanceof Uint8Array)||b.length<8||b[0]!==0xc2||b[3]!==0xe9)return undefined;
 const count=b[4];check(b.length===8+count*41,'Invalid guild union list size');
 const members=[];
 for(let i=0;i<count;i++){const p=8+i*41;members.push({count:b[p],mark:[...b.slice(p+1,p+33)],name:readName(b,p+33,8)});}
 return {count,result:b[5],unionNumber:b[6],rivalNumber:b[7],members};
}
export function decodeGuildCreateResult(b){if(!(b instanceof Uint8Array)||b[0]!==0xc1||b[2]!==0x56)return undefined;check(b.length===5,'Invalid guild create result');return {result:b[3],type:b[4]};}
export function guildLeavePacket(name){check(typeof name==='string'&&/^[A-Za-z0-9_]{1,10}$/.test(name),'Invalid guild leave name');const b=new Uint8Array(23);b.set([0xc1,23,0x53]);for(let i=0;i<name.length;i++)b[3+i]=name.charCodeAt(i);return b;}
export function decodeGuildDeleteResult(b){if(!(b instanceof Uint8Array)||b[0]!==0xc1||b[2]!==0x53)return undefined;check(b.length===4,'Invalid guild delete result');return {result:b[3]};}
