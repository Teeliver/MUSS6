import test from 'node:test';import assert from 'node:assert/strict';
import {friendListPacket,friendAddPacket,friendAcceptPacket,friendDeletePacket,decodeFriendList,decodeFriendRequest,decodeFriendAcceptRequest,decodeFriendDelete,decodeFriendState} from '../client/src/protocol/friend.js';
const ascii=(s,n)=>{const o=new Uint8Array(n);o.set(new TextEncoder().encode(s));return o;};
function list(entries){const size=7+entries.length*11,b=new Uint8Array(size);b.set([0xc2,size>>8&255,size&255,0xc0,0,50,entries.length]);entries.forEach((e,i)=>{b.set(ascii(e.name,10),7+i*11);b[7+i*11+10]=e.server??0;});return b;}
test('friend packets match C1 C0/C1/C2/C3 layouts',()=>{
 assert.deepEqual([...friendListPacket()],[0xc1,3,0xc0]);
 assert.deepEqual([...friendAddPacket('Hero')].slice(0,4),[0xc1,13,0xc1,72]);
 assert.deepEqual([...friendAcceptPacket(true,'Hero')].slice(0,4),[0xc1,14,0xc2,1]);
 assert.deepEqual([...friendDeletePacket('Hero')].slice(0,4),[0xc1,13,0xc3,72]);
});
test('friend list/result/state decoders',()=>{
 const l=list([{name:'One',server:1},{name:'Two',server:253}]);
 assert.deepEqual(decodeFriendList(l),{mailCount:0,mailTotal:50,count:2,list:[{name:'One',server:1},{name:'Two',server:253}]});
 const req=new Uint8Array(15);req.set([0xc1,15,0xc1,1]);req.set(ascii('Hero',10),4);req[14]=2;
 assert.deepEqual(decodeFriendRequest(req),{result:1,name:'Hero',server:2});
 const accept=new Uint8Array(13);accept.set([0xc1,13,0xc2]);accept.set(ascii('Hero',10),3);
 assert.deepEqual(decodeFriendAcceptRequest(accept),{name:'Hero'});
 const del=new Uint8Array(14);del.set([0xc1,14,0xc3,1]);del.set(ascii('Hero',10),4);
 assert.deepEqual(decodeFriendDelete(del),{result:1,name:'Hero'});
 const state=new Uint8Array(14);state.set([0xc1,14,0xc4]);state.set(ascii('Hero',10),3);state[13]=0;
 assert.deepEqual(decodeFriendState(state),{name:'Hero',server:0});
});
test('session sends add/accept/delete and applies friend list/state',async()=>{
 const {createWorldSession}=await import('../client/src/session/world-session.js'),{hudLayout:L}=await import('../client/src/protocol/hud-layout.js');
 const sent=[],states=[],errors=[],world=createWorldSession({onState:s=>states.push(s),onError:e=>errors.push(e),timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:b=>sent.push(b),close(){}});
 const entry=new Uint8Array(L.PMSG_CHARACTER_INFO_SEND.size);entry.set([0xc1,entry.length,0xf3,3,100,100,0,3]);const choosing=world.select({name:'Buyer'},0);world.receive(entry);await choosing;
 const info=new Uint8Array(L.PMSG_NEW_CHARACTER_INFO_SEND.size);info.set([0xc1,info.length,0xf3,224]);new DataView(info.buffer).setUint16(L.PMSG_NEW_CHARACTER_INFO_SEND.Level,1,true);world.receive(info);
 await world.friendListRequest();assert.deepEqual([...sent.at(-1)],[...friendListPacket()]);
 world.receive(list([{name:'Hero',server:1}]));assert.equal(world.gameState.friend.list[0].name,'Hero');
 await world.friendAdd('Hero');assert.deepEqual([...sent.at(-1)],[...friendAddPacket('Hero')]);
 await world.friendAccept(1,'Hero');assert.deepEqual([...sent.at(-1)],[...friendAcceptPacket(1,'Hero')]);
 await world.friendDelete('Hero');assert.deepEqual([...sent.at(-1)],[...friendDeletePacket('Hero')]);
 world.receive(list([{name:'Hero',server:253}]));assert.equal(world.gameState.friend.list[0].server,253);
 assert.deepEqual(errors,[]);world.close();
});
