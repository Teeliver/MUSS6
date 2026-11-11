import test from 'node:test';
import assert from 'node:assert/strict';
import {establishLobbySession} from '../client/src/session/lobby-session.js';
import {createIdentityCodec} from '../client/src/protocol/client-identity.js';
import {core,profile,key,Socket,transport} from './helpers/game-fixture.mjs';
const input={account:'TestUser',password:'Test!123',tick:10,version:Uint8Array.of(1,2,3,4,5),serial:'1234567890123456'};
const roster=[0xc1,8,0xf3,0,1,2,0,3],policy=[0xc1,14,0xde,0,7,255,1,0,2,0,0x90,1,0x2c,1];
function setup(extra={}){
 const socket=new Socket(),jobs=new Map(),phases=[],packets=[];let id=0;
 const promise=establishLobbySession({core,profile,encryptionKeyFile:key(3),decryptionKeyFile:key(21846),gatewayUrl:'ws://127.0.0.1:9000',
 routes:{1:{id:'game-one',address:'127.0.0.1',port:55901}},selected:{code:1,endpoint:{address:'127.0.0.1',port:55901}},createSocket:()=>socket},
 {identityProvider:()=> '12345678-12345678-12345678-12345678-12345678',onPhase:e=>phases.push(e.phase),onPacket:p=>packets.push([...p]),
 timers:{setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(i){jobs.delete(i);}},...extra});
 return {socket,jobs,phases,packets,promise};
}
const emit=(t,b)=>t.socket.emit('message',transport().encodeTransport(Uint8Array.from(b)));
async function ready(t){t.socket.emit('open');emit(t,[0xc1,14,0xf1,0,1,0,1,49,48,50,48,48,1,0]);return t.promise;}
async function login(t){const s=await ready(t),r=s.login(input);emit(t,[0xc1,5,0xf1,1,1]);await r;return s;}
test('lobby waits for list and policy, handles coalescing and permits sequential refresh',async()=>{
 const t=setup(),s=await ready(t);await assert.rejects(s.characters(),/not available/);
 const logged=s.login(input);emit(t,[0xc1,5,0xf1,1,1]);await logged;
 const result=s.characters();await assert.rejects(s.characters(),/not available/);assert.throws(()=>s.send(Uint8Array.of(0xc1,3,1)));
 const sent=createIdentityCodec(core).decodePlain(transport().decodeTransport(t.socket.sent[2]));assert.deepEqual([...sent],[0xc1,4,0xf3,0]);
 let resolved=false;result.then(()=>resolved=true);emit(t,roster);await Promise.resolve();assert.equal(resolved,false);assert.equal(s.phase,'awaiting-roster-policy');
 emit(t,policy);const list=await result;assert.equal(list.classCode,1);assert.equal(list.policy.classCode,7);assert.equal(list.policy.deleteMaxLevel,300);assert.equal(t.jobs.size,0);
 const next=s.characters();emit(t,[...roster,...policy,0xc1,3,9]);await next;assert.equal(s.phase,'roster-ready');assert.deepEqual(t.packets,[[0xc1,3,9]]);s.close();
});
test('lobby rejects swapped, duplicate and malformed paired responses',async()=>{
 for(const sequence of [[policy],[roster,roster],[roster,[0xc1,13,0xde,0,7,0,0,0,0,0,0,0,0]],[[0xc1,8,0xf3,0,0,0,6,0]]]){
  const t=setup(),s=await login(t),result=s.characters(),rejected=assert.rejects(result,/closed/);
  for(const packet of sequence)emit(t,packet);await rejected;assert.equal(s.phase,'closed');assert.equal(t.jobs.size,0);
 }
});
test('lobby deadline covers both packets and abort/close discard late replies',async()=>{
 for(const mode of ['timeout','abort','remote','local']){
  const t=setup(),s=await login(t),controller=new AbortController(),result=s.characters({signal:controller.signal}),rejected=assert.rejects(result,/closed/);
  const timer=[...t.jobs.keys()][0];emit(t,roster);assert.ok(t.jobs.has(timer));
  if(mode==='timeout')t.jobs.get(timer)();if(mode==='abort')controller.abort();if(mode==='remote')t.socket.emit('close');if(mode==='local')s.close();
  await rejected;emit(t,policy);assert.equal(s.phase,'closed');assert.equal(t.jobs.size,0);
 }
 const t=setup(),s=await login(t),controller=new AbortController();controller.abort();await assert.rejects(s.characters({signal:controller.signal}),/cancelled/);assert.equal(t.socket.sent.length,2);s.close();
});
test('lobby queue/observer failure closes and independent sessions keep their rosters separate',async()=>{
 const t=setup(),s=await login(t);t.socket.bufferedAmount=262144;await assert.rejects(s.characters(),/closed/);assert.equal(t.jobs.size,0);
 const bad=setup({onPhase:e=>{if(e.phase==='roster-ready')throw new Error('observer');}}),b=await login(bad),r=b.characters(),rejected=assert.rejects(r,/closed/);emit(bad,[...roster,...policy]);await rejected;assert.equal(bad.jobs.size,0);
 const a=setup(),c=setup(),sa=await login(a),sc=await login(c),ra=sa.characters(),rc=sc.characters();emit(a,[...roster,...policy]);await ra;assert.equal(sc.phase,'awaiting-roster');emit(c,[...roster,...policy]);await rc;sa.close();sc.close();
});
