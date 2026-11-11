import test from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import WebSocket from 'ws';
import { readFile } from 'node:fs/promises';
import createCore from '../../.build/wasm/frame-reader.mjs';
import { deriveGameKeys } from '../../client/src/protocol/game-profile.js';
import { createTransportCodec } from '../../client/src/protocol/connect-codec.js';
import { createGameProbe } from '../../client/src/session/game-probe.js';
import { createGateway } from '../src/gateway.mjs';
const core=await createCore({wasmBinary:await readFile(new URL('../../.build/wasm/frame-reader.wasm',import.meta.url))});
const origin='http://127.0.0.1:8080';
const serial='1234567890123456'; // Synthetic test profile, not runtime configuration.
async function fixture(t,{mhp=false,code=1,result=1,silent=false}={}) {
 const sockets=new Set();let bytesFromClient=0;
 const profile={serial,rawMhp1:mhp?1:0,rawMhp2:mhp?255:0};
 const codec=createTransportCodec(core,deriveGameKeys(core,profile));
 const wire=codec.encodeTransport(Uint8Array.from([0xc1,14,0xf1,0,result,0x2e,0x7c,49,48,50,48,48,code&255,code>>8]));
 const tcp=net.createServer(socket=>{
  sockets.add(socket);socket.on('error',()=>{});socket.on('close',()=>sockets.delete(socket));socket.on('data',b=>{bytesFromClient+=b.length;});
  if(!silent){socket.write(wire.subarray(0,1));setImmediate(()=>{if(!socket.destroyed)socket.write(wire.subarray(1));});}
 });
 await new Promise(resolve=>tcp.listen(0,'127.0.0.1',resolve));
 const gateway=createGateway({routes:{'game-one':{host:'127.0.0.1',port:tcp.address().port}},origins:[origin]});
 const address=await gateway.listen();
 t.after(async()=>{await gateway.close();for(const socket of sockets)socket.destroy();await new Promise(resolve=>tcp.close(resolve));});
 const endpoint={address:'127.0.0.1',port:tcp.address().port};
 const probe=createGameProbe({core,createSocket:url=>new WebSocket(url,{origin,perMessageDeflate:false}),gatewayUrl:`ws://127.0.0.1:${address.port}`,routes:{1:{id:'game-one',...endpoint}},deadlineMs:silent?150:1000});
 return {probe,profile,selected:{code:1,endpoint},get bytesFromClient(){return bytesFromClient;}};
}
test('game greeting encrypted with derived keys passes WS/TCP with MHP off/on',{timeout:5000},async t=>{
 for(const mhp of [false,true]){
  const f=await fixture(t,{mhp});
  assert.deepEqual(await f.probe(f.selected,f.profile),{index:11900,serverCode:1,encodedVersion:[49,48,50,48,48]});
  assert.equal(f.bytesFromClient,0);
 }
});
test('network probe rejects server denial/code mismatch/key mismatch',{timeout:5000},async t=>{
 for(const options of [{result:0},{code:2}]){
  const f=await fixture(t,options);await assert.rejects(f.probe(f.selected,f.profile),/rejected/);assert.equal(f.bytesFromClient,0);
 }
 const f=await fixture(t,{mhp:true});await assert.rejects(f.probe(f.selected,{...f.profile,rawMhp1:2}));assert.equal(f.bytesFromClient,0);
});
test('silent GameServer fixture times out without sending login data',{timeout:5000},async t=>{
 const f=await fixture(t,{silent:true});await assert.rejects(f.probe(f.selected,f.profile),/timeout/);assert.equal(f.bytesFromClient,0);
});
