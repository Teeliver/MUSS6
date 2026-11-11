import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import createModule from '../.build/wasm/frame-reader.mjs';
import { createReader } from '../client/src/platform/wasm/create-reader.js';
import { TransportSession } from '../client/src/platform/transport/session.js';
const core=await createModule({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
class Socket extends EventTarget {
 bufferedAmount=0;sent=[];closed=0;
 emit(type,data){const e=new Event(type);if(data!==undefined)e.data=Uint8Array.from(data).buffer;this.dispatchEvent(e);}
 send(data){this.sent.push([...new Uint8Array(data)]);}
 close(){this.closed++;this.emit('close');}
}
function setup(extra={}) {
 const socket=new Socket(),frames=[],states=[],jobs=new Map();let sequence=0,deletes=0;
 const timers={setTimeout(fn){jobs.set(++sequence,fn);return sequence;},clearTimeout(id){jobs.delete(id);}};
 const session=new TransportSession({createSocket:()=>socket,createReader:()=>{
  const r=createReader(core);return {feed:b=>r.feed(b),finish:()=>r.finish(),pendingSize:()=>r.pendingSize(),delete(){deletes++;r.delete();}};
 },decodeTransport:b=>b,encodeTransport:b=>b,onFrame:f=>frames.push([...f]),onState:s=>states.push(s),timers,...extra});
 return {socket,session,frames,states,jobs,get deletes(){return deletes;}};
}
const start=t=>{t.session.connect('ws://127.0.0.1:9000');t.socket.emit('open');};
test('transport and actual WASM preserve fragmented/coalesced byte order',()=>{
 const t=setup();start(t);t.socket.emit('message',[0xc1]);t.socket.emit('message',[4,0,1,0xc2,0,5,0xf4,6]);
 assert.deepEqual(t.frames,[[0xc1,4,0,1],[0xc2,0,5,0xf4,6]]);assert.equal(t.jobs.size,0);
 t.session.send(Uint8Array.from([0xc1,4,0,1]));assert.deepEqual(t.socket.sent,[[0xc1,4,0,1]]);
 t.socket.emit('close');assert.equal(t.session.state,'closed');assert.equal(t.deletes,1);
 t.socket.emit('message',[0]);assert.equal(t.frames.length,2);assert.throws(()=>t.session.connect('ws://localhost:1'));
});
test('connection and non-resetting partial deadlines release reader once',()=>{
 const t=setup();t.session.connect('ws://localhost:9000');[...t.jobs.values()][0]();assert.equal(t.session.state,'failed');assert.equal(t.deletes,1);
 const p=setup();start(p);p.socket.emit('message',[0xc1]);const id=[...p.jobs.keys()][0];p.socket.emit('message',[4]);assert.ok(p.jobs.has(id));p.jobs.get(id)();assert.equal(p.session.state,'failed');assert.equal(p.deletes,1);
});
test('EOF, invalid input, callback failure and backpressure fail closed',()=>{
 for(const operation of [t=>{t.socket.emit('message',[0xc1]);t.socket.emit('close');},t=>t.socket.emit('message',[0]),t=>{t.socket.bufferedAmount=262144;assert.throws(()=>t.session.send(Uint8Array.of(1)));}]){
  const t=setup();start(t);operation(t);assert.equal(t.session.state,'failed');assert.equal(t.deletes,1);assert.equal(t.jobs.size,0);
 }
 const t=setup({onFrame(){throw new Error('consumer');}});start(t);t.socket.emit('message',[0xc1,4,0,1]);assert.equal(t.session.state,'failed');
});
test('no cipher defaults, separate sessions, and URL restrictions',()=>{
 assert.throws(()=>new TransportSession({}));
 for(const url of ['http://localhost','ws://example.com','wss://user:pass@example.com'])assert.throws(()=>setup().session.connect(url));
 const a=setup(),b=setup();start(a);start(b);a.socket.emit('message',[0xc1]);b.socket.emit('message',[0xc1,4,0,1]);assert.equal(a.frames.length,0);assert.equal(b.frames.length,1);a.session.close();b.session.close();assert.equal(a.deletes,1);assert.equal(b.deletes,1);
});

test('decoder failures, non-binary messages and pending bytes are bounded',()=>{
 const d=setup({decodeTransport(){throw new Error('cipher');}});start(d);d.socket.emit('message',[1]);assert.equal(d.session.state,'failed');assert.equal(d.deletes,1);
 const text=setup();start(text);const event=new Event('message');event.data='invalid';text.socket.dispatchEvent(event);assert.equal(text.session.state,'failed');
 const limited=setup({maxBytes:4});start(limited);limited.socket.emit('message',[0xc2,0,8]);limited.socket.emit('message',[0,0]);assert.equal(limited.session.state,'failed');assert.equal(limited.deletes,1);
});
test('state callback failure and local cancellation do not leave timers/readers',()=>{
 const t=setup({onState(){throw new Error('observer');}});t.session.connect('ws://localhost:9000');assert.equal(t.session.state,'failed');assert.equal(t.deletes,1);assert.equal(t.jobs.size,0);
 const local=setup();local.session.connect('ws://localhost:9000');local.session.close();local.session.close();assert.equal(local.deletes,1);assert.equal(local.jobs.size,0);local.socket.emit('open');assert.equal(local.session.state,'closed');
});
