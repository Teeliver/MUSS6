import {hudLayout as L} from './hud-layout.js';
// Personal shop packets: GameServer/PersonalShop.h (UPDATE603 layout).
// Client -> server uses C3:3F subcode 05/06/07; server list is C2:3F 05/13.
const ascii=(text,size)=>{const out=new Uint8Array(size);const bytes=new TextEncoder().encode(String(text??''));out.set(bytes.slice(0,size));return out;};
const nameText=new TextDecoder('windows-1252'),titleText=new TextDecoder('utf-8');
const readName=(bytes,start,size)=>{const slice=bytes.slice(start,start+size);const end=slice.indexOf(0);return nameText.decode(end<0?slice:slice.slice(0,end));};
const readTitle=(bytes,start,size)=>{const slice=bytes.slice(start,start+size);const end=slice.indexOf(0);return titleText.decode(end<0?slice:slice.slice(0,end));};
export function personalShopOpenPacket(index,name){const b=new Uint8Array(16);b.set([0xc3,b.length,0x3f,0x05,index>>8&255,index&255]);b.set(ascii(name,10),6);return b;}
export function personalShopBuyPacket(index,name,slot){const b=new Uint8Array(17);b.set([0xc3,b.length,0x3f,0x06,index>>8&255,index&255]);b.set(ascii(name,10),6);b[16]=slot;return b;}
export function personalShopLeavePacket(index,name){const b=new Uint8Array(16);b.set([0xc3,b.length,0x3f,0x07,index>>8&255,index&255]);b.set(ascii(name,10),6);return b;}
export function decodePersonalShopList(b){
 if(!(b instanceof Uint8Array)||b.length<55||![0xc2,0xc4].includes(b[0])||b[3]!==0x3f||![0x05,0x13].includes(b[4]))throw Error('Invalid personal shop list');
 if((b[1]<<8|b[2])!==b.length)throw Error('Invalid personal shop list size');
 const layout=L.PMSG_PSHOP_ITEM_LIST,header=L.PMSG_PSHOP_ITEM_LIST_SEND,count=b[header.count],entrySize=layout.size;
 if(b.length!==header.size+count*entrySize)throw Error('Invalid personal shop item count');
 const items=[],d=new DataView(b.buffer,b.byteOffset,b.byteLength);
 for(let p=header.size;p<b.length;p+=entrySize){const wire=[...b.slice(p+layout.ItemInfo,p+layout.ItemInfo+12)];items.push({slot:b[p+layout.slot],wire,price:d.getUint32(p+layout.value,true)});}
 return {result:b[5],index:b[6]<<8|b[7],name:readName(b,8,10),title:readTitle(b,18,36),items};
}
export function decodePersonalShopViewport(b){
 if(!(b instanceof Uint8Array)||b.length<6||![0xc2,0xc4].includes(b[0])||b[3]!==0x3f||b[4]!==0x00)throw Error('Invalid personal shop viewport');
 if((b[1]<<8|b[2])!==b.length)throw Error('Invalid personal shop viewport size');
 const count=b[5];if(b.length!==6+count*38)throw Error('Invalid personal shop title count');
 const titles={};
 for(let p=6;p<b.length;p+=38)titles[b[p]<<8|b[p+1]]=readTitle(b,p+2,36);
 return titles;
}
export function decodePersonalShopBuy(b){
 if(!(b instanceof Uint8Array)||b.length!==20||![0xc1,0xc3].includes(b[0])||b[2]!==0x3f||b[3]!==0x06)throw Error('Invalid personal shop buy result');
 return {result:b[4],index:b[5]<<8|b[6],wire:[...b.slice(7,19)],slot:b[19]};
}
export function decodePersonalShopTitleChange(b){
 if(!(b instanceof Uint8Array))throw Error('Invalid personal shop title change');
 if(b[0]===0xc2){if(b.length!==55||b[3]!==0x3f||b[4]!==0x10)throw Error('Invalid personal shop title change');return {index:b[5]<<8|b[6],title:readTitle(b,7,36),name:readName(b,43,10)};}
 if(b.length!==52||![0xc1,0xc3].includes(b[0])||b[2]!==0x3f||b[3]!==0x10)throw Error('Invalid personal shop title change');
 return {index:b[4]<<8|b[5],title:readTitle(b,6,36),name:readName(b,42,10)};
}
export function decodePersonalShopLeave(b){
 if(!(b instanceof Uint8Array)||b.length!==6||![0xc1,0xc3].includes(b[0])||b[2]!==0x3f||b[3]!==0x12)throw Error('Invalid personal shop leave');
 return {index:b[4]<<8|b[5]};
}
