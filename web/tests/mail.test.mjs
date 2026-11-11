import test from 'node:test';import assert from 'node:assert/strict';
import {mailListPacket,mailReadPacket,mailDeletePacket,mailSendPacket,decodeMailResult,decodeMailAlert,decodeMailText,decodeMailDelete} from '../client/src/protocol/mail.js';
const be16=(n)=>[n>>8&255,n&255];
function alertPacket({index=1,name='Hero',date='2026-11-02 10:00:00',title='Hello',read=0}){const b=new Uint8Array(107);b.set([0xc1,107,0xc6]);new DataView(b.buffer).setUint16(4,index,true);b.set(new TextEncoder().encode(name),6);b.set(new TextEncoder().encode(date),16);b.set(new TextEncoder().encode(title),46);b[106]=read;return b;}
function textPacket({index=1,text='Body',photoDir=0,photoAction=0}){const body=new TextEncoder().encode(text),b=new Uint8Array(28+body.length);b.set([0xc1,b.length,0xc7]);const d=new DataView(b.buffer);d.setUint16(4,index,true);d.setUint16(6,body.length,true);b[26]=photoDir;b[27]=photoAction;b.set(body,28);return b;}
test('mail request and send packets match UPDATE603 layout',()=>{
 assert.deepEqual([...mailListPacket()],[0xc1,3,0xc9]);
 assert.deepEqual([...mailReadPacket(7)],[0xc1,6,0xc7,0,7,0]);
 assert.deepEqual([...mailDeletePacket(7)],[0xc1,6,0xc8,0,7,0]);
 const b=mailSendPacket({window:0x11223344,name:'Hero',title:'Hi',body:'Body',photoDir:0x51,photoAction:2});
 assert.equal(b.length,82+4);assert.deepEqual([...b.slice(0,4)],[0xc4,be16(86)[0],be16(86)[1],0xc5]);
 assert.equal(new DataView(b.buffer).getUint32(4,true),0x11223344);
 assert.equal(new TextDecoder().decode(b.slice(8,18)).split('\0')[0],'Hero');
 assert.equal(new TextDecoder().decode(b.slice(18,78)).split('\0')[0],'Hi');
 assert.equal(b[78],0x51);assert.equal(b[79],2);assert.equal(new DataView(b.buffer).getUint16(80,true),4);
 assert.equal(new TextDecoder().decode(b.slice(82)),'Body');
});
test('mail decoders accept C1 and C3 encrypted forms',()=>{
 const res=new Uint8Array(8);res.set([0xc3,8,0xc5,1]);new DataView(res.buffer).setUint32(4,9,true);
 assert.deepEqual(decodeMailResult(res),{result:1,window:9});
 assert.deepEqual(decodeMailAlert(alertPacket({index:5,name:'Elf',title:'Re: Hi',read:2})),{index:5,name:'Elf',date:'2026-11-02 10:00:00',title:'Re: Hi',read:2});
 const text=textPacket({index:5,text:'Xin chào',photoDir:1,photoAction:2});text[0]=0xc3;
 assert.deepEqual(decodeMailText(text),{index:5,size:9,photoDir:1,photoAction:2,text:'Xin chào'});
 const wide=textPacket({index:5,text:'Wide',photoDir:0,photoAction:0});wide[0]=0xc2;wide[3]=0xc7;wide[2]=wide.length;assert.equal(decodeMailText(wide)?.text,'Wide');
 const del=new Uint8Array(6);del.set([0xc1,6,0xc8,1]);new DataView(del.buffer).setUint16(4,5,true);
 assert.deepEqual(decodeMailDelete(del),{result:1,index:5});
});
test('session applies mail list, alert, body and delete',async()=>{
 const {createWorldSession}=await import('../client/src/session/world-session.js'),{hudLayout:L}=await import('../client/src/protocol/hud-layout.js');
 const sent=[],errors=[],world=createWorldSession({onError:e=>errors.push(e),timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}}});
 world.attach({greeting:{index:1},send:b=>sent.push(b),close(){}});
 const entry=new Uint8Array(L.PMSG_CHARACTER_INFO_SEND.size);entry.set([0xc1,entry.length,0xf3,3,100,100,0,3]);const choosing=world.select({name:'Buyer'},0);world.receive(entry);await choosing;
 const info=new Uint8Array(L.PMSG_NEW_CHARACTER_INFO_SEND.size);info.set([0xc1,info.length,0xf3,224]);new DataView(info.buffer).setUint16(L.PMSG_NEW_CHARACTER_INFO_SEND.Level,1,true);world.receive(info);
 await world.requestMailList();assert.deepEqual([...sent.at(-1)],[...mailListPacket()]);
 world.receive(alertPacket({index:3,name:'Hero',title:'Hello',read:0}));
 assert.equal(world.gameState.friend.mail.list[0].title,'Hello');
 await world.readMail(3);assert.deepEqual([...sent.at(-1)],[...mailReadPacket(3)]);
 world.receive(textPacket({index:3,text:'Noi dung',photoDir:0x4b,photoAction:5}));
 assert.equal(world.gameState.friend.mail.bodies[3].text,'Noi dung');assert.equal(world.gameState.friend.mail.bodies[3].photoDir,0x4b);assert.equal(world.gameState.friend.mail.bodies[3].photoAction,5);
 await world.deleteMail(3);assert.deepEqual([...sent.at(-1)],[...mailDeletePacket(3)]);
 const del=new Uint8Array(6);del.set([0xc1,6,0xc8,1]);new DataView(del.buffer).setUint16(4,3,true);world.receive(del);
 assert.equal(world.gameState.friend.mail.list.length,0);
 await world.sendMail({window:7,name:'Hero',title:'Hi',body:'Body',photoDir:0x4b,photoAction:5});assert.deepEqual([...sent.at(-1)],[...mailSendPacket({window:7,name:'Hero',title:'Hi',body:'Body',photoDir:0x4b,photoAction:5})]);
 const ack=new Uint8Array(8);ack.set([0xc1,8,0xc5,1]);new DataView(ack.buffer).setUint32(4,7,true);world.receive(ack);
 assert.equal(world.gameState.friend.mail.result.result,1);
 assert.deepEqual(errors,[]);world.close();
});
