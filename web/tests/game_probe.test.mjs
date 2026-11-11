import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import createCore from '../.build/wasm/frame-reader.mjs';
import { createGameProbe } from '../client/src/session/game-probe.js';
import { deriveGameKeys } from '../client/src/protocol/game-profile.js';
import { createTransportCodec } from '../client/src/protocol/connect-codec.js';
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const profile={serial:'1234567890123456',rawMhp1:1,rawMhp2:255};
const selected={code:1,endpoint:{address:'127.0.0.1',port:55901},gameUrl:'ws://evil.invalid/mu/bad'};
const plain=Uint8Array.from([0xc1,14,0xf1,0,1,0x2e,0x7c,49,48,50,48,48,1,0]);
const encode=bytes=>createTransportCodec(core,deriveGameKeys(core,profile)).encodeTransport(bytes);
class Socket extends EventTarget {
 bufferedAmount=0;closed=0;sent=[];
 emit(type,bytes){const e=new Event(type);if(bytes)e.data=Uint8Array.from(bytes).buffer;this.dispatchEvent(e);}
 send(bytes){this.sent.push(bytes);}
 close(){this.closed++;this.emit('close');}
}
function setup(){
 const sockets=[],urls=[],jobs=new Map();let id=0;
 const timers={setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(key){jobs.delete(key);}};
 const routes={1:{id:'game-one',address:'127.0.0.1',port:55901}};
 const probe=createGameProbe({core,createSocket:url=>{urls.push(url);const s=new Socket();sockets.push(s);return s;},gatewayUrl:'ws://127.0.0.1:9000',routes,timers});
 return {probe,sockets,urls,jobs,routes};
}
test('encrypted greeting is decoded across every split and closes without login bytes',{timeout:2000},async()=>{
 const wire=encode(plain);
 for(let split=0;split<=wire.length;split++){
  const t=setup(),result=t.probe(selected,profile);const s=t.sockets[0];s.emit('open');s.emit('message',wire.subarray(0,split));s.emit('message',wire.subarray(split));
  assert.deepEqual(await result,{index:11900,serverCode:1,encodedVersion:[49,48,50,48,48]});
  assert.equal(s.sent.length,0);assert.equal(s.closed,1);assert.equal(t.jobs.size,0);assert.deepEqual(t.urls,['ws://127.0.0.1:9000/mu/game-one']);
 }
});
test('server rejection, wrong code and wrong keys are rejected',{timeout:2000},async()=>{
 for(const field of [4,12]){
  const t=setup(),result=t.probe(selected,profile),s=t.sockets[0];s.emit('open');const b=plain.slice();b[field]=0;s.emit('message',encode(b));await assert.rejects(result);assert.equal(s.closed,1);assert.equal(t.jobs.size,0);
 }
 const t=setup(),result=t.probe(selected,{...profile,rawMhp1:2});const s=t.sockets[0];s.emit('open');s.emit('message',encode(plain));for(const fn of [...t.jobs.values()])fn();await assert.rejects(result);assert.equal(s.closed,1);
});
test('unlisted endpoint and invalid profile never create a socket; route snapshot is fixed',{timeout:2000},async()=>{
 const t=setup();await assert.rejects(t.probe({...selected,endpoint:{address:'example.com',port:55901}},profile));await assert.rejects(t.probe(selected,{...profile,serial:'short'}));assert.equal(t.sockets.length,0);
 const result=t.probe(selected,profile);t.routes[1].id='changed';const s=t.sockets[0];s.emit('open');s.emit('message',encode(plain));await result;assert.ok(t.urls[0].endsWith('/mu/game-one'));
});
test('cancel, handshake deadline and truncated EOF dispose the probe',{timeout:2000},async()=>{
 const t=setup(),abort=new AbortController(),result=t.probe(selected,profile,{signal:abort.signal});abort.abort();await assert.rejects(result);assert.equal(t.jobs.size,0);assert.equal(t.sockets[0].closed,1);
 await assert.rejects(t.probe(selected,profile,{signal:abort.signal}));assert.equal(t.sockets.length,1);
 const u=setup(),timeout=u.probe(selected,profile),s=u.sockets[0];s.emit('open');for(const fn of [...u.jobs.values()])fn();await assert.rejects(timeout);assert.equal(u.jobs.size,0);
 const v=setup(),eof=v.probe(selected,profile),vs=v.sockets[0];vs.emit('open');vs.emit('message',encode(plain).subarray(0,3));vs.emit('close');await assert.rejects(eof);assert.equal(v.jobs.size,0);
});
