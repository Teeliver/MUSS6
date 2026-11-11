// GameServer UPDATE603 guild/duel command packets (Protocol.h / Guild.h / Duel.h).
const check=(ok,message)=>{if(!ok)throw new Error(message);};
const indexBytes=index=>[(index>>8)&255,index&255];
const nameBytes=(name,size=10)=>{const out=new Uint8Array(size),text=String(name??'');for(let i=0;i<size&&i<text.length;i++)out[i]=text.charCodeAt(i)&255;return out;};
const readName=b=>new TextDecoder('windows-1252').decode(b).split('\0')[0];

export function guildRequestPacket(index){
  check(Number.isInteger(index)&&index>=0&&index<0x4000,'Invalid guild target index');
  return Uint8Array.of(0xc1,5,0x50,...indexBytes(index));
}

export function guildAnswerPacket(result,index){
  check([0,1].includes(result)&&Number.isInteger(index)&&index>=0&&index<0x4000,'Invalid guild answer');
  return Uint8Array.of(0xc1,6,0x51,result,...indexBytes(index));
}

export function guildRelationPacket(relation,type,index){
  check([1,2].includes(relation)&&[1,2].includes(type),'Invalid guild relation');
  check(Number.isInteger(index)&&index>=0&&index<0x4000,'Invalid guild target index');
  return Uint8Array.of(0xc1,7,0xe5,relation,type,...indexBytes(index));
}

// Duel packets are listed with Encrypt=1 in HackPacketCheck.txt, so they must use C3.
export function duelStartPacket(index,name){
  check(Number.isInteger(index)&&index>=0&&index<0x4000,'Invalid duel target index');
  const packet=new Uint8Array(16);packet.set([0xc3,16,0xaa,0x01,...indexBytes(index)]);packet.set(nameBytes(name),6);return packet;
}

export function duelAnswerPacket(ok,index,name){
  check(Number.isInteger(index)&&index>=0&&index<0x4000,'Invalid duel target index');
  const packet=new Uint8Array(17);packet.set([0xc3,17,0xaa,0x02,ok?1:0,...indexBytes(index)]);packet.set(nameBytes(name),7);return packet;
}

export function duelEndPacket(){return Uint8Array.of(0xc3,4,0xaa,0x03);}

function guildRequest(b){
  if(b[0]!==0xc1||b[2]!==0x50)return null;
  check(b.length===5,'Invalid guild request size');
  return {kind:'guildRequest',index:(b[3]<<8)|b[4]};
}

function guildResult(b){
  if(b[0]!==0xc1||b[2]!==0x51)return null;
  check(b.length===4,'Invalid guild result size');
  return {kind:'guild',result:b[3]};
}

function guildUnion(b){
  if(b[0]!==0xc1||b[2]!==0xe6)return null;
  check(b.length===8,'Invalid guild union result size');
  return {kind:'union',relation:b[3],type:b[4],result:b[5],index:(b[6]<<8)|b[7]};
}

function duel(b){
  if(b[0]!==0xc1||b[2]!==0xaa)return null;
  const sub=b[3];
  if(sub===0x01){check(b.length===17,'Invalid duel start size');return {kind:'duel',sub,result:b[4],index:(b[5]<<8)|b[6],name:readName(b.slice(7,17))};}
  if(sub===0x02){check(b.length===16,'Invalid duel request size');return {kind:'duel',sub,index:(b[4]<<8)|b[5],name:readName(b.slice(6,16))};}
  if(sub===0x03){check(b.length===5,'Invalid duel end size');return {kind:'duel',sub,result:b[4]};}
  return null;
}

export function decodeCommandPacket(b){
  check(b instanceof Uint8Array,'Invalid command packet');
  return guildRequest(b)??guildResult(b)??guildUnion(b)??duel(b);
}
