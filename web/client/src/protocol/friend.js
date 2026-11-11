// Friend packets for UPDATE603 (GameServer/Friend.h).
const text=new TextDecoder('windows-1252');
const readName=(b,o)=>text.decode(b.slice(o,o+10)).split('\0')[0];
export function friendListPacket(){return Uint8Array.of(0xc1,3,0xc0);}
export function friendAddPacket(name){const b=new Uint8Array(13);b.set([0xc1,13,0xc1]);b.set(ascii(name,10),3);return b;}
export function friendAcceptPacket(result,name){const b=new Uint8Array(14);b.set([0xc1,14,0xc2,result?1:0]);b.set(ascii(name,10),4);return b;}
export function friendDeletePacket(name){const b=new Uint8Array(13);b.set([0xc1,13,0xc3]);b.set(ascii(name,10),3);return b;}
function ascii(s,n){const o=new Uint8Array(n);o.set(new TextEncoder().encode(String(s??'')).slice(0,n));return o;}
export function decodeFriendList(b){
 if(!(b instanceof Uint8Array)||b.length<6||b[0]!==0xc2||b[3]!==0xc0)return null;
 const mailCount=b[4],mailTotal=b[5],count=b[6],base=7;
 if(b.length!==base+count*11)throw Error('Invalid friend list size');
 const list=[];for(let i=0;i<count;i++)list.push({name:readName(b,base+i*11),server:b[base+i*11+10]});
 return {mailCount,mailTotal,count,list};
}
export function decodeFriendRequest(b){
 if(!(b instanceof Uint8Array)||b.length!==15||![0xc1,0xc3].includes(b[0])||b[2]!==0xc1)return null;
 return {result:b[3],name:readName(b,4),server:b[14]};
}
export function decodeFriendAcceptRequest(b){
 if(!(b instanceof Uint8Array)||b.length!==13||![0xc1,0xc3].includes(b[0])||b[2]!==0xc2)return null;
 return {name:readName(b,3)};
}
export function decodeFriendDelete(b){
 if(!(b instanceof Uint8Array)||b.length!==14||![0xc1,0xc3].includes(b[0])||b[2]!==0xc3)return null;
 return {result:b[3],name:readName(b,4)};
}
export function decodeFriendState(b){
 if(!(b instanceof Uint8Array)||b.length!==14||![0xc1,0xc3].includes(b[0])||b[2]!==0xc4)return null;
 return {name:readName(b,3),server:b[13]};
}
