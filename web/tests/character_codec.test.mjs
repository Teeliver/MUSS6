import test from 'node:test';
import assert from 'node:assert/strict';
import {createCharacterCodec} from '../client/src/protocol/character-codec.js';
import {core} from './helpers/game-fixture.mjs';
test('character WASM codec preserves charset ownership, slots, levels and policy',()=>{
 const codec=createCharacterCodec(core);assert.deepEqual([...codec.listRequest()],[0xC1,4,0xF3,0]);
 const b=new Uint8Array(42);b.set([0xC1,42,0xF3,0,3,2,1,4]);b[8]=4;
 b.set(new TextEncoder().encode('TenLetters'),9);b[19]=255;b[20]=0x90;b[21]=1;b[22]=18;
 b.set(Array.from({length:18},(_,i)=>i+32),23);b[41]=255;
 const result=codec.roster(b);assert.equal(result.characters[0].name,'TenLetters');assert.equal(result.characters[0].level,400);
 assert.equal(result.characters[0].slot,4);assert.equal(result.characters[0].guildStatus,255);
 assert.deepEqual([...result.characters[0].charset],Array.from({length:18},(_,i)=>i+32));
 const valid=b.slice();
 for(const [offset,value] of [[0,0xC3],[6,2],[8,5],[9,0],[9,0xFF]]){
  const invalid=valid.slice();invalid[offset]=value;assert.throws(()=>codec.roster(invalid));
 }
 const trailing=new Uint8Array(43);trailing.set(valid);trailing[1]=43;assert.throws(()=>codec.roster(trailing));
 b.fill(0);assert.equal(result.characters[0].charset[0],32);
 assert.deepEqual(codec.policy(Uint8Array.of(0xC1,14,0xDE,0,7,255,1,0,2,0,0x90,1,0x2C,1)),
 {flag:0,classCode:7,experienceA:1,experienceB:2,maxLevel:400,deleteMaxLevel:300});
 for(const bytes of [Uint8Array.of(0xC1,8,0xF3,0,0,0,6,0),new Uint8Array(7)])assert.throws(()=>codec.roster(bytes));
 assert.throws(()=>codec.policy(new Uint8Array(13)));
});
