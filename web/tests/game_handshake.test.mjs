import test from 'node:test';
import assert from 'node:assert/strict';
import {establishGameIdentity} from '../client/src/session/game-handshake.js';
import {createIdentityCodec} from '../client/src/protocol/client-identity.js';
import {core,profile,key,Socket,transport} from './helpers/game-fixture.mjs';
const identity='12345678-12345678-12345678-12345678-12345678';
const greeting=Uint8Array.of(0xc1,14,0xf1,0,1,0,1,49,48,50,48,48,1,0);
const flush=async()=>{for(let i=0;i<8;i++)await Promise.resolve();};
function setup(extra={}){
 const socket=new Socket(),jobs=new Map(),phases=[],packets=[];let id=0,created=0,calls=0;
 const timers={setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(id){jobs.delete(id);}};
 const promise=establishGameIdentity({core,profile,encryptionKeyFile:key(3),decryptionKeyFile:key(21846),
  gatewayUrl:'ws://127.0.0.1:9000',routes:{1:{id:'game-one',address:'127.0.0.1',port:55901}},selected:{code:1,endpoint:{address:'127.0.0.1',port:55901}},
  createSocket(){created++;return socket;}},
 {timers,identityProvider:()=>{calls++;return identity;},onPhase:s=>phases.push(s.phase),onPacket:p=>packets.push([...p]),...extra});
 return {promise,socket,jobs,phases,packets,get created(){return created;},get calls(){return calls;}};
}
function open(t){t.socket.emit('open');}
function greet(t,bytes=greeting){t.socket.emit('message',transport().encodeTransport(bytes));}
test('identity handshake sends F3:09 only after accepted greeting and preserves live channel',async()=>{
 const t=setup();assert.equal(t.calls,0);assert.equal(t.socket.sent.length,0);open(t);assert.equal(t.calls,0);
 const wire=transport().encodeTransport(greeting);t.socket.emit('message',wire.slice(0,6));assert.equal(t.calls,0);t.socket.emit('message',wire.slice(6));
 const connection=await t.promise;
 assert.equal(t.calls,1);assert.equal(connection.phase,'identity-sent');assert.equal(connection.state,'open');assert.equal(t.jobs.size,0);
 const raw=transport().decodeTransport(t.socket.sent[0]);
 assert.deepEqual(createIdentityCodec(core).decodePlain(raw),createIdentityCodec(core).packet(identity));
 assert.deepEqual(t.phases,['connecting','awaiting-greeting','awaiting-identity','identity-sent']);
 assert.ok(!t.phases.includes('verified'));
 greet(t,Uint8Array.of(0xc1,3,1));assert.deepEqual(t.packets,[[0xc1,3,1]]);
 connection.close();connection.close();assert.equal(connection.phase,'closed');assert.equal(t.socket.closed,1);
});
test('greeting rejection and missing provider never send identity',async()=>{
 for(const bytes of [Uint8Array.of(0xc1,3,1),Uint8Array.from(greeting,(b,i)=>i===4?0:b),Uint8Array.from(greeting,(b,i)=>i===12?2:b)]){
  const t=setup();const rejected=assert.rejects(t.promise,/greeting/);open(t);greet(t,bytes);await rejected;
  assert.equal(t.calls,0);assert.equal(t.socket.sent.length,0);assert.equal(t.jobs.size,0);
 }
 const t=setup({identityProvider:undefined});await assert.rejects(t.promise,/provider/);assert.equal(t.created,0);
});
test('phase deadlines and abort ignore late provider resolution',async()=>{
 for(const mode of ['timeout','abort']){
  let provide,providerSignal;const controller=new AbortController();
  const t=setup({signal:controller.signal,identityProvider:({signal})=>{providerSignal=signal;return new Promise(resolve=>provide=resolve);}});
  const rejected=assert.rejects(t.promise,/timeout|cancelled/);open(t);greet(t);await flush();assert.equal(typeof provide,'function');
  if(mode==='timeout')[...t.jobs.values()][0]();else controller.abort();
  await rejected;assert.equal(providerSignal.aborted,true);provide(identity);await flush();
  assert.equal(t.socket.sent.length,0);assert.equal(t.jobs.size,0);assert.equal(t.socket.closed,1);
 }
 const t=setup();const rejected=assert.rejects(t.promise,/timeout/);open(t);[...t.jobs.values()][0]();await rejected;assert.equal(t.calls,0);
});
test('provider errors, early packets and observer failures close without identity',async()=>{
 for(const provider of [()=>Promise.reject(new Error('private provider detail')),()=> 'invalid']){
  const t=setup({identityProvider:provider});const rejected=assert.rejects(t.promise,/Identity provider failed/);open(t);greet(t);await rejected;assert.equal(t.socket.sent.length,0);assert.equal(t.jobs.size,0);
 }
 let resolve;const pending=setup({identityProvider:()=>new Promise(r=>resolve=r)});
 const rejected=assert.rejects(pending.promise,/Unexpected packet/);open(pending);greet(pending);await flush();greet(pending,Uint8Array.of(0xc1,3,1));await rejected;resolve(identity);await flush();assert.equal(pending.socket.sent.length,0);
 const observer=setup({onPhase(){throw new Error('observer');}});await assert.rejects(observer.promise,/callback/);assert.equal(observer.jobs.size,0);assert.equal(observer.created,0);
});
test('cancel before start and remote close release handshake; lifetime abort closes established channel',async()=>{
 const controller=new AbortController();controller.abort();const early=setup({signal:controller.signal});await assert.rejects(early.promise,/cancelled/);assert.equal(early.created,0);
 const remote=setup();const rejected=assert.rejects(remote.promise,/transport ended/);open(remote);remote.socket.emit('close');await rejected;assert.equal(remote.jobs.size,0);
 const lifetime=new AbortController(),t=setup({signal:lifetime.signal});open(t);greet(t);const connection=await t.promise;lifetime.abort();assert.equal(connection.phase,'closed');assert.equal(t.socket.closed,1);assert.equal(t.jobs.size,0);
});
