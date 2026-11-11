// EX603 ENG: GameServer/Protocol.h and native/client/client.cpp path_packet.
export const roadX=Object.freeze([-1,0,1,1,1,0,-1,-1]);
export const roadY=Object.freeze([-1,-1,-1,0,1,1,1,0]);
const tile=n=>Number.isInteger(n)&&n>=0&&n<256;
export function pathPacket(x,y,directions){
 if(!tile(x)||!tile(y)||!Array.isArray(directions)||directions.length<1||directions.length>14)throw new Error('Invalid movement path');
 const bytes=new Uint8Array(13);bytes.set([0xc1,13,0xd4,x,y]);
 bytes[5]=(directions[0]<<4)|directions.length;
 for(let i=0;i<directions.length;i++){
  const d=directions[i];if(!Number.isInteger(d)||d<0||d>7)throw new Error('Invalid movement direction');
  x+=roadX[d];y+=roadY[d];if(!tile(x)||!tile(y))throw new Error('Path leaves map');
  bytes[6+(i>>1)]|=d<<(i%2?0:4);
 }
 return {bytes,x,y,direction:directions[0]};
}
export function movementReply(bytes){
 if(!(bytes instanceof Uint8Array)||![0xd4,0x15].includes(bytes[2]))throw new Error('Invalid movement reply');
 const correction=bytes[2]===0x15,size=correction?7:8;
 if(bytes.length!==size||bytes[0]!==0xc1||bytes[1]!==size||(!correction&&(bytes[7]&15)!==0)||(!correction&&(bytes[7]>>4)>7))throw new Error('Invalid movement reply layout');
 return {index:bytes[3]*256+bytes[4],x:bytes[5],y:bytes[6],...(correction?{}:{direction:bytes[7]>>4}),correction};
}
