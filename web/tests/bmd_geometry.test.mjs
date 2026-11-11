import test from 'node:test';
import assert from 'node:assert/strict';
import {decodeBmdGeometry} from '../client/src/platform/assets/decode-model.js';
import {core} from './helpers/game-fixture.mjs';
import {fixture} from './helpers/bmd-fixture.mjs';
test('BMD geometry WASM returns owned Float32 positions/normals/UV in initial pose',()=>{
 const bytes=fixture(),result=decodeBmdGeometry(core,bytes),mesh=result.meshes[0];
 assert.ok(mesh.positions instanceof Float32Array);assert.ok(mesh.normals instanceof Float32Array);assert.ok(mesh.uv instanceof Float32Array);
 assert.deepEqual([...mesh.positions],[2,0,0,3,0,0,4,0,0]);assert.deepEqual([...mesh.normals],[1,0,0,1,0,0,1,0,0]);assert.deepEqual([...mesh.uv],[0,0,1,0,2,0]);assert.equal(mesh.texture,'fixture.tga');
 bytes.fill(0);const another=decodeBmdGeometry(core,fixture());another.meshes[0].positions.fill(99);assert.equal(mesh.positions[0],2);
 assert.throws(()=>decodeBmdGeometry(core,new Uint8Array(10)));assert.throws(()=>decodeBmdGeometry(core,'bad'));
});
