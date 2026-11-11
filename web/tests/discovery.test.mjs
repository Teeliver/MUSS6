import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import createCore from '../.build/wasm/frame-reader.mjs';
import { createDiscovery } from '../client/src/session/discovery.js';
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
class Socket extends EventTarget {
  bufferedAmount=0;sent=[];closed=0;
  emit(type,bytes){const event=new Event(type);if(bytes)event.data=Uint8Array.from(bytes).buffer;this.dispatchEvent(event);}
  send(buffer){this.sent.push([...new Uint8Array(buffer)]);}
  close(){this.closed++;this.emit('close');}
}
function setup(){
 const sockets=[],jobs=new Map();let id=0;
 const timers={setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(key){jobs.delete(key);}};
 const routes={1:{id:'game-one',address:'127.0.0.1',port:55901}};
 const api=createDiscovery({core,createSocket:()=>{const s=new Socket();sockets.push(s);return s;},connectUrl:'ws://127.0.0.1:9000/mu/connect',routes,timers});
 return {api,sockets,jobs,routes};
}
const open=t=>{const socket=t.sockets.at(-1);socket.emit('open');socket.emit('message',[0xc1,4,0,1]);return socket;};
test('discovery handshake then list, fragments and exact request order',async()=>{
 const t=setup(),result=t.api.discover();const s=t.sockets[0];s.emit('open');assert.equal(s.sent.length,0);
 s.emit('message',[0xc1,4]);s.emit('message',[0,1]);assert.deepEqual(s.sent,[[0xc1,4,0xf4,6]]);
 s.emit('message',[0xc2,0,11,0xf4]);s.emit('message',[6,0,1,1,0,42,0xcc]);
 assert.deepEqual(await result,[{code:1,load:42,type:0xcc}]);assert.equal(t.jobs.size,0);assert.equal(s.closed,1);
});
test('select uses a new CS connection and validates immutable route metadata',async()=>{
 const t=setup(),result=t.api.select(1);t.routes[1].port=80;const s=open(t);
 assert.deepEqual(s.sent,[[0xc1,5,0xf4,3,1]]);
 const b=new Uint8Array(22);b.set([0xc1,22,0xf4,3]);b.set(new TextEncoder().encode('127.0.0.1'),4);b[20]=55901&255;b[21]=55901>>8;s.emit('message',b);
 assert.deepEqual(await result,{code:1,endpoint:{address:'127.0.0.1',port:55901},routePath:'/mu/game-one',gameUrl:'ws://127.0.0.1:9000/mu/game-one'});
 await assert.rejects(t.api.select(256));await assert.rejects(t.api.select(2));assert.equal(t.sockets.length,1);
});
test('invalid handshake, duplicate list and endpoint mismatch fail closed',async()=>{
 for(const response of [[0xc1,4,0,0],[0xc1,4,0xf4,6]]){
  const t=setup(),result=t.api.discover();const s=t.sockets[0];s.emit('open');s.emit('message',response);await assert.rejects(result);assert.equal(t.jobs.size,0);assert.equal(s.closed,1);
 }
 const duplicate=setup(),badList=duplicate.api.discover();const ds=open(duplicate);
 ds.emit('message',[0xc2,0,15,0xf4,6,0,2,1,0,42,0xcc,1,0,99,0xcc]);await assert.rejects(badList);assert.equal(duplicate.jobs.size,0);
 const t=setup(),result=t.api.select(1);const s=open(t);const b=new Uint8Array(22);b.set([0xc1,22,0xf4,3]);b.set(new TextEncoder().encode('127.0.0.1'),4);b[20]=80;s.emit('message',b);await assert.rejects(result);assert.equal(s.closed,1);
});
test('abort, response deadline, remote close and independent operations clean up',async()=>{
 const t=setup(),abort=new AbortController(),result=t.api.discover({signal:abort.signal});abort.abort();await assert.rejects(result);assert.equal(t.jobs.size,0);assert.equal(t.sockets[0].closed,1);
 const before=t.sockets.length;await assert.rejects(t.api.discover({signal:abort.signal}));assert.equal(t.sockets.length,before);
 const u=setup(),timeout=u.api.discover();const s=open(u);for(const fn of [...u.jobs.values()])fn();await assert.rejects(timeout);assert.equal(s.closed,1);assert.equal(u.jobs.size,0);
 const v=setup(),a=v.api.discover(),b=v.api.discover();v.sockets[0].emit('close');await assert.rejects(a);const second=v.sockets[1];second.emit('open');second.emit('message',[0xc1,4,0,1]);second.emit('message',[0xc2,0,7,0xf4,6,0,0]);assert.deepEqual(await b,[]);assert.equal(v.jobs.size,0);
});

test('live ConnectServer coalesces greeting and F3:EA name list before F4:06 reply',async()=>{
 const t=setup(),result=t.api.discover(),socket=t.sockets[0];socket.emit('open');
 const names=new Uint8Array(40);names.set([0xc2,0,40,0xf3,0xea,1,1,0]);names.set(new TextEncoder().encode('GameServer_1'),8);
 socket.emit('message',[0xc1,4,0,1,...names]);socket.emit('message',[0xc2,0,11,0xf4,6,0,1,1,0,42,0xcc]);
 assert.deepEqual(await result,[{code:1,load:42,type:0xcc,name:'GameServer_1'}]);assert.equal(t.jobs.size,0);
});
test('malformed or repeated server-name packets fail discovery',async()=>{
 for(const malformed of [true,false]){const t=setup(),pending=t.api.discover(),s=open(t);s.emit('message',[0xc2,0,6,0xf3,0xea,malformed?1:0]);if(!malformed)s.emit('message',[0xc2,0,6,0xf3,0xea,0]);await assert.rejects(pending);assert.equal(t.jobs.size,0);}
});
