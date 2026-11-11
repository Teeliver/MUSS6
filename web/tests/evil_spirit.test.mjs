import test from 'node:test';
import assert from 'node:assert/strict';
import {EvilSpirits,spiritGeometry} from '../client/src/preview/evil-spirit.js';
test('Evil Spirit deterministic 25Hz ticks, six tail points and bounded lifetime',()=>{
 const a=new EvilSpirits(1),b=new EvilSpirits(1);for(let i=0;i<50;i++){a.advance(.04);b.step();assert.deepEqual(a.heads,b.heads);assert.ok(a.heads.every(h=>h.tail.length<=6));}a.advance(1);assert.equal(a.frame,50);assert.equal(a.turn(0,180),10);assert.equal(a.turn(0,540),-10);assert.throws(()=>a.advance(-1));
});
test('Evil Spirit geometry keeps fixed topology, finite vertices, dark material and reusable arrays',()=>{
 const sim=new EvilSpirits(),head={meshes:[{positions:Float32Array.of(0,0,0,1,0,0,0,1,0),normals:new Float32Array(9),uv:new Float32Array(6)}]},output=spiritGeometry(head,sim,0),buffer=output.meshes[0].positions;
 assert.equal(output.meshes.length,12);for(let i=0;i<50;i++){sim.step();assert.equal(spiritGeometry(head,sim,i*.04,output),output);assert.equal(output.meshes[0].positions,buffer);assert.ok(output.meshes.every(m=>m.positions.every(Number.isFinite)&&m.positions.length/3===m.uv.length/2&&m.multiply));}assert.equal(output.meshes[0].spiritLight,0);
});
