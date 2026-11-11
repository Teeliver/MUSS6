import test from 'node:test';import assert from 'node:assert/strict';
import {tradeRequestPacket,tradeAnswerPacket,tradeMoneyPacket,tradeConfirmPacket,tradeExitPacket,tradeMovePacket,tradeReturnPacket,decodeTradeRequest,decodeTradeResponse,decodeTradeItemAdd,decodeTradeItemDel,decodeTradeMoney,decodeTradeConfirm,decodeTradeResult} from '../client/src/protocol/trade.js';
const wire=[0,0,20,0,0,0,0,0,0,0,0,0];
const ascii=(s,n)=>{const o=new Uint8Array(n);o.set(new TextEncoder().encode(s));return o;};
function response(result=1,name='Seller',level=300,guild=7){const b=new Uint8Array(20);b.set([0xc1,20,0x37,result]);b.set(ascii(name,10),4);new DataView(b.buffer).setUint16(14,level,true);new DataView(b.buffer).setUint32(16,guild,true);return b;}
test('trade packets match C3 36/3C/3D, C1 37/3A and item move flags',()=>{
 assert.deepEqual([...tradeRequestPacket(0x1234)],[0xc3,5,0x36,0x12,0x34]);
 assert.deepEqual([...tradeAnswerPacket(true)],[0xc1,4,0x37,1]);
 assert.deepEqual([...tradeMoneyPacket(123456)],[0xc1,8,0x3a,0,0x40,0xe2,0x01,0x00]);
 assert.deepEqual([...tradeConfirmPacket(true)],[0xc3,4,0x3c,1]);
 assert.deepEqual([...tradeExitPacket()],[0xc3,3,0x3d]);
 assert.equal(tradeMovePacket(12,0,wire)[17],1);
 assert.equal(tradeReturnPacket(0,12,wire)[17],0);
});
test('trade response, item, money, confirm and result decoders',()=>{
 assert.deepEqual(decodeTradeResponse(response()),{response:1,name:'Seller',level:300,guild:7});
 assert.deepEqual(decodeTradeRequest(Uint8Array.from([0xc3,13,0x36,...ascii('Buyer',10)])),{name:'Buyer'});assert.deepEqual(decodeTradeRequest(Uint8Array.from([0xc1,13,0x36,...ascii('Buyer',10)])),{name:'Buyer'});
 assert.deepEqual(decodeTradeItemAdd(Uint8Array.from([0xc1,16,0x39,3,...wire])),{slot:3,wire});
 assert.deepEqual(decodeTradeItemDel(Uint8Array.of(0xc1,4,0x38,3)),{slot:3});
 assert.deepEqual(decodeTradeMoney(Uint8Array.of(0xc1,8,0x3b,0,0x40,0xe2,0x01,0x00)),{money:123456});
 assert.deepEqual(decodeTradeConfirm(Uint8Array.of(0xc1,4,0x3c,1)),{flag:1});
 assert.deepEqual(decodeTradeResult(Uint8Array.of(0xc1,4,0x3d,2)),{result:2});
});
test('session opens trade, updates partner items and closes on result',async()=>{
 const {createWorldSession}=await import('../client/src/session/world-session.js'),{hudLayout:L}=await import('../client/src/protocol/hud-layout.js');
 const sent=[],states=[],errors=[],world=createWorldSession({onState:s=>states.push(s),onError:e=>errors.push(e),timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:b=>sent.push(b),close(){}});
 const entry=new Uint8Array(L.PMSG_CHARACTER_INFO_SEND.size);entry.set([0xc1,entry.length,0xf3,3,100,100,0,3]);const choosing=world.select({name:'Buyer'},0);world.receive(entry);await choosing;
 const info=new Uint8Array(L.PMSG_NEW_CHARACTER_INFO_SEND.size);info.set([0xc1,info.length,0xf3,224]);new DataView(info.buffer).setUint16(L.PMSG_NEW_CHARACTER_INFO_SEND.Level,1,true);world.receive(info);
 world.tradeRequest(0x1234);assert.deepEqual([...sent.at(-1)],[...tradeRequestPacket(0x1234)]);
 world.receive(response());assert.equal(world.gameState.trade.open,true);assert.equal(world.gameState.trade.partner.name,'Seller');
 world.receive(Uint8Array.from([0xc1,16,0x39,2,...wire]));assert.deepEqual(world.gameState.trade.yourItems[0],{slot:2,wire});
 world.receive(Uint8Array.of(0xc1,8,0x3b,0,0x40,0xe2,0x01,0x00));assert.equal(world.gameState.trade.yourMoney,123456);
 world.receive(Uint8Array.of(0xc1,4,0x3c,1));assert.equal(world.gameState.trade.yourConfirm,true);
 world.receive(Uint8Array.of(0xc1,4,0x3d,1));assert.equal(world.gameState.trade.open,false);assert.equal(world.gameState.trade.result,1);
 assert.deepEqual(errors,[]);world.close();
});
