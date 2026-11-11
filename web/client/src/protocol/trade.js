// Trade packets for UPDATE603 (GameServer/Trade.h + Protocol.cpp).
const nameText=new TextDecoder('windows-1252');
const readName=(b,o)=>nameText.decode(b.slice(o,o+10)).split('\0')[0];

export function tradeRequestPacket(index){
 if(!Number.isInteger(index)||index<0||index>0xffff)throw Error('Nhân vật giao dịch không hợp lệ.');
 return Uint8Array.of(0xc3,5,0x36,index>>8&255,index&255);
}
export function tradeAnswerPacket(accept){
 return Uint8Array.of(0xc1,4,0x37,accept?1:0);
}
export function tradeMoneyPacket(money){
 if(!Number.isInteger(money)||money<0||money>1000000000)throw Error('Zen giao dịch không hợp lệ.');
 const b=new Uint8Array(8);b.set([0xc1,8,0x3a,0]);new DataView(b.buffer).setUint32(4,money,true);return b;
}
export function tradeConfirmPacket(flag){
 return Uint8Array.of(0xc3,4,0x3c,flag?1:0);
}
export function tradeExitPacket(){return Uint8Array.of(0xc3,3,0x3d);}

export function decodeTradeRequest(b){
 if(!(b instanceof Uint8Array)||b.length!==13||![0xc1,0xc3].includes(b[0])||b[2]!==0x36)return null;
 return {name:readName(b,3)};
}
export function decodeTradeResponse(b){
 if(!(b instanceof Uint8Array)||b.length!==20||![0xc1,0xc3].includes(b[0])||b[2]!==0x37)return null;
 return {response:b[3],name:readName(b,4),level:new DataView(b.buffer,b.byteOffset,b.byteLength).getUint16(14,true),guild:new DataView(b.buffer,b.byteOffset,b.byteLength).getUint32(16,true)};
}
export function decodeTradeItemAdd(b){
 if(!(b instanceof Uint8Array)||b.length!==16||![0xc1,0xc3].includes(b[0])||b[2]!==0x39)return null;
 return {slot:b[3],wire:[...b.slice(4,16)]};
}
export function decodeTradeItemDel(b){
 if(!(b instanceof Uint8Array)||b.length!==4||![0xc1,0xc3].includes(b[0])||b[2]!==0x38)return null;
 return {slot:b[3]};
}
export function decodeTradeMoney(b){
 if(!(b instanceof Uint8Array)||b.length<8||![0xc1,0xc3].includes(b[0])||b[2]!==0x3b)return null;
 return {money:new DataView(b.buffer,b.byteOffset,b.byteLength).getUint32(4,true)};
}
export function decodeTradeConfirm(b){
 if(!(b instanceof Uint8Array)||b.length!==4||![0xc1,0xc3].includes(b[0])||b[2]!==0x3c)return null;
 return {flag:b[3]};
}
export function decodeTradeResult(b){
 if(!(b instanceof Uint8Array)||b.length!==4||![0xc1,0xc3].includes(b[0])||b[2]!==0x3d)return null;
 return {result:b[3]};
}
export function tradeMovePacket(sourceSlot,targetSlot,wire){
 if(!Number.isInteger(sourceSlot)||sourceSlot<0||sourceSlot>=76)throw Error('Ô nguồn giao dịch không hợp lệ.');
 if(!Number.isInteger(targetSlot)||targetSlot<0||targetSlot>=32)throw Error('Ô trade không hợp lệ.');
 if(!(wire instanceof Array)||wire.length!==12)throw Error('Item trade không hợp lệ.');
 const b=new Uint8Array(19);b.set([0xc3,19,0x24,0,sourceSlot]);b.set(wire,5);b[17]=1;b[18]=targetSlot;return b;
}
export function tradeReturnPacket(sourceSlot,targetSlot,wire){
 if(!Number.isInteger(sourceSlot)||sourceSlot<0||sourceSlot>=32)throw Error('Ô trade không hợp lệ.');
 if(!Number.isInteger(targetSlot)||targetSlot<0||targetSlot>=76)throw Error('Ô túi không hợp lệ.');
 if(!(wire instanceof Array)||wire.length!==12)throw Error('Item trade không hợp lệ.');
 const b=new Uint8Array(19);b.set([0xc3,19,0x24,1,sourceSlot]);b.set(wire,5);b[17]=0;b[18]=targetSlot;return b;
}
