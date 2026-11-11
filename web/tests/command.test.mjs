import test from 'node:test';
import assert from 'node:assert/strict';
import {guildRequestPacket,guildAnswerPacket,guildRelationPacket,duelStartPacket,duelAnswerPacket,duelEndPacket,decodeCommandPacket} from '../client/src/protocol/command.js';

const ascii=(s,n)=>{const out=new Uint8Array(n);for(let i=0;i<n&&i<s.length;i++)out[i]=s.charCodeAt(i);return out;};

test('guild command packets match UPDATE603 Guild.h',()=>{
  assert.deepEqual([...guildRequestPacket(0x1234)],[0xc1,5,0x50,0x12,0x34]);
  assert.deepEqual([...guildAnswerPacket(1,0x1234)],[0xc1,6,0x51,1,0x12,0x34]);
  assert.deepEqual([...guildRelationPacket(1,1,0x1234)],[0xc1,7,0xe5,1,1,0x12,0x34]);
  assert.deepEqual([...guildRelationPacket(2,2,0x00ff)],[0xc1,7,0xe5,2,2,0,0xff]);
});

test('duel packets use C3 because HackPacketCheck requires encryption',()=>{
  const start=duelStartPacket(0x1234,'Hero');
  const expect=new Uint8Array(16);expect.set([0xc3,16,0xaa,1,0x12,0x34]);expect.set(ascii('Hero',10),6);
  assert.deepEqual([...start],[...expect]);
  const answer=duelAnswerPacket(1,0x1234,'Hero');
  const answerExpect=new Uint8Array(17);answerExpect.set([0xc3,17,0xaa,2,1,0x12,0x34]);answerExpect.set(ascii('Hero',10),7);
  assert.deepEqual([...answer],[...answerExpect]);
  assert.deepEqual([...duelEndPacket()],[0xc3,4,0xaa,3]);
});

test('decodes guild, union and duel results from normalized C1',()=>{
  assert.deepEqual(decodeCommandPacket(Uint8Array.of(0xc1,5,0x50,0x12,0x34)),{kind:'guildRequest',index:0x1234});
  assert.deepEqual(decodeCommandPacket(Uint8Array.of(0xc1,4,0x51,7)),{kind:'guild',result:7});
  assert.deepEqual(decodeCommandPacket(Uint8Array.of(0xc1,8,0xe6,1,1,0x11,0x12,0x34)),{kind:'union',relation:1,type:1,result:0x11,index:0x1234});
  const start=new Uint8Array(17);start.set([0xc1,17,0xaa,1,0,0x12,0x34]);start.set(ascii('Hero',10),7);
  assert.deepEqual(decodeCommandPacket(start),{kind:'duel',sub:1,result:0,index:0x1234,name:'Hero'});
  const request=new Uint8Array(16);request.set([0xc1,16,0xaa,2,0x12,0x34]);request.set(ascii('Hero',10),6);
  assert.deepEqual(decodeCommandPacket(request),{kind:'duel',sub:2,index:0x1234,name:'Hero'});
  assert.deepEqual(decodeCommandPacket(Uint8Array.of(0xc1,5,0xaa,3,0)),{kind:'duel',sub:3,result:0});
  assert.equal(decodeCommandPacket(Uint8Array.of(0xc1,3,0x42)),null);
});

test('rejects malformed command packets',()=>{
  assert.throws(()=>decodeCommandPacket(Uint8Array.of(0xc1,3,0x51)),/guild result/);
  assert.throws(()=>decodeCommandPacket(Uint8Array.of(0xc1,9,0xe6,1,1,1,0,0,0)),/union result/);
  assert.throws(()=>decodeCommandPacket(Uint8Array.of(0xc1,6,0xaa,3,0,0)),/duel end/);
  assert.throws(()=>duelStartPacket(0x4000,'Hero'),/index/);
});
