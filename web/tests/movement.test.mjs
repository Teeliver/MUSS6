import test from 'node:test';import assert from 'node:assert/strict';
import {pathPacket,movementReply,roadX,roadY} from '../client/src/protocol/movement-codec.js';
import {createWorldSession} from '../client/src/session/world-session.js';
import {createStepMotion,installStepKeys} from '../client/src/world/step-motion.js';
test('ENG movement packs all directions and14 steps in native13-byte layout',()=>{
 assert.deepEqual([...pathPacket(100,100,[0,1,2,3,4,5,6,7]).bytes],[193,13,212,100,100,8,1,35,69,103,0,0,0]);
 for(let d=0;d<8;d++){const p=pathPacket(100,100,[d]);assert.equal(p.x,100+roadX[d]);assert.equal(p.y,100+roadY[d]);assert.equal(p.bytes[5],d*16+1);assert.equal(p.bytes[6],d*16);}
 assert.equal(pathPacket(100,100,Array(14).fill(3)).bytes[12],0x33);
 for(const args of [[0,0,[0]],[255,255,[4]],[1.5,1,[3]],[1,1,[]],[1,1,Array(15).fill(3)],[1,1,[8]],[1,1,[NaN]]])assert.throws(()=>pathPacket(...args));
 assert.deepEqual(movementReply(Uint8Array.of(193,8,212,1,2,100,101,48)),{index:258,x:100,y:101,direction:3,correction:false});
 assert.equal(movementReply(Uint8Array.of(193,7,21,1,2,10,11)).correction,true);
 for(const b of [[193,8,212,1,2,1,1,49],[193,8,212,1,2,1,1,128],[193,8,21,1,2,1,1,0]])assert.throws(()=>movementReply(Uint8Array.from(b)));
});
function fixture(){let time=100,closed=0;const jobs=new Map(),sent=[],positions=[];let next=0;
 const world=createWorldSession({now:()=>time,onPosition:p=>positions.push(p),timers:{setTimeout(fn){jobs.set(++next,fn);return next;},clearTimeout:id=>jobs.delete(id),clearInterval(){}}});
 world.attach({greeting:{index:258},send:p=>sent.push(p),close:()=>closed++});
 const packet=new Uint8Array(136);packet.set([193,136,243,3,100,100,0,3]);
 return {world,jobs,sent,positions,get closed(){return closed;},tick(){time+=400;},async enter(){const p=world.select({name:'Test'},0);world.receive(packet);await p;},reply(x=101,y=100,index=258,correction=false){world.receive(Uint8Array.from(correction?[193,7,21,index>>8,index&255,x,y]:[193,8,212,index>>8,index&255,x,y,48]));}};
}
test('movement waits for own ACK, ignores foreign index, preserves authority and prevents overlap',async()=>{const f=fixture();await assert.rejects(f.world.move([3]));await f.enter();const p=f.world.move([3]);await assert.rejects(f.world.move([3]));f.reply(250,250,259);assert.equal(f.world.position.x,100);f.reply();assert.equal((await p).x,101);assert.equal(f.jobs.size,0);await assert.rejects(f.world.move([3]));f.tick();const q=f.world.move([3]);f.reply(102);await q;f.world.close();assert.equal(f.closed,1);});
test('correction cancels pending path; malformed/timeout/mismatch/send failure close and reject',async()=>{
 const f=fixture();await f.enter();const p=f.world.move([3]);f.reply(99,99,258,true);await assert.rejects(p);assert.equal(f.world.position.x,99);assert.equal(f.positions[0].correction,true);assert.equal(f.closed,0);f.world.close();
 for(const mode of ['malformed','mismatch','close']){const f=fixture();await f.enter();const p=f.world.move([3]);if(mode==='timeout')f.jobs.values().next().value();if(mode==='malformed')f.world.receive(Uint8Array.of(193,4,212,0));if(mode==='mismatch')f.reply(120);if(mode==='close')f.world.close();await assert.rejects(p);assert.equal(f.closed,1);assert.equal(f.jobs.size,0);}
});
test('walking is distance based, bounded, wall/corner aware and correction drops interpolation',()=>{const terrain={flags:new Uint16Array(65536)},m=createStepMotion({x:100,y:100,direction:3},terrain);terrain.flags[100*256+101]=4;assert.equal(m.canStep(3),false);assert.equal(m.canStep(2),false);terrain.flags.fill(0);assert.equal(m.canStep(3),true);m.confirm({x:101,y:100,direction:3});assert.equal(m.position.x,100);m.advance(10);assert.ok(m.position.x<100.2);assert.equal(m.canStep(3),false);for(let i=0;i<30;i++)m.advance(1/60);assert.equal(m.position.x,101);m.correct({x:112,y:100});assert.equal(m.canStep(3),false);assert.equal(m.canStep(7),true);assert.throws(()=>m.advance(NaN));const edge=createStepMotion({x:0,y:0,direction:0},terrain);assert.equal(edge.canStep(0),false);});
test('keys ignore typing/hidden/modifiers, serialize requests, dispose and discard stale ACK after correction',async()=>{let listener,calls=0,resolve;const target={addEventListener(_,fn){listener=fn;},removeEventListener(){listener=undefined;}},motion=createStepMotion({x:100,y:100,direction:3},{flags:new Uint16Array(65536)});let hidden=false;
 const stop=installStepKeys(target,{motion,hidden:()=>hidden,move:()=>{calls++;return new Promise(r=>resolve=r);}}),event={key:'ArrowRight',preventDefault(){}};
 hidden=true;await listener(event);hidden=false;await listener({...event,ctrlKey:true});await listener({...event,target:{closest:()=>true}});assert.equal(calls,0);
 const p=listener(event);await listener(event);assert.equal(calls,1);motion.correct({x:99,y:99});resolve({x:101,y:100,direction:3});await p;assert.equal(motion.position.x,99);stop();assert.equal(listener,undefined);
});
test('send failure closes movement once and packets after close cannot change position',async()=>{const f=fixture();await f.enter();f.world.attach({greeting:{index:258},send(){throw new Error('offline');},close(){}});await assert.rejects(f.world.move([3]));assert.equal(f.jobs.size,0);f.reply();assert.equal(f.world.position.x,100);});
test('60/120Hz walking covers equal distance and camera follows without changing projection orientation',async()=>{const {worldCamera}=await import('../client/src/world/camera.js');const make=()=>createStepMotion({x:100,y:100,direction:3},{flags:new Uint16Array(65536)});const a=make(),b=make();a.confirm({x:101,y:100,direction:3});b.confirm({x:101,y:100,direction:3});for(let i=0;i<12;i++)a.advance(1/60);for(let i=0;i<24;i++)b.advance(1/120);assert.ok(Math.abs(a.position.x-b.position.x)<1e-10);const base=worldCamera(1.5),follow=worldCamera(1.5,0,1,2);assert.deepEqual([...base.slice(0,12)],[...follow.slice(0,12)]);assert.notDeepEqual([...base.slice(12)],[...follow.slice(12)]);});

test('movement timeout keeps the authenticated connection and accepts a late authoritative ACK',async()=>{const f=fixture();await f.enter();const p=f.world.move([3]);f.jobs.values().next().value();await assert.rejects(p);assert.equal(f.closed,0);f.reply(101);assert.equal(f.closed,0);assert.equal(f.world.position.x,101);f.world.close();});
