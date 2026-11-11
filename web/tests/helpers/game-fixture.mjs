import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import createCore from '../../.build/wasm/frame-reader.mjs';
import {createGameChannel} from '../../client/src/session/game-channel.js';
import {createTransportCodec} from '../../client/src/protocol/connect-codec.js';
import {deriveGameKeys} from '../../client/src/protocol/game-profile.js';
export const core=await createCore({wasmBinary:await readFile(new URL('../../.build/wasm/frame-reader.wasm',import.meta.url))});
export const profile={serial:'1234567890123456',rawMhp1:1,rawMhp2:255};
export function key(multiplier){const b=new Uint8Array(54),v=new DataView(b.buffer),m=[0x3F08A79B,0xE25CC287,0x93D27AB9,0x20DEA7BF];v.setUint16(0,4370,true);v.setUint32(2,54,true);for(let i=0;i<4;i++){v.setUint32(6+i*4,65537^m[i],true);v.setUint32(22+i*4,multiplier^m[i],true);v.setUint32(38+i*4,0^m[i],true);}return b;}
export class Socket extends EventTarget{
 bufferedAmount=0;sent=[];closed=0;
 emit(type,bytes){const e=new Event(type);if(bytes)e.data=Uint8Array.from(bytes).buffer;this.dispatchEvent(e);}
 send(bytes){this.sent.push(new Uint8Array(bytes));}close(){this.closed++;this.emit('close');}
}
export function setup(extra={}){
 const socket=new Socket(),packets=[],states=[],jobs=new Map();let id=0,created=0;
 const enc=key(3),dec=key(21846);
 const channel=createGameChannel({core,createSocket:url=>{assert.equal(url,'ws://127.0.0.1:9000/mu/game-one');created++;return socket;},
 gatewayUrl:'ws://127.0.0.1:9000',routes:{1:{id:'game-one',address:'127.0.0.1',port:55901}},selected:{code:1,endpoint:{address:'127.0.0.1',port:55901}},profile,
 encryptionKeyFile:enc,decryptionKeyFile:dec,onPacket:p=>packets.push([...p]),onState:s=>states.push(s),
 timers:{setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(i){jobs.delete(i);}},...extra});
 return {channel,socket,packets,states,jobs,enc,dec,get created(){return created;}};
}
export const transport=()=>createTransportCodec(core,deriveGameKeys(core,profile));
export const start=t=>{t.channel.connect();t.socket.emit('open');};
