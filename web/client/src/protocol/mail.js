// Friend mail packets for UPDATE603 (GameServer/Friend.h).
const encoder=new TextEncoder();
const text=new TextDecoder('windows-1252');const utf8=new TextDecoder();
const need=(ok,message)=>{if(!ok)throw Error(message);};
const fixed=(value,size)=>{const out=new Uint8Array(size);out.set(encoder.encode(String(value??'')).slice(0,size));return out;};
const readName=(b,o)=>text.decode(b.slice(o,o+10)).split('\0')[0];
const readFixed=(b,o,n)=>new TextDecoder().decode(b.slice(o,o+n)).split('\0')[0];
export function mailListPacket(){return Uint8Array.of(0xc1,3,0xc9);}
export function mailReadPacket(index){const b=new Uint8Array(6);b.set([0xc1,6,0xc7,0]);new DataView(b.buffer).setUint16(4,index,true);return b;}
export function mailDeletePacket(index){const b=new Uint8Array(6);b.set([0xc1,6,0xc8,0]);new DataView(b.buffer).setUint16(4,index,true);return b;}
export function mailSendPacket({window=0,name,title,body,photoDir=0,photoAction=0}){
  const mail=encoder.encode(String(body??''));need(mail.length<=1000,'Thư tối đa 1000 byte.');need(/^[A-Za-z0-9]{1,10}$/.test(String(name??'')),'Tên người nhận 1–10 ký tự Latin.');
  const titleBytes=encoder.encode(String(title??''));need(titleBytes.length<=59,'Tiêu đề tối đa 59 byte UTF-8.');need(Number.isInteger(photoDir)&&photoDir>=0&&photoDir<=255&&Number.isInteger(photoAction)&&photoAction>=0&&photoAction<=255,'Thông số ảnh không hợp lệ.');
  const size=4+4+10+60+1+1+2+mail.length,b=new Uint8Array(size);
  b.set([0xc4,size>>8&255,size&255,0xc5]);
  new DataView(b.buffer).setUint32(4,window>>>0,true);
  b.set(fixed(name,10),8);b.set(fixed(title,60),18);b[78]=photoDir;b[79]=photoAction;
  new DataView(b.buffer).setUint16(80,mail.length,true);b.set(mail,82);
  return b;
}
export function decodeMailResult(b){
  if(!(b instanceof Uint8Array)||b.length!==8||![0xc1,0xc3].includes(b[0])||b[2]!==0xc5)return null;
  return {result:b[3],window:new DataView(b.buffer,b.byteOffset,b.byteLength).getUint32(4,true)};
}
export function decodeMailAlert(b){
  if(!(b instanceof Uint8Array)||b.length<107||![0xc1,0xc3].includes(b[0])||b[2]!==0xc6)return null;
  const d=new DataView(b.buffer,b.byteOffset,b.byteLength);
  return {index:d.getUint16(4,true),name:readName(b,6),date:readFixed(b,16,30),title:readFixed(b,46,60),read:b[106]};
}
export function decodeMailText(b){
  if(!(b instanceof Uint8Array)||b.length<28||![0xc1,0xc2,0xc3].includes(b[0]))return null;
  const head=b[0]===0xc2?3:2;if(b[head]!==0xc7)return null;
  const d=new DataView(b.buffer,b.byteOffset,b.byteLength),size=d.getUint16(6,true);
  if(size>1000||b.length<28+size)throw Error('Invalid mail text size');
  return {index:d.getUint16(4,true),size,photoDir:b[26],photoAction:b[27],text:utf8.decode(b.slice(28,28+size)).split('\0')[0]};
}
export function decodeMailDelete(b){
  if(!(b instanceof Uint8Array)||b.length!==6||![0xc1,0xc3].includes(b[0])||b[2]!==0xc8)return null;
  return {result:b[3],index:new DataView(b.buffer,b.byteOffset,b.byteLength).getUint16(4,true)};
}
