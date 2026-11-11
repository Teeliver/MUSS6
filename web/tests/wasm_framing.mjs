// Explicit integration command, not silently skipped when toolchain/build is absent.
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { execFileSync } from 'node:child_process';
import { checkFraming } from '../client/src/diagnostics/framing.js';
import createModule from '../.build/wasm/frame-reader.mjs';
const base=new URL('../.build/wasm/',import.meta.url);
const module=await createModule({wasmBinary:await readFile(new URL('frame-reader.wasm',base))});
assert.deepEqual(checkFraming(module), {frames:4, splitCases:19});
const safe = (await import('../client/src/platform/wasm/create-reader.js')).createReader(module);
assert.throws(()=>safe.feed(Uint8Array.of(0)), Error);
assert.equal(safe.failed(),true);safe.delete();safe.delete();assert.throws(()=>safe.finish(),Error);
const fixtures=[[0xc1,4,0,1],[0xc2,0,5,0xf4,6],[0xc3,4,0,0],[0xc4,0,5,0,0]];
const stream=Uint8Array.from(fixtures.flat());
const own=(frames)=>frames.map(frame=>Array.from(frame));
for(let cut=0;cut<=stream.length;cut++){
 const reader=new module.FrameReader();
 try {
  const out=[...reader.feed(stream.subarray(0,cut)),...reader.feed(stream.subarray(cut))];
  assert.deepEqual(own(out),fixtures);reader.finish();assert.equal(reader.pendingSize(),0);
 } finally{reader.delete();}
}
const reader=new module.FrameReader();
const input=Uint8Array.from(fixtures[0]);const out=reader.feed(input);input.fill(0);reader.delete();
assert.deepEqual(own(out),[fixtures[0]]);
for(const invalid of [[0],[0xc1,2],[0xc2,0,3],[0xc4,0x40,1]]){
 const r=new module.FrameReader();try{assert.throws(()=>r.feed(Uint8Array.from(invalid)));assert.equal(r.failed(),true);assert.throws(()=>r.feed(stream));}finally{r.delete();}
}
const partial=new module.FrameReader();try{partial.feed(Uint8Array.from([0xc1]));assert.throws(()=>partial.finish());}finally{partial.delete();}
const output=execFileSync(process.execPath,[new URL('contract.cjs',base).pathname],{encoding:'utf8'});
assert.match(output,/frame-reader contract passed/);
console.log('WASM bridge checks passed; identical C++ contract passed in WASM.');

const {createConnectCodec,createTransportCodec}=await import('../client/src/protocol/connect-codec.js');
const cs=createConnectCodec(module);
cs.handshake(Uint8Array.from([0xc1,4,0,1]));
assert.deepEqual([...cs.listRequest()],[0xc1,4,0xf4,6]);
assert.deepEqual(cs.list(Uint8Array.from([0xc2,0,11,0xf4,6,0,1,1,0,50,0xcc])),[{code:1,load:50,type:0xcc}]);
assert.throws(()=>cs.endpointRequest(256),RangeError);
assert.throws(()=>cs.list(Uint8Array.from([0xc2,0,7,0xf4,6,0,2])),Error);
const codec=createTransportCodec(module,{key1:0x12,key2:0x34,mhp:true,mhp1:0x56,mhp2:0x78});
const bytes=Uint8Array.from({length:256},(_,i)=>i);
assert.deepEqual(codec.decodeTransport(codec.encodeTransport(bytes)),bytes);
assert.deepEqual([...codec.encodeTransport(Uint8Array.from([0,1,127,128,255]))],[100,101,251,228,123]);
assert.throws(()=>createTransportCodec(module,{key1:999,key2:0,mhp:false,mhp1:0,mhp2:0}));
assert.match(execFileSync(process.execPath,[new URL('protocol-contract.cjs',base).pathname],{encoding:'utf8'}),/connect-protocol contract passed/);
console.log('ConnectServer/transport codec JS-WASM checks and native-equivalent C++ contract passed.');

const {deriveGameKeys,parseGameGreeting}=await import('../client/src/protocol/game-profile.js');
for(const raw of [[0,0],[1,0],[0,255],[255,255]]){
 const keys=deriveGameKeys(module,{serial:'1234567890123456',rawMhp1:raw[0],rawMhp2:raw[1]});
 const derivedCodec=createTransportCodec(module,keys);assert.deepEqual(derivedCodec.decodeTransport(derivedCodec.encodeTransport(bytes)),bytes);
}
const greeting=Uint8Array.from([0xc1,14,0xf1,0,1,0x2e,0x7c,49,48,50,48,48,0x34,0x12]);
assert.deepEqual(parseGameGreeting(module,greeting,4660),{index:11900,serverCode:4660,encodedVersion:[49,48,50,48,48]});
assert.throws(()=>parseGameGreeting(module,greeting,1));
assert.throws(()=>deriveGameKeys(module,{serial:'short',rawMhp1:0,rawMhp2:0}));
console.log('Game profile derivation/source-reference and EXTRA1 greeting checks passed.');

const {createIdentityCodec,prepareIdentityPacket}=await import('../client/src/protocol/client-identity.js');
const identityCodec=createIdentityCodec(module);
const syntheticIdentity='01234567-89ABCDEF-01234567-89ABCDEF-01234567';
const identityPacket=await prepareIdentityPacket(module,async()=>syntheticIdentity);
assert.equal(identityPacket.length,49);assert.deepEqual([...identityPacket.subarray(0,4)],[0xc1,49,0xf3,9]);assert.equal(identityPacket[48],0);
assert.deepEqual(identityCodec.decodePlain(identityCodec.encodePlain(identityPacket)),identityPacket);
await assert.rejects(prepareIdentityPacket(module));
await assert.rejects(prepareIdentityPacket(module,async()=>syntheticIdentity.toLowerCase()));
assert.throws(()=>identityCodec.encodePlain(Uint8Array.of(0xc3,4,0xf1,1)));
console.log('Identity wire format and C1/C2 XOR checks passed; no identity provider was installed.');

const {createPacketCipher}=await import('../client/src/platform/wasm/create-packet-cipher.js');
function syntheticModulusKey(decrypt=false){
 const result=new Uint8Array(54),view=new DataView(result.buffer);view.setUint16(0,4370,true);view.setUint32(2,54,true);
 const masks=[0x3F08A79B,0xE25CC287,0x93D27AB9,0x20DEA7BF],tables=[[65537,65537,65537,65537],Array(4).fill(decrypt?21846:3),[0x1234,0xabcd,0x5678,0x1357]];
 for(let table=0;table<3;table++)for(let i=0;i<4;i++)view.setUint32(6+table*16+i*4,(tables[table][i]^masks[i])>>>0,true);
 return result;
}
const packetCipher=createPacketCipher(module,{encryptionKeyFile:syntheticModulusKey(),decryptionKeyFile:syntheticModulusKey(true)});
const encrypted=packetCipher.encode(Uint8Array.from([0xc3,5,0xf1,1,1]));
assert.equal(encrypted[0],0xc3);assert.equal(packetCipher.status().sent,1);
const normalized=packetCipher.decode(encrypted);
assert.deepEqual(identityCodec.decodePlain(normalized),Uint8Array.from([0xc1,5,0xf1,1,1]));
assert.equal(packetCipher.status().received,1);
assert.throws(()=>packetCipher.decode(encrypted));assert.equal(packetCipher.status().failed,true);
packetCipher.delete();packetCipher.delete();assert.throws(()=>packetCipher.status());
assert.throws(()=>createPacketCipher(module,{encryptionKeyFile:new Uint8Array(54),decryptionKeyFile:syntheticModulusKey(true)}));
assert.match(execFileSync(process.execPath,[new URL('modulus-contract.cjs',base).pathname],{encoding:'utf8'}),/simple-modulus contract passed/);
console.log('SimpleModulus/source-reference, key-file and serial checks passed in WASM.');

// Account ABI/reference contract uses only synthetic credentials.
assert.match(execFileSync(process.execPath,[new URL('account-contract.cjs',base).pathname],{encoding:'utf8'}),/account protocol contract passed/);
console.log('EX603 account layout and server credential-transform reference passed in WASM.');
assert.match(execFileSync(process.execPath,[new URL('character-contract.cjs',base).pathname],{encoding:'utf8'}),/character protocol contract passed/);
console.log('EX603 character-list and creation-policy ABI contract passed in WASM.');
assert.match(execFileSync(process.execPath,[new URL('appearance-contract.cjs',base).pathname],{encoding:'utf8'}),/appearance contract passed/);
console.log('Appearance decoder/source packer contract passed in WASM.');
assert.match(execFileSync(process.execPath,[new URL('companions-contract.cjs',base).pathname],{encoding:'utf8'}),/companions contract passed/);
console.log('Wing/pet decoder/source packer contract passed in WASM.');
assert.match(execFileSync(process.execPath,[new URL('item-resources-contract.cjs',base).pathname],{encoding:'utf8'}),/item resource contract passed/);
console.log('Portable item resource domain contract passed in WASM; native comparison is host-only.');
assert.match(execFileSync(process.execPath,[new URL('bmd-model-contract.cjs',base).pathname],{encoding:'utf8'}),/BMD model contract passed/);
console.log('Direct BMD v10/v12 mesh/bone/animation parser contract passed in WASM.');
assert.match(execFileSync(process.execPath,[new URL('texture-contract.cjs',base).pathname],{encoding:'utf8'}),/texture contract passed/);
console.log('OZT channel/alpha/origin contract passed in WASM.');

assert.match(execFileSync(process.execPath,[new URL('armor-resources-contract.cjs',base).pathname],{encoding:'utf8'}),/armor resource contract passed/);

assert.match(execFileSync(process.execPath,[new URL('equipment-grip-contract.cjs',base).pathname],{encoding:'utf8'}),/equipment grip contract passed: 49152 cases/);

assert.match(execFileSync(process.execPath,[new URL('attachment-geometry-contract.cjs',base).pathname],{encoding:'utf8'}),/attachment geometry contract passed/);

assert.match(execFileSync(process.execPath,[new URL('equipment-actions-contract.cjs',base).pathname],{encoding:'utf8'}),/equipment actions contract passed/);

assert.match(execFileSync(process.execPath,[new URL('animation-pose-contract.cjs',base).pathname],{encoding:'utf8'}),/animation pose contract passed/);

assert.match(execFileSync(process.execPath,[new URL('motion-animation-contract.cjs',base).pathname],{encoding:'utf8'}),/motion animation contract passed/);
