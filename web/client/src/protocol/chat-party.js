// Chat/whisper/party packets for UPDATE603 (Main5.2 source of truth).
const latin1=new TextDecoder('latin1'),utf8=new TextDecoder('utf-8'),encoder=new TextEncoder();
const ascii=s=>[...s].every(ch=>ch.charCodeAt(0)<128);
const readName=(b,offset,length)=>latin1.decode(b.slice(offset,offset+length)).split('\0')[0];
const readMessage=(b,offset,length)=>utf8.decode(b.slice(offset,offset+length)).split('\0')[0];
export const CHAT_MESSAGE_MAX_BYTES=59;
export function chatMessageBytes(value){return encoder.encode(String(value??'')).length;}

const reserved=new Set(['warp','move','trade','exchange','whisper','whisper on','whisper off','post','party']);
export function parseChatInput(value){
 const text=String(value??'').trim();
 if(text.startsWith('/')){
  const match=/^\/([^\s]{1,10})\s+([\s\S]+)$/.exec(text);
  if(match&&!reserved.has(match[1].toLowerCase()))return {kind:'whisper',name:match[1],message:match[2].trim()};
 }
 return {kind:'chat',message:text};
}
export function chatPacket(name,message){
 if(typeof name!=='string'||!name.length||name.length>10||!ascii(name))throw Error('Tên nhân vật tối đa 10 ký tự Latin.');
 if(typeof message!=='string'||!message.length)throw Error('Nội dung chat không được rỗng.');
 if(chatMessageBytes(message)>CHAT_MESSAGE_MAX_BYTES)throw Error('Nội dung chat tối đa 59 byte UTF-8 (khoảng 19–29 ký tự tiếng Việt).');
 return packet(0x00,name,message);
}

export function whisperPacket(name,message){
 if(typeof name!=='string'||!name.length||name.length>10||!ascii(name))throw Error('Tên whisper tối đa 10 ký tự Latin.');
 if(typeof message!=='string'||!message.length)throw Error('Nội dung whisper không được rỗng.');
 if(chatMessageBytes(message)>CHAT_MESSAGE_MAX_BYTES)throw Error('Nội dung whisper tối đa 59 byte UTF-8 (khoảng 19–29 ký tự tiếng Việt).');
 return packet(0x02,name,message);
}

function packet(head,name,message){
 const nameBytes=[...name].map(ch=>ch.charCodeAt(0)),messageBytes=encoder.encode(message),bytes=new Uint8Array(3+10+messageBytes.length+1);
 bytes.set([0xc1,bytes.length,head],0);bytes.set(nameBytes,3);bytes.set(messageBytes,13);bytes[13+messageBytes.length]=0;
 return bytes;
}

export function partyInvitePacket(index){
 if(!Number.isInteger(index)||index<0||index>0xffff)throw Error('Nhân vật mời nhóm không hợp lệ.');
 return Uint8Array.of(0xc3,5,0x40,index>>8&255,index&255);
}

export function partyAnswerPacket(result,key){
 if(![0,1].includes(result)||!Number.isInteger(key)||key<0||key>0xffff)throw Error('Phản hồi nhóm không hợp lệ.');
 return Uint8Array.of(0xc3,6,0x41,result,key>>8&255,key&255);
}

export function partyListPacket(){return Uint8Array.of(0xc1,3,0x42);}
export function partyLeavePacket(number){
 if(!Number.isInteger(number)||number<0||number>=5)throw Error('Số thứ tự thành viên nhóm không hợp lệ.');
 return Uint8Array.of(0xc1,4,0x43,number);
}

const header=b=>{const wide=b[0]===0xc2||b[0]===0xc4,off=wide?3:2;return {wide,off,head:b[off],start:off+1};};

export function decodeChatPacket(b){
 if(!(b instanceof Uint8Array)||b.length<3)return null;
 const h=header(b);
 if(h.head===0x00||h.head===0x02){
  if(b.length<h.start+10)throw Error('Invalid chat packet');
  return {kind:h.head===0x02?'whisper':'chat',name:readName(b,h.start,10),message:readMessage(b,h.start+10,Math.min(60,b.length-h.start-10))};
 }
 if(h.head===0x0c&&!h.wide)return {kind:'serverMsg',number:b[3]};
 return null;
}

export function decodePartyPacket(b){
 if(!(b instanceof Uint8Array)||b.length<3||![0xc1,0xc3].includes(b[0]))return null;
 const h={off:2,head:b[2],start:3};
 if(h.head===0x40)return {kind:'request',index:b[h.off+1]<<8|b[h.off+2]};
 if(h.head===0x41)return {kind:'result',result:b[h.off+1]};
 if(h.head===0x42){
  if(b.length<5)throw Error('Invalid party list');
  const count=b[h.off+2],items=[],base=h.off+3,rem=b.length-base,size=count?rem/count:0;
  if(count&&![22,24,36].includes(size))throw Error('Invalid party list size');
  for(let i=0;i<count;i++){
   const p=base+i*size,d=new DataView(b.buffer,b.byteOffset,b.byteLength),hpOff=size===22?14:16,maxOff=size===22?18:20;
   items.push({name:readName(b,p,10),number:b[p+10],map:b[p+11],x:b[p+12],y:b[p+13],hp:d.getUint32(p+hpOff,true),maxHp:d.getUint32(p+maxOff,true)});
  }
  return {kind:'list',items};
 }
 if(h.head===0x43)return {kind:'delMember'};
 if(h.head===0x44){
  if(b.length<4)throw Error('Invalid party life');
  const count=b[h.off+1],items=[];
  if(b.length!==4+count)throw Error('Invalid party life size');
  for(let i=0;i<count;i++)items.push({number:(b[4+i]>>4)&15,step:b[4+i]&15});
  return {kind:'life',items};
 }
 return null;
}
