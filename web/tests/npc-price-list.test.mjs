import test from 'node:test';import assert from 'node:assert/strict';
import {createNpcShop} from '../client/src/protocol/npc-shop.js';
test('NPC price list C1:F3:E7 is parsed into prices/currency',()=>{
 const ns=createNpcShop();
 ns.patch({open:true,result:0,npc:{index:1}});
 const b=new Uint8Array(27);b.set([0xc1,0,27,0xf3,0xe7,1,2]);
 const d=new DataView(b.buffer);d.setUint32(7,123,true);d.setUint32(11,4,true);d.setUint32(15,255,true);d.setUint32(19,0,true);d.setUint32(23,9999,true);
 assert.equal(ns.apply(b),true);
 const s=ns.snapshot();
 assert.equal(s.currency,2);assert.equal(s.prices.length,1);assert.deepEqual(s.prices[0],{index:123,level:4,durability:255,excellent:0,price:9999});
});
