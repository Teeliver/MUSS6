// Duel broadcasts for UPDATE603. Source: GameServer/Duel.h + Main5.2 WSclient.cpp.
const latin1=new TextDecoder('latin1');
const readName=(b,offset,length)=>latin1.decode(b.slice(offset,offset+length)).split('\0')[0];
const check=(ok,message)=>{if(!ok)throw Error(message);};
export function decodeDuelBroadcast(b){
 if(!(b instanceof Uint8Array)||b[0]!==0xc1||b[2]!==0xaa)return undefined;
 const sub=b[3];if([4,5,12,13].includes(sub))check(b[1]===b.length,'Invalid duel frame length');
 if(sub===0x04){check(b.length===10,'Invalid duel score size');return {kind:'score',index1:(b[4]<<8)|b[5],index2:(b[6]<<8)|b[7],score1:b[8],score2:b[9]};}
 if(sub===0x05){check(b.length===12,'Invalid duel HP size');return {kind:'hp',index1:(b[4]<<8)|b[5],index2:(b[6]<<8)|b[7],life1:b[8],life2:b[9],shield1:b[10],shield2:b[11]};}
 if(sub===0x0d){check(b.length===5,'Invalid duel round size');return {kind:'round',flag:b[4]};}
 if(sub===0x0c){check(b.length===24,'Invalid duel result size');return {kind:'win',winner:readName(b,4,10),loser:readName(b,14,10)};}
 return undefined;
}

export const duelWatching=watch=>['watching','leaving'].includes(watch?.phase);
export function duelWatchPacket(action,channel){check(['join','leave'].includes(action)&&Number.isInteger(channel)&&channel>=0&&channel<4,'Invalid duel watch request');return Uint8Array.of(0xc3,5,0xaa,action==='join'?7:9,channel);}
export function decodeDuelWatch(b){
 if(!(b instanceof Uint8Array)||![0xc1,0xc3].includes(b[0])||b[2]!==0xaa||![7,8,9,10,11].includes(b[3]))return undefined;
 check(b[1]===b.length,'Invalid duel watch frame length');const sub=b[3];
 if(sub===7){check(b.length===30,'Invalid duel watch join size');if(b[4]!==0)return {kind:'join',result:b[4]};check(b[5]<4,'Invalid duel channel');return {kind:'join',result:0,channel:b[5],fighters:[{name:readName(b,6,10),index:(b[26]<<8)|b[27]},{name:readName(b,16,10),index:(b[28]<<8)|b[29]}]};}
 if(sub===9){check(b.length===5,'Invalid duel watch leave size');return {kind:'leave',result:b[4]};}
 if(sub===8||sub===10){check(b.length===14,'Invalid duel spectator update size');return {kind:sub===8?'spectatorAdd':'spectatorRemove',name:readName(b,4,10)};}
 check(b.length>=5&&b[4]<=10&&b.length===5+b[4]*10,'Invalid duel spectator list size');return {kind:'spectatorList',names:Array.from({length:b[4]},(_,i)=>readName(b,5+i*10,10))};
}
export const emptyDuelWatch=()=>({phase:'idle',roundSeq:0,channel:undefined,fighters:[],spectators:[],winner:'',loser:'',result:undefined});
export const cloneDuelWatch=w=>({...w,fighters:w.fighters.map(f=>({...f})),spectators:[...w.spectators]});
export function reduceDuelWatch(w,event){
 if(event.kind==='joinPending')return {...emptyDuelWatch(),phase:'joining',channel:event.channel};
 if(event.kind==='reset'||event.kind==='map'&&event.map!==64&&duelWatching(w))return emptyDuelWatch();
 if(event.kind==='join'){if(event.result!==0)return {...emptyDuelWatch(),result:event.result};return {...emptyDuelWatch(),phase:'watching',channel:event.channel,fighters:event.fighters.map(f=>({...f,score:0,hp:1,sd:1}))};}
 if(event.kind==='timeout')return {...w,phase:w.phase==='joining'?'idle':w.phase==='leaving'?'watching':w.phase};
 if(event.kind==='leavePending'&&w.phase==='watching')return {...w,phase:'leaving'};
 if(event.kind==='leave'&&event.result===0)return emptyDuelWatch();
 if(!duelWatching(w))return w;
 if(event.kind==='spectatorAdd')return {...w,spectators:[...new Set([...w.spectators,event.name].filter(Boolean))].slice(0,10)};
 if(event.kind==='spectatorRemove')return {...w,spectators:w.spectators.filter(n=>n!==event.name)};
 if(event.kind==='spectatorList')return {...w,spectators:[...new Set(event.names.filter(Boolean))].slice(0,10)};
 if(event.kind==='win')return {...w,winner:event.winner,loser:event.loser};
 if(event.kind==='round'&&event.flag===0)return {...w,roundSeq:w.roundSeq+1,fighters:w.fighters.map(f=>({...f,hp:1,sd:1}))};
 if(['score','hp'].includes(event.kind)&&w.fighters.length===2&&w.fighters.every(f=>[event.index1,event.index2].includes(f.index))){return {...w,fighters:w.fighters.map(f=>{const one=f.index===event.index1;return {...f,...(event.kind==='score'?{score:one?event.score1:event.score2}:{hp:Math.max(0,Math.min(1,(one?event.life1:event.life2)/100)),sd:Math.max(0,Math.min(1,(one?event.shield1:event.shield2)/100))})};})};}
 return w;
}

// CheckAttack player branch: a Duel only targets the registered opponent.
export const isDuelOpponent=(duel,index)=>!!duel?.active&&!duelWatching(duel.watch)&&Number.isInteger(index)&&duel.enemyIndex===index;
