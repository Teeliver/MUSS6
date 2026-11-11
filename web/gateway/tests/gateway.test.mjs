import test from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import { once } from 'node:events';
import WebSocket from 'ws';
import { readFile } from 'node:fs/promises';
import createCore from '../../.build/wasm/frame-reader.mjs';
import { createReader } from '../../client/src/platform/wasm/create-reader.js';
import { TransportSession } from '../../client/src/platform/transport/session.js';
import { createGateway } from '../src/gateway.mjs';
const core=await createCore({wasmBinary:await readFile(new URL('../../.build/wasm/frame-reader.wasm',import.meta.url))});
const origin='http://127.0.0.1:8080';
async function setup(t,handler=socket=>socket.pipe(socket),limits={}){
 const sockets=new Set();
 const tcp=net.createServer(socket=>{sockets.add(socket);socket.on('close',()=>sockets.delete(socket));handler(socket);});
 await new Promise(resolve=>tcp.listen(0,'127.0.0.1',resolve));
 const gateway=createGateway({routes:{fixture:{host:'127.0.0.1',port:tcp.address().port}},origins:[origin],...limits});
 const address=await gateway.listen();
 t.after(async()=>{await gateway.close();for(const s of sockets)s.destroy();await new Promise(resolve=>tcp.close(resolve));});
 return {gateway,url:`ws://127.0.0.1:${address.port}/mu/fixture`};
}
async function client(t,url,opts={}){
 const ws=new WebSocket(url,{origin,perMessageDeflate:false,...opts});ws.on('error',()=>{});t.after(()=>ws.terminate());await once(ws,'open');return ws;
}
const binary=message=>new Promise((resolve,reject)=>{message.once('message',(data,isBinary)=>{if(!isBinary)reject(new Error('Nonbinary'));else resolve(data);});});
test('real loopback WS/TCP preserves all bytes and client isolation',{timeout:5000},async t=>{
 let connections=0;
 const {url}=await setup(t,socket=>{connections++;socket.pipe(socket);});
 const a=await client(t,url),b=await client(t,url);
 const expected=Buffer.from(Array.from({length:1024},(_,i)=>i%256));
 const receive=async ws=>{let bytes=Buffer.alloc(0);while(bytes.length<expected.length)bytes=Buffer.concat([bytes,await binary(ws)]);return bytes;};
 const output=receive(a);a.send(expected.subarray(0,1));a.send(expected.subarray(1,512));a.send(expected.subarray(512));
 assert.deepEqual(await output,expected);
 const reply=binary(b);b.send(Buffer.from([7,8,9]));assert.deepEqual(await reply,Buffer.from([7,8,9]));assert.equal(connections,2);
});
test('unknown/query routes, Origin and connection cap reject upgrades',{timeout:5000},async t=>{
 const {url}=await setup(t,()=>{},{maxConnections:1});
 for(const [endpoint,options,status] of [[url, {origin:'http://evil.invalid'},403],[url+'?host=127.0.0.1',{},404],[url.replace('fixture','unknown'),{},404]]){
  const ws=new WebSocket(endpoint,{origin,...options});ws.on('error',()=>{});const [,res]=await once(ws,'unexpected-response');assert.equal(res.statusCode,status);res.resume();ws.terminate();
 }
 await client(t,url);
 const ws=new WebSocket(url,{origin});ws.on('error',()=>{});const [,res]=await once(ws,'unexpected-response');assert.equal(res.statusCode,503);res.resume();ws.terminate();
});
test('text and oversized binary are closed; TCP EOF closes WS',{timeout:5000},async t=>{
 const {url}=await setup(t,()=>{},{maxPayload:32});
 const text=await client(t,url);let closed=once(text,'close');text.send('not MU bytes');assert.equal((await closed)[0],1003);
 const large=await client(t,url);closed=once(large,'close');large.send(Buffer.alloc(33));assert.equal((await closed)[0],1009);
 const end=await setup(t,socket=>socket.end(Buffer.from([0xc1,4,0,1])));
 const ws=new WebSocket(end.url,{origin});ws.on('error',()=>{});t.after(()=>ws.terminate());const received=binary(ws);const done=once(ws,'close');assert.deepEqual(await received,Buffer.from([0xc1,4,0,1]));assert.equal((await done)[0],1000);
});
test('byte budget and idle deadline close both sides',{timeout:5000},async t=>{
 const {url}=await setup(t,()=>{},{maxBytesPerSecond:4,idleMs:1000});
 const a=await client(t,url);let closed=once(a,'close');a.send(Buffer.from([1,2,3,4,5]));assert.equal((await closed)[0],1009);
 const b=await client(t,url);closed=once(b,'close');assert.equal((await closed)[0],1001);
});
test('config cannot expose listener or arbitrary remote TCP targets',()=>{
 assert.throws(()=>createGateway({routes:{bad:{host:'example.com',port:4444}},origins:[origin]}));
 assert.throws(()=>createGateway({routes:{x:{host:'127.0.0.1',port:80}},origins:[]}));
});

test('browser transport adapter + WASM + real gateway/TCP work together',{timeout:5000},async t=>{
 const {url}=await setup(t,socket=>{
  let ready=false;const queued=[];
  socket.write(Buffer.from([0xc1]));
  socket.on('data',data=>{if(ready)socket.write(data);else queued.push(data);});
  setImmediate(()=>{if(socket.destroyed)return;socket.write(Buffer.from([4,0,1,0xc2,0,5,0xf4,6]));ready=true;for(const data of queued)socket.write(data);});
 });
 const frames=[];let session;
 const done=new Promise((resolve,reject)=>{
  session=new TransportSession({createSocket:address=>new WebSocket(address,{origin,perMessageDeflate:false}),
   createReader:()=>createReader(core),decodeTransport:b=>b,encodeTransport:b=>b,
   onState:({state,reason})=>{if(state==='open')session.send(Uint8Array.from([0xc3,4,0,0]));if(state==='failed')reject(new Error(reason));},
   onFrame:frame=>{frames.push([...frame]);if(frames.length===3)resolve();}});
 });
 t.after(()=>session.close());session.connect(url);await done;
 assert.deepEqual(frames.slice(0,2),[[0xc1,4,0,1],[0xc2,0,5,0xf4,6]]);
 assert.deepEqual(frames[2],[0xc3,4,0,0]);session.close();assert.equal(session.state,'closed');
});
test('heartbeat closes a client that does not pong',{timeout:5000},async t=>{
 const {url}=await setup(t,()=>{},{heartbeatMs:20,idleMs:2000});
 const ws=await client(t,url,{autoPong:false});const [code]=await once(ws,'close');assert.equal(code,1001);
});
