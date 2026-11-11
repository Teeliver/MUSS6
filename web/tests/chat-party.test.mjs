import test from 'node:test';import assert from 'node:assert/strict';
import {chatPacket,whisperPacket,partyInvitePacket,partyAnswerPacket,partyListPacket,partyLeavePacket,decodeChatPacket,decodePartyPacket,parseChatInput,chatMessageBytes,CHAT_MESSAGE_MAX_BYTES} from '../client/src/protocol/chat-party.js';
const ascii=(s,n)=>{const o=new Uint8Array(n);o.set(new TextEncoder().encode(s));return o;};
test('chat supports Vietnamese UTF-8 and enforces 59-byte server limit',()=>{
 const viet=chatPacket('Hero','Chào bạn khỏe không');
 assert.equal(viet.length,3+10+chatMessageBytes('Chào bạn khỏe không')+1);
 assert.equal(chatMessageBytes('Chào'),5);
 assert.deepEqual(decodeChatPacket(viet),{kind:'chat',name:'Hero',message:'Chào bạn khỏe không'});
 assert.throws(()=>chatPacket('Hero','ế'.repeat(30)),/59 byte/);
 assert.equal(CHAT_MESSAGE_MAX_BYTES,59);
 assert.throws(()=>chatPacket('Tên','hello'),/10 ký tự Latin/);
});
test('slash name sends whisper, normal text sends chat, reserved slash commands stay chat',()=>{
 assert.deepEqual(parseChatInput('hello world'),{kind:'chat',message:'hello world'});
 assert.deepEqual(parseChatInput('/Seller hi there'),{kind:'whisper',name:'Seller',message:'hi there'});
 assert.deepEqual(parseChatInput('/warp Atlans'),{kind:'chat',message:'/warp Atlans'});
 assert.deepEqual(parseChatInput('/toolongname123 hi'),{kind:'chat',message:'/toolongname123 hi'});
 assert.deepEqual(parseChatInput('   '),{kind:'chat',message:''});
});
test('chat and whisper packets use C1 00/02 with 10-byte name',()=>{
 const chat=chatPacket('Hero','hello');
 assert.deepEqual([...chat.slice(0,3)],[0xc1,19,0x00]);assert.equal(chat[13],'h'.charCodeAt(0));
 const whispered=whisperPacket('Hero','hi');
 assert.deepEqual([...whispered.slice(0,3)],[0xc1,16,0x02]);
 assert.throws(()=>chatPacket('Hero',''),/rỗng/);
 assert.throws(()=>whisperPacket('Hero','x'.repeat(60)),/59/);
});
test('party packets match C1 40/41/42/43 and UPDATE603 list entries',()=>{
 assert.deepEqual([...partyInvitePacket(0x1234)],[0xc3,5,0x40,0x12,0x34]);
 assert.deepEqual([...partyAnswerPacket(1,0x1234)],[0xc3,6,0x41,1,0x12,0x34]);
 assert.deepEqual([...partyListPacket()],[0xc1,3,0x42]);
 assert.deepEqual([...partyLeavePacket(2)],[0xc1,4,0x43,2]);
 const list=new Uint8Array(5+24);list.set([0xc1,29,0x42,1,1]);list.set(ascii('Hero',10),5);list[15]=0;const d=new DataView(list.buffer);d.setUint32(21,100,true);d.setUint32(25,250,true);list[5+10]=2;
 const decoded=decodePartyPacket(list);
 assert.equal(decoded.kind,'list');assert.equal(decoded.items[0].name,'Hero');assert.equal(decoded.items[0].number,2);assert.equal(decoded.items[0].hp,100);assert.equal(decoded.items[0].maxHp,250);
});
test('chat whitelist decodes C1 self, C2 broadcast and server messages',()=>{
 const c1=chatPacket('Hero','hello');assert.deepEqual(decodeChatPacket(c1),{kind:'chat',name:'Hero',message:'hello'});
 const wide=new Uint8Array(3+c1.length);wide.set([0xc2,0,c1.length]);wide.set(c1.slice(2),3);
 assert.deepEqual(decodeChatPacket(wide),{kind:'chat',name:'Hero',message:'hello'});
 assert.deepEqual(decodeChatPacket(Uint8Array.of(0xc1,4,0x0c,0)),{kind:'serverMsg',number:0});
});
test('session sends chat/whisper/party and applies party request, list, life and leave',async()=>{
 const {createWorldSession}=await import('../client/src/session/world-session.js'),{hudLayout:L}=await import('../client/src/protocol/hud-layout.js');
 const sent=[],chat=[],states=[],errors=[],world=createWorldSession({onChat:e=>chat.push(e),onState:s=>states.push(s),onError:e=>errors.push(e),timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:b=>sent.push(b),close(){}});
 const entry=new Uint8Array(L.PMSG_CHARACTER_INFO_SEND.size);entry.set([0xc1,entry.length,0xf3,3,100,100,0,3]);const choosing=world.select({name:'Buyer'},0);world.receive(entry);await choosing;
 const info=new Uint8Array(L.PMSG_NEW_CHARACTER_INFO_SEND.size);info.set([0xc1,info.length,0xf3,224]);new DataView(info.buffer).setUint16(L.PMSG_NEW_CHARACTER_INFO_SEND.Level,1,true);world.receive(info);
 world.chat('hello');assert.deepEqual([...sent.at(-1)],[...chatPacket('Buyer','hello')]);
 world.whisper('Seller','hi');assert.deepEqual([...sent.at(-1)],[...whisperPacket('Seller','hi')]);
 world.partyInvite(0x1234);assert.deepEqual([...sent.at(-1)],[...partyInvitePacket(0x1234)]);
 const request=Uint8Array.of(0xc1,4,0x40,0x12,0x34);world.receive(request);assert.deepEqual(world.gameState.party.request,{index:0x1234,name:'Player 4660'});
 world.partyAnswer(1,0x1234);assert.deepEqual([...sent.at(-1)],[...partyAnswerPacket(1,0x1234)]);
 const list=new Uint8Array(5+24);list.set([0xc1,29,0x42,1,1]);list.set(ascii('Seller',10),5);list[15]=0;const d=new DataView(list.buffer);d.setUint32(21,100,true);d.setUint32(25,250,true);world.receive(list);assert.equal(world.gameState.party.members[0].name,'Seller');
 const life=Uint8Array.of(0xc1,5,0x44,1,0x07);world.receive(life);assert.equal(world.gameState.party.members[0].step,7);
 const wideChat=new Uint8Array(3+chatPacket('Other','~party').length);const body=chatPacket('Other','~party');wideChat.set([0xc2,0,body.length]);wideChat.set(body.slice(2),3);world.receive(wideChat);assert.deepEqual(chat.at(-1),{kind:'party',name:'Other',message:'party'});
 world.receive(Uint8Array.of(0xc1,3,0x43));assert.equal(world.gameState.party.members.length,0);
 assert.deepEqual(errors,[]);world.close();
});
