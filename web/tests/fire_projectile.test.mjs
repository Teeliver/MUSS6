import test from 'node:test';
import assert from 'node:assert/strict';
import {fireProjectile} from '../client/src/preview/fire-projectile.js';
const geometry=()=>({meshes:[0,1].map(()=>({positions:Float32Array.of(1,0,0,0,1,0,0,0,1),normals:Float32Array.of(1,0,0,0,1,0,0,0,1),uv:new Float32Array(6)}))});
test('Fire01 follows native scale/axis/yaw, height and speed for fixed synthetic target',()=>{
 const original=geometry(),start=fireProjectile(original,0),middle=fireProjectile(original,.12);
 assert.ok(Math.abs(start.meshes[0].positions[1]-.01/.009)<.0001);assert.ok(Math.abs(start.meshes[0].positions[2]-1.2/.009)<.0001);
 assert.ok(Math.abs(middle.meshes[0].positions[0]-1.5/.009)<.0001);assert.equal(start.meshes[1].additive,true);assert.equal(start.meshes[0].additive,false);
 assert.deepEqual([...original.meshes[0].positions],[1,0,0,0,1,0,0,0,1]);assert.ok(fireProjectile(original,.25).meshes.every(m=>m.positions.every(v=>v===0)));
 assert.throws(()=>fireProjectile(original,-1));assert.throws(()=>fireProjectile(original,NaN));
});
test('Fire01 pooled output preserves numeric result and buffer identity without mutating source',()=>{
 const source=geometry(),pooled=fireProjectile(source,0),positions=pooled.meshes[0].positions,normals=pooled.meshes[0].normals;
 for(const time of [.12,.25,2.1]){const expected=fireProjectile(source,time),actual=fireProjectile(source,time,pooled);assert.equal(actual,pooled);assert.equal(actual.meshes[0].positions,positions);assert.equal(actual.meshes[0].normals,normals);assert.deepEqual(actual,expected);}
 assert.equal(source.meshes[0].positions[0],1);assert.throws(()=>fireProjectile({meshes:[]},0,pooled));
});
