import test from 'node:test';import assert from 'node:assert/strict';
import {itemUsePacket} from '../client/src/protocol/action-codec.js';
import {createWorldSession} from '../client/src/session/world-session.js';
import {jewelAction,jewelCanUse,jewelActionText,JEWEL_OF_BLESS,JEWEL_OF_SOUL,JEWEL_OF_LIFE,JEWEL_OF_HARMONY} from '../client/src/ui/jewel-use-policy.js';
test('item use packet carries jewel source and item target slots',()=>{
 assert.deepEqual([...itemUsePacket(12)],[195,6,38,12,0,0]);
 assert.deepEqual([...itemUsePacket(12,3)],[195,6,38,12,3,0]);
 assert.throws(()=>itemUsePacket(11,3));assert.throws(()=>itemUsePacket(12,204));
});
test('upgrade jewels map to the server direct-use branches and level caps',()=>{
 assert.equal(jewelAction(JEWEL_OF_BLESS),'bless');assert.equal(jewelAction(JEWEL_OF_SOUL),'soul');
 assert.equal(jewelAction(JEWEL_OF_LIFE),'life');assert.equal(jewelAction(JEWEL_OF_HARMONY),'harmony');
 assert.equal(jewelAction(14*512+15),undefined);
 assert.equal(jewelCanUse('bless',{level:5}),true);assert.equal(jewelCanUse('bless',{level:6}),false);
 assert.equal(jewelCanUse('soul',{level:8}),true);assert.equal(jewelCanUse('soul',{level:9}),false);
 assert.equal(jewelCanUse('life',{level:0}),true);assert.equal(jewelCanUse('harmony',{level:0}),true);
 assert.match(jewelActionText('soul'),/Jewel of Soul/);
});
test('session sends C1:26 with the dragged jewel and target item, never moves the jewel',async()=>{
 const sent=[],world=createWorldSession({timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:bytes=>sent.push([...bytes]),close(){}});
 const entry=new Uint8Array(136);entry.set([193,136,243,3,100,100,0,1]);new DataView(entry.buffer).setUint32(80,100,true);
 const ready=world.select({name:'Tester'},0);world.receive(entry);await ready;
 const jewel=[13,0,1,0,0,0xE0,0,255,255,255,255,255],weapon=[0,0,20,0,0,0,0,255,255,255,255,255];
 const list=new Uint8Array(6+2*13);list.set([194,0,list.length,243,16,2],0);list.set([12,...jewel,13,...weapon],6);world.receive(list);
 assert.equal(world.gameState.items.length,2);
 world.useItemOn(12,13);
 assert.deepEqual(sent.at(-1),[195,6,38,12,13,0]);
 assert.equal(world.gameState.items.find(i=>i.slot===12).index,JEWEL_OF_BLESS);
 await assert.rejects(async()=>world.useItemOn(12,14),/vật phẩm/i);
 world.close();
});
