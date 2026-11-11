import test from 'node:test';import assert from 'node:assert/strict';
import {decodeDuelBroadcast} from '../client/src/protocol/duel.js';
import {createGameState} from '../client/src/protocol/game-state.js';
const name=(value,length)=>Uint8Array.from([...value].map(c=>c.charCodeAt(0)).concat(Array(length-value.length).fill(0)));
test('duel score/HP/round/win broadcasts decode UPDATE603 layouts',()=>{
 assert.deepEqual(decodeDuelBroadcast(Uint8Array.of(0xc1,10,0xaa,4,0,1,0,2,3,4)),{kind:'score',index1:1,index2:2,score1:3,score2:4});
 assert.deepEqual(decodeDuelBroadcast(Uint8Array.of(0xc1,12,0xaa,5,0,1,0,2,80,70,60,50)),{kind:'hp',index1:1,index2:2,life1:80,life2:70,shield1:60,shield2:50});
 assert.deepEqual(decodeDuelBroadcast(Uint8Array.of(0xc1,5,0xaa,0x0d,0)),{kind:'round',flag:0});
 const win=new Uint8Array(24);win.set([0xc1,24,0xaa,0x0c]);win.set(name('Winner',10),4);win.set(name('Loser',10),14);
 assert.deepEqual(decodeDuelBroadcast(win),{kind:'win',winner:'Winner',loser:'Loser'});
 assert.equal(decodeDuelBroadcast(Uint8Array.of(0xc1,17,0xaa,1,0,0,1,...name('Enemy',10))),undefined);
});
test('game state tracks duel orientation, score, HP, round reset and winner',()=>{
 const gs=createGameState();gs.duelStart({index:2,name:'Enemy'});
 assert.equal(gs.snapshot().duel.active,true);assert.equal(gs.snapshot().duel.enemy,'Enemy');
 gs.duelScore({index1:1,index2:2,score1:3,score2:1},1);assert.equal(gs.snapshot().duel.myScore,3);assert.equal(gs.snapshot().duel.enemyScore,1);
 gs.duelHp({index1:2,index2:1,life1:40,life2:90,shield1:30,shield2:80},1);assert.equal(gs.snapshot().duel.myHp,.9);assert.equal(gs.snapshot().duel.enemyHp,.4);assert.equal(gs.snapshot().duel.mySd,.8);
 gs.duelHp({index1:1,index2:2,life1:10,life2:20,shield1:5,shield2:6},1);gs.duelRound(0);assert.equal(gs.snapshot().duel.myHp,1);assert.equal(gs.snapshot().duel.enemySd,1);
 gs.duelWin('Me','Enemy');assert.equal(gs.snapshot().duel.winner,'Me');gs.duelEnd();assert.equal(gs.snapshot().duel.active,false);
});
