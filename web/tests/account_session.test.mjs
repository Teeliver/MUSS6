import test from 'node:test';
import assert from 'node:assert/strict';
import {establishAccountSession} from '../client/src/session/account-session.js';
import {createPacketCipher} from '../client/src/platform/wasm/create-packet-cipher.js';
import {createIdentityCodec} from '../client/src/protocol/client-identity.js';
import {createAccountCodec} from '../client/src/protocol/account-codec.js';
import {core,profile,key,Socket,transport} from './helpers/game-fixture.mjs';
const input={account:'TestUser',password:'Test!123',tick:10,version:Uint8Array.of(1,2,3,4,5),serial:'1234567890123456'};
function setup(extra={}){
 const socket=new Socket(),jobs=new Map(),phases=[],packets=[];let id=0;
 const promise=establishAccountSession({core,profile,encryptionKeyFile:key(3),decryptionKeyFile:key(21846),
 gatewayUrl:'ws://127.0.0.1:9000',routes:{1:{id:'game-one',address:'127.0.0.1',port:55901}},selected:{code:1,endpoint:{address:'127.0.0.1',port:55901}},createSocket:()=>socket},
 {identityProvider:()=> '12345678-12345678-12345678-12345678-12345678',onPhase:e=>phases.push(e.phase),onPacket:p=>packets.push([...p]),
 timers:{setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(i){jobs.delete(i);}},...extra});
 return {socket,jobs,phases,packets,promise};
}
const emit=(t,bytes)=>t.socket.emit('message',transport().encodeTransport(Uint8Array.from(bytes)));
async function ready(t){t.socket.emit('open');emit(t,[0xc1,14,0xf1,0,1,0,1,49,48,50,48,48,1,0]);return t.promise;}
test('account session waits for identity, sends one encrypted login, and accepts only result 1',async()=>{
 const t=setup(),session=await ready(t);assert.equal(session.phase,'ready-for-login');assert.equal(t.socket.sent.length,1);
 assert.throws(()=>session.send(Uint8Array.of(0xc1,3,1)));
 const result=session.login(input);await assert.rejects(session.login(input),/not available/);assert.equal(t.socket.sent.length,2);
 const cipher=createPacketCipher(core,{encryptionKeyFile:key(3),decryptionKeyFile:key(21846)});
 try{
  const plain=createIdentityCodec(core).decodePlain(cipher.decode(transport().decodeTransport(t.socket.sent[1])));
  const expected=createAccountCodec(core).login(input);expected[0]=0xc1;assert.deepEqual(plain,expected);
 }finally{cipher.delete();}
 // Successful result and following packet in a single transport chunk.
 emit(t,[0xc1,5,0xf1,1,1,0xc1,3,7]);assert.deepEqual(await result,{code:1,accepted:true});
 assert.equal(session.phase,'authenticated');assert.deepEqual(t.packets,[[0xc1,3,7]]);assert.equal(t.jobs.size,0);
 session.send(Uint8Array.of(0xc1,3,2));session.close();assert.equal(t.socket.closed,1);
});
test('account rejection preserves raw code and closes without retry',async()=>{
 for(const code of [0,6,8,255]){
  const t=setup(),session=await ready(t),result=session.login(input);emit(t,[0xc1,5,0xf1,1,code]);
  assert.deepEqual(await result,{code,accepted:false});assert.equal(session.phase,'closed');assert.equal(t.jobs.size,0);
  await assert.rejects(session.login(input));assert.equal(t.socket.sent.length,2);
 }
});
test('account timeout, abort, malformed response and remote close reject pending login',async()=>{
 for(const mode of ['timeout','abort','malformed','remote','local']){
  const controller=new AbortController(),t=setup({signal:controller.signal}),session=await ready(t);
  const result=session.login(input),rejected=assert.rejects(result,/closed/);
  if(mode==='timeout')[...t.jobs.values()][0]();
  if(mode==='abort')controller.abort();
  if(mode==='malformed')emit(t,[0xc1,4,0xf1,1]);
  if(mode==='remote')t.socket.emit('close');
  if(mode==='local')session.close();
  await rejected;assert.equal(session.phase,'closed');assert.equal(t.jobs.size,0);
  emit(t,[0xc1,5,0xf1,1,1]);assert.equal(session.phase,'closed');
 }
});
test('invalid input sends nothing and callback or queue failures close the account session',async()=>{
 const t=setup(),session=await ready(t);await assert.rejects(session.login({...input,tick:-1}),/Invalid/);
 assert.equal(t.socket.sent.length,1);assert.equal(session.phase,'ready-for-login');
 t.socket.bufferedAmount=262144;await assert.rejects(session.login(input),/closed/);assert.equal(t.jobs.size,0);
 const observer=setup({onPhase:e=>{if(e.phase==='awaiting-login')throw new Error('observer');}}),s=await ready(observer);
 await assert.rejects(s.login(input),/closed/);assert.equal(observer.jobs.size,0);
 const callback=setup({onPacket(){throw new Error('consumer');}}),c=await ready(callback),r=c.login(input);
 emit(callback,[0xc1,5,0xf1,1,1]);await r;emit(callback,[0xc1,3,2]);assert.equal(c.phase,'closed');
});
