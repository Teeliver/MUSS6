import test from 'node:test';import assert from 'node:assert/strict';
import {guildListRequestPacket,guildViewportRequestPacket,guildUnionListRequestPacket,decodeGuildViewport,decodeGuildInfo,decodeGuildList,decodeGuildUnionList,decodeGuildCreateResult,decodeGuildDeleteResult,guildLeavePacket,guildStatusTextId,guildStatusBackground,GUILD_STATUS} from '../client/src/protocol/guild.js';
import {createGameState} from '../client/src/protocol/game-state.js';
const name=(value,length)=>Uint8Array.from([...value].map(c=>c.charCodeAt(0)).concat(Array(length-value.length).fill(0)));
test('guild requests match C1 52 / C1 66 with padded DWORD key',()=>{
 assert.deepEqual([...guildListRequestPacket()],[0xc1,3,0x52]);
 assert.deepEqual([...guildViewportRequestPacket(0x11223344)],[0xc1,8,0x66,0,0x44,0x33,0x22,0x11]);assert.deepEqual([...guildUnionListRequestPacket()],[0xc1,3,0xe9]);
});
test('guild viewport and guild info decode UPDATE603 layouts',()=>{
 const v=new Uint8Array(17);v.set([0xc2,0,17,0x65,1]);new DataView(v.buffer).setUint32(5,0x11223344,true);v.set([128,1,0,0,5,0],9);
 assert.deepEqual(decodeGuildViewport(v),{entries:[{key:0x11223344,status:128,type:1,relationship:0,index:5,owner:0}]});
 const i=new Uint8Array(60);i.set([0xc1,60,0x66,0]);new DataView(i.buffer).setUint32(4,0x11223344,true);i[8]=2;i.set(name('UNION',8),9);i.set(name('GUILD',8),17);i.set(Array.from({length:32},(_,n)=>n),25);
 assert.deepEqual(decodeGuildInfo(i),{key:0x11223344,type:2,unionName:'UNION',name:'GUILD',mark:Array.from({length:32},(_,n)=>n)});
});
test('guild member list decodes total score, rival and member status',()=>{
 const b=new Uint8Array(24+2*13);b.set([0xc2,0,b.length,0x52,1,2]);new DataView(b.buffer).setUint32(8,12345,true);b[12]=5;b.set(name('RIVAL',9),13);
 b.set(name('Master',10),24);b[34]=3;b[35]=0x83;b[36]=128;
 b.set(name('Member',10),37);b[47]=0xff;b[48]=0xff;b[49]=0;
 assert.deepEqual(decodeGuildList(b),{result:1,count:2,totalScore:12345,score:5,rival:'RIVAL',members:[{name:'Master',number:3,connected:0x83,status:128,server:3,online:true},{name:'Member',number:0xff,connected:0xff,status:0,server:-1,online:false}]});
 assert.equal(guildStatusTextId(GUILD_STATUS.MASTER),1300);assert.equal(guildStatusTextId(GUILD_STATUS.OFFICE_MASTER),1301);assert.equal(guildStatusTextId(GUILD_STATUS.BATTLE_MASTER),1302);assert.equal(guildStatusTextId(0),0);
 assert.equal(guildStatusBackground(GUILD_STATUS.MASTER),'rgba(255,100,50,.5)');assert.equal(guildStatusBackground(0),undefined);
});



test('guild leave packet C1:53 and delete result decode',()=>{const p=guildLeavePacket('PWTest');assert.equal(p.length,23);assert.deepEqual([...p.slice(0,3)],[0xc1,23,0x53]);assert.equal(String.fromCharCode(...p.slice(3,9)),'PWTest');assert.deepEqual(decodeGuildDeleteResult(Uint8Array.of(0xc1,4,0x53,1)),{result:1});assert.deepEqual(decodeGuildDeleteResult(Uint8Array.of(0xc1,4,0x53,4)),{result:4});assert.equal(decodeGuildDeleteResult(Uint8Array.of(0xc1,4,0x52,1)),undefined);});
test('guild create result C1:56 decodes result and type',()=>{assert.deepEqual(decodeGuildCreateResult(Uint8Array.of(0xc1,5,0x56,1,0)),{result:1,type:0});assert.deepEqual(decodeGuildCreateResult(Uint8Array.of(0xc1,5,0x56,3,2)),{result:3,type:2});assert.equal(decodeGuildCreateResult(Uint8Array.of(0xc1,5,0x55,1,0)),undefined);});
test('guild union list decodes C2:E9 count, marks and member counts',()=>{
 const b=new Uint8Array(8+2*41);b.set([0xc2,0,b.length,0xe9,2,1,7,8]);
 b[8]=5;b.set(Array.from({length:32},(_,n)=>n),9);b.set(name('ALLYA',8),41);
 b[49]=9;b.set(Array.from({length:32},(_,n)=>31-n),50);b.set(name('ALLYB',8),82);
 const u=decodeGuildUnionList(b);assert.equal(u.count,2);assert.equal(u.result,1);assert.equal(u.unionNumber,7);assert.equal(u.rivalNumber,8);assert.equal(u.members[0].name,'ALLYA');assert.equal(u.members[0].count,5);assert.equal(u.members[0].mark[0],0);assert.equal(u.members[1].name,'ALLYB');assert.equal(u.members[1].mark[0],31);
});
test('game state stores guild identity, info and member list',()=>{
 const gs=createGameState();gs.guildKey(7);gs.guildInfo({key:7,name:'G',unionName:'U',mark:new Uint8Array(32)});gs.guildList({result:1,totalScore:10,score:2,rival:'R',members:[{name:'A'}]});gs.guildUnionList({result:1,unionNumber:7,rivalNumber:8,members:[{name:'U',count:2,mark:new Uint8Array(32)}]});gs.guildNotice('line');
 const g=gs.snapshot().guild;assert.equal(g.key,7);assert.equal(g.name,'G');assert.equal(g.unionName,'U');assert.equal(g.mark.length,32);assert.equal(g.members[0].name,'A');assert.equal(g.totalScore,10);assert.equal(g.rival,'R');assert.equal(g.unionMembers[0].name,'U');assert.equal(g.unionNumber,7);assert.equal(g.notice[0],'line');gs.guildLeaveResult(4);assert.equal(gs.snapshot().guild.name,'');
});
test('guild member server byte uses the online flag and hides offline 0x7f/0xff sentinels',()=>{
 const b=new Uint8Array(24+2*13);b.set([0xc2,0,b.length,0x52,1,2]);b.set(name('Online',10),24);b[34]=1;b[35]=0x80;b[36]=128;b.set(name('Offline',10),37);b[47]=2;b[48]=0x7f;b[49]=0;
 const list=decodeGuildList(b);assert.deepEqual(list.members.map(m=>[m.server,m.online]),[[0,true],[-1,false]]);
 const b2=b.slice();b2[48]=0xff;assert.deepEqual(decodeGuildList(b2).members.map(m=>m.online),[true,false]);
 const b3=b.slice();b3[48]=0x93;assert.equal(decodeGuildList(b3).members[1].server,19);
});
