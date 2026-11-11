import test from 'node:test';
import assert from 'node:assert/strict';
import {createPacketCipher} from '../client/src/platform/wasm/create-packet-cipher.js';
import {core,key,setup,transport,start} from './helpers/game-fixture.mjs';
test('game channel composes real WASM packet crypto and transport across all splits',()=>{
 const peer=createPacketCipher(core,{encryptionKeyFile:key(3),decryptionKeyFile:key(21846)});
 const encrypted=peer.encode(Uint8Array.of(0xc3,5,0xf1,1,7));
 // Expected is server-style raw payload after modulus decode: no inbound XOR removal.
 const decoder=createPacketCipher(core,{encryptionKeyFile:key(3),decryptionKeyFile:key(21846)});
 const expected=[...decoder.decode(encrypted)];decoder.delete();peer.delete();
 const wire=transport().encodeTransport(encrypted);
 for(let split=1;split<wire.length;split++){
  const t=setup();t.enc.fill(0);t.dec.fill(0);start(t);
  t.socket.emit('message',wire.slice(0,split));assert.equal(t.packets.length,0);
  t.socket.emit('message',wire.slice(split));assert.deepEqual(t.packets,[expected]);
  t.channel.send(Uint8Array.of(0xc3,5,0xf1,1,7));
  assert.deepEqual(t.socket.sent[0],wire);t.channel.close();assert.equal(t.jobs.size,0);assert.equal(t.socket.closed,1);
 }
});
test('game channel replay and consumer errors terminate without later delivery',()=>{
 const peer=createPacketCipher(core,{encryptionKeyFile:key(3),decryptionKeyFile:key(21846)});
 const wire=transport().encodeTransport(peer.encode(Uint8Array.of(0xc3,4,0xf1,1)));peer.delete();
 const t=setup();start(t);t.socket.emit('message',wire);t.socket.emit('message',wire);
 assert.equal(t.packets.length,1);assert.equal(t.channel.state,'failed');assert.equal(t.jobs.size,0);
 assert.throws(()=>t.channel.send(Uint8Array.of(0xc1,3,1)));t.channel.close();assert.equal(t.socket.closed,1);
 const bad=setup({onPacket(){throw new Error('consumer');}});start(bad);bad.socket.emit('message',transport().encodeTransport(Uint8Array.of(0xc1,3,1)));assert.equal(bad.channel.state,'failed');
});
test('game channel cancellation, initialization failure and timeout release session',()=>{
 const before=setup();before.channel.close();assert.throws(()=>before.channel.connect());assert.equal(before.created,0);
 const invalid=setup({encryptionKeyFile:new Uint8Array(54)});invalid.channel.connect();assert.equal(invalid.channel.state,'failed');assert.equal(invalid.created,0);
 const timeout=setup();start(timeout);timeout.socket.emit('message',transport().encodeTransport(Uint8Array.of(0xc3)));[...timeout.jobs.values()][0]();assert.equal(timeout.channel.state,'failed');assert.equal(timeout.socket.closed,1);
 const a=setup(),b=setup();start(a);start(b);a.channel.send(Uint8Array.of(0xc3,4,1,1));b.channel.send(Uint8Array.of(0xc3,4,1,1));assert.deepEqual(a.socket.sent,b.socket.sent);a.channel.close();b.channel.close();
});

test('game channel coalesced plain frames and invalid outbound stop the channel',()=>{
 const t=setup();start(t);
 t.socket.emit('message',transport().encodeTransport(Uint8Array.of(0xc1,3,1,0xc2,0,4,2)));
 assert.deepEqual(t.packets,[[0xc1,3,1],[0xc2,0,4,2]]);
 assert.throws(()=>t.channel.send(Uint8Array.of(0xc3,5,1)));
 assert.equal(t.channel.state,'failed');assert.equal(t.socket.sent.length,0);assert.equal(t.jobs.size,0);
 t.socket.emit('message',transport().encodeTransport(Uint8Array.of(0xc1,3,3)));
 assert.equal(t.packets.length,2);
});
