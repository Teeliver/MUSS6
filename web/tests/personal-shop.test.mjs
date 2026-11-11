import test from 'node:test';import assert from 'node:assert/strict';
import {createGameState} from '../client/src/protocol/game-state.js';
import {personalShopOpenPacket,personalShopBuyPacket,personalShopLeavePacket} from '../client/src/protocol/personal-shop.js';
const wire=[0,0,20,0,0,0,0,0,0,0,0,0];
const ascii=(s,n)=>{const o=new Uint8Array(n);o.set(new TextEncoder().encode(s));return o;};
function viewport(index,title){const b=new Uint8Array(44);b.set([0xc2,0,44,0x3f,0x00,1,index>>8&255,index&255]);b.set(ascii(title,36),8);return b;}
function list(index,name,title){const b=new Uint8Array(75);b.set([0xc2,0,75,0x3f,0x05,1,index>>8&255,index&255]);b.set(ascii(name,10),8);b.set(ascii(title,36),18);b[54]=1;b[55]=204;b.set(wire,56);b.set([0xa5,0x5a,0xff],68);new DataView(b.buffer).setUint32(71,123456,true);return b;}
test('personal shop client packets match C3:3F 05/06/07 layouts',()=>{
 assert.deepEqual([...personalShopOpenPacket(0x1234,'Shop')].slice(0,6),[0xc3,16,0x3f,0x05,0x12,0x34]);
 assert.equal(personalShopOpenPacket(1,'x').length,16);
 assert.deepEqual([...personalShopBuyPacket(0x1234,'Shop',204)].slice(0,6),[0xc3,17,0x3f,0x06,0x12,0x34]);
 assert.equal(personalShopBuyPacket(1,'x',204)[16],204);
 assert.deepEqual([...personalShopLeavePacket(0x1234,'Shop')].slice(0,6),[0xc3,16,0x3f,0x07,0x12,0x34]);
});
test('personal shop viewport title, item list, buy result, title change and leave update state',()=>{
 const state=createGameState();
 assert.equal(state.apply(viewport(0x1234,'Shop Title'),1),true);
 assert.equal(state.snapshot().shop.titles[0x1234],'Shop Title');
 assert.equal(state.apply(list(0x1234,'Seller','Shop Title'),1),true);
 const remote=state.snapshot().shop.remote;
 assert.equal(remote.index,0x1234);assert.equal(remote.name,'Seller');assert.equal(remote.title,'Shop Title');
 assert.deepEqual(remote.items[0],{slot:204,wire,price:123456});
 const buy=new Uint8Array(20);buy.set([0xc1,20,0x3f,0x06,1,0x12,0x34]);buy.set(wire,7);buy[19]=20;
 assert.equal(state.commitPersonalBuy(204,buy),1);
 assert.equal(state.snapshot().items[0].slot,20);assert.equal(state.snapshot().shop.remote,undefined);
 const title=new Uint8Array(52);title.set([0xc1,52,0x3f,0x10,0x12,0x34]);title.set(ascii('New Title',36),6);title.set(ascii('Seller',10),42);
 assert.equal(state.apply(title,1),true);assert.equal(state.snapshot().shop.titles[0x1234],'New Title');
 const leave=Uint8Array.of(0xc1,6,0x3f,0x12,0x12,0x34);
 assert.equal(state.apply(leave,1),true);assert.equal(state.snapshot().shop.remote,undefined);
});
test('seller close broadcast removes title and closes buyer purchase window',()=>{
 const state=createGameState();
 state.apply(viewport(0x1234,'Shop Title'),1);
 state.apply(list(0x1234,'Seller','Shop Title'),1);
 assert.equal(state.snapshot().shop.remote.open,true);
 const close=Uint8Array.of(0xc1,7,0x3f,3,1,0x12,0x34);
 assert.equal(state.apply(close,1),true);
 assert.equal(state.snapshot().shop.remote,undefined);
 assert.equal(state.snapshot().shop.titles[0x1234],undefined);
});
test('wide C2 seller-close broadcast and title change reach the buyer',()=>{
 const state=createGameState();
 state.apply(viewport(0x1234,'Shop Title'),1);
 state.apply(list(0x1234,'Seller','Shop Title'),1);
 const wideTitle=new Uint8Array(55);wideTitle.set([0xc2,0,55,0x3f,0x10,0x12,0x34]);wideTitle.set(ascii('Wide Title',36),7);wideTitle.set(ascii('Seller',10),43);
 assert.equal(state.apply(wideTitle,1),true);
 assert.equal(state.snapshot().shop.titles[0x1234],'Wide Title');
 const close=Uint8Array.of(0xc2,0,8,0x3f,3,1,0x12,0x34);
 assert.equal(state.apply(close,1),true);
 assert.equal(state.snapshot().shop.remote,undefined);
 assert.equal(state.snapshot().shop.titles[0x1234],undefined);
});
test('server leave notice 3F:12 and inventory-full buy result close the buyer window',()=>{
 const state=createGameState();
 state.apply(list(0x1234,'Seller','Shop Title'),1);
 state.apply(Uint8Array.of(0xc1,6,0x3f,0x12,0xff,0xff),1);
 assert.equal(state.snapshot().shop.remote,undefined);
 state.apply(list(0x1234,'Seller','Shop Title'),1);
 const full=new Uint8Array(20);full.set([0xc1,20,0x3f,6,6,0x12,0x34]);full.set(wire,7);full[19]=20;
 assert.equal(state.commitPersonalBuy(204,full),6);
 assert.equal(state.snapshot().shop.remote,undefined);
});
test('empty item list closes the purchase window instead of showing an empty grid',()=>{
 const state=createGameState();
 state.apply(list(0x1234,'Seller','Shop Title'),1);
 const empty=new Uint8Array(55);empty.set([0xc2,0,55,0x3f,0x13,1,0x12,0x34]);empty.set(ascii('Seller',10),8);empty.set(ascii('Shop Title',36),18);state.apply(empty,1);
 assert.equal(state.snapshot().shop.remote,undefined);
});
test('malformed personal shop frames are rejected',()=>{
 const state=createGameState();
 assert.throws(()=>state.apply(viewport(1,'x').slice(0,-1),1),/viewport size/);
 assert.throws(()=>state.apply(list(1,'x','y').slice(0,-1),1),/list size/);
 assert.throws(()=>state.apply(Uint8Array.of(0xc1,5,0x3f,0x12,0),1));
});

test('buyer session sends open request and publishes seller goods only after server ACK',async()=>{
 const {createWorldSession}=await import('../client/src/session/world-session.js'),{hudLayout:L}=await import('../client/src/protocol/hud-layout.js');
 const sent=[],updates=[],errors=[],world=createWorldSession({onState:s=>updates.push(s),onError:e=>errors.push(e),timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:b=>sent.push(b),close(){}});
 const entry=new Uint8Array(L.PMSG_CHARACTER_INFO_SEND.size);entry.set([0xc1,entry.length,0xf3,3,100,100,0,3]);const choosing=world.select({name:'Buyer'},0);world.receive(entry);await choosing;
 const info=new Uint8Array(L.PMSG_NEW_CHARACTER_INFO_SEND.size);info.set([0xc1,info.length,0xf3,224]);new DataView(info.buffer).setUint16(L.PMSG_NEW_CHARACTER_INFO_SEND.Level,1,true);world.receive(info);
 const open=world.personalShop('openOther',{index:0x1234,name:'Seller'});
 assert.deepEqual([...sent.at(-1)],[...personalShopOpenPacket(0x1234,'Seller')]);assert.equal(world.gameState.shop.remote,undefined);
 world.receive(list(0x1234,'Seller','My Shop'));assert.equal(await open,1);assert.equal(updates.at(-1).shop.remote.open,true);assert.equal(updates.at(-1).shop.remote.name,'Seller');assert.equal(updates.at(-1).shop.remote.items[0].price,123456);
 const purchase=world.personalShop('buy',{index:0x1234,name:'Seller',slot:204});assert.deepEqual([...sent.at(-1)],[...personalShopBuyPacket(0x1234,'Seller',204)]);
 const denied=new Uint8Array(20);denied.set([0xc1,20,0x3f,6,4,0x12,0x34]);world.receive(denied);assert.equal(await purchase,4);assert.equal(world.gameState.shop.remote.items.length,1);assert.equal(await world.personalShop('leave'),1);assert.deepEqual([...sent.at(-1)],[...personalShopLeavePacket(0x1234,'Seller')]);assert.equal(world.gameState.shop.remote,undefined);const reopen=world.personalShop('openOther',{index:0x1234,name:'Seller'});world.receive(list(0x1234,'Seller','My Shop'));assert.equal(await reopen,1);assert.deepEqual(errors,[]);world.close();
});

test('D Buy selection clicks a player and opens seller without sending movement/attack',async()=>{
 const {createWorldControls}=await import('../client/src/world/world-controls.js'),{createRouteMotion}=await import('../client/src/world/navigation.js'),{projectWorld}=await import('../client/src/world/picking.js');
 const events=new Map(),canvasEvents=new Map(),docEvents=new Map(),entry={map:0,x:100,y:100},terrain={flags:new Uint16Array(65536),height:()=>0,heightAt:()=>0},motion=createRouteMotion(entry,terrain),seller={kind:'player',index:258,name:'Seller',x:102,y:100,charset:[]};
 const target={addEventListener:(k,f)=>events.set(k,f),removeEventListener:k=>events.delete(k)},doc={addEventListener:(k,f)=>docEvents.set(k,f),removeEventListener:k=>docEvents.delete(k)},canvas={ownerDocument:doc,getBoundingClientRect:()=>({left:0,top:0,width:1280,height:720}),addEventListener:(k,f)=>canvasEvents.set(k,f),removeEventListener:k=>canvasEvents.delete(k)};let opened;
 const controls=createWorldControls({canvas,target,entry,terrain,motion,entities:()=>[seller],resources:{actors:{}},openShop:(index,name)=>opened={index,name},move:()=>assert.fail('Buy should not move'),attack:()=>assert.fail('Buy should not attack')});
 docEvents.get('mu-command-purchase')();const p=projectWorld({x:102.5,y:100.5,z:1},{width:1280,height:720,terrain,entry,focus:motion.position});canvasEvents.get('pointerdown')({button:0,clientX:p.x,clientY:p.y,preventDefault(){}});assert.deepEqual(opened,{index:258,name:'Seller'});controls.dispose();assert.equal(docEvents.has('mu-command-purchase'),false);
});

test('UPDATE603 shop ABI has native padding: 55-byte header, 20-byte item, price at offset 16',async()=>{
 const {hudLayout:L}=await import('../client/src/protocol/hud-layout.js'),{decodePersonalShopList}=await import('../client/src/protocol/personal-shop.js');
 assert.equal(L.PMSG_PSHOP_ITEM_LIST_SEND.size,55);assert.deepEqual(L.PMSG_PSHOP_ITEM_LIST,{size:20,slot:0,ItemInfo:1,value:16});
 const one=list(258,'Seller','Goods');assert.equal(one.length,75);assert.equal(decodePersonalShopList(one).items[0].price,123456);
 const two=new Uint8Array(95);two.set(one);two.set(one.slice(55),75);two[54]=2;two[2]=95;two[75]=205;new DataView(two.buffer).setUint32(91,7654321,true);assert.deepEqual(decodePersonalShopList(two).items.map(i=>i.price),[123456,7654321]);two[4]=0x13;assert.equal(decodePersonalShopList(two).items[1].slot,205);
 const packed=new Uint8Array(72);packed.set(one.slice(0,68));packed.set(one.slice(71),68);packed[2]=72;assert.throws(()=>decodePersonalShopList(packed),/item count/);
 const empty=one.slice(0,55);empty[2]=55;empty[54]=0;assert.equal(decodePersonalShopList(empty).items.length,0);
});

test('leaving another seller clears only remote shop, preserving own store and confirmed prices',()=>{
 const state=createGameState();state.apply(Uint8Array.of(0xc1,5,0x3f,2,1),1);state.apply(Uint8Array.of(0xc1,6,0x3f,1,1,204),1);
 state.apply(list(258,'Seller','Other Store'),1);const own=state.snapshot().shop;assert.equal(own.remote.name,'Seller');state.closePersonalShop();const after=state.snapshot().shop;assert.equal(after.remote,undefined);assert.equal(after.open,own.open);assert.deepEqual(after.prices,own.prices);assert.equal(after.title,own.title);
});
