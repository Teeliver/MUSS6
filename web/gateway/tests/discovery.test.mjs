import test from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import WebSocket from 'ws';
import { readFile } from 'node:fs/promises';
import createCore from '../../.build/wasm/frame-reader.mjs';
import { createReader } from '../../client/src/platform/wasm/create-reader.js';
import { createDiscovery } from '../../client/src/session/discovery.js';
import { createGateway } from '../src/gateway.mjs';
const core=await createCore({wasmBinary:await readFile(new URL('../../.build/wasm/frame-reader.wasm',import.meta.url))});
const origin='http://127.0.0.1:8080';
async function fixture(t,{wrongEndpoint=false,silent=false}={}) {
 const sockets=new Set(),requests=[];let connections=0;
 const tcp=net.createServer(socket=>{
  connections++;sockets.add(socket);const reader=createReader(core);
  socket.on('error',()=>{});socket.on('close',()=>{reader.delete();sockets.delete(socket);});
  if(!silent){socket.write(Buffer.from([0xc1]));setImmediate(()=>{if(!socket.destroyed)socket.write(Buffer.from([4,0,1]));});}
  socket.on('data',data=>{
   try {for(const frame of reader.feed(new Uint8Array(data))){
    requests.push([...frame]);
    if(Buffer.from(frame).equals(Buffer.from([0xc1,4,0xf4,6])))socket.write(Buffer.from([0xc2,0,15,0xf4,6,0,2,1,0,42,0xcc,2,0,7,0xcc]));
    else if(Buffer.from(frame).equals(Buffer.from([0xc1,5,0xf4,3,1]))){
     const endpoint=Buffer.alloc(22);endpoint.set([0xc1,22,0xf4,3]);endpoint.write('127.0.0.1',4);endpoint.writeUInt16LE(wrongEndpoint?80:55901,20);socket.write(endpoint);
    }else socket.destroy();
   }}catch{socket.destroy();}
  });
 });
 await new Promise(resolve=>tcp.listen(0,'127.0.0.1',resolve));
 const gateway=createGateway({routes:{connect:{host:'127.0.0.1',port:tcp.address().port}},origins:[origin]});
 const address=await gateway.listen();
 t.after(async()=>{await gateway.close();for(const socket of sockets)socket.destroy();await new Promise(resolve=>tcp.close(resolve));});
 const api=createDiscovery({core,createSocket:url=>new WebSocket(url,{origin,perMessageDeflate:false}),
  connectUrl:`ws://127.0.0.1:${address.port}/mu/connect`,routes:{1:{id:'game-one',address:'127.0.0.1',port:55901}},deadlineMs:200});
 return {api,requests,get connections(){return connections;}};
}
test('full discovery then select use independent CS connections through gateway',{timeout:5000},async t=>{
 const f=await fixture(t);
 assert.deepEqual(await f.api.discover(),[{code:1,load:42,type:0xcc},{code:2,load:7,type:0xcc}]);
 const selected=await f.api.select(1);assert.equal(selected.routePath,'/mu/game-one');assert.equal(selected.endpoint.port,55901);
 assert.equal(f.connections,2);assert.deepEqual(f.requests,[[0xc1,4,0xf4,6],[0xc1,5,0xf4,3,1]]);
 // Selecting resolves metadata only: no game connection is attempted.
 await assert.rejects(f.api.select(2));assert.equal(f.connections,2);
});
test('real network wrong endpoint and silent handshake are rejected',{timeout:5000},async t=>{
 const mismatch=await fixture(t,{wrongEndpoint:true});await assert.rejects(mismatch.api.select(1),/rejected/);
 const silent=await fixture(t,{silent:true});await assert.rejects(silent.api.discover(),/timeout/);
});
