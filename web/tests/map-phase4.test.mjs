import test from 'node:test';import assert from 'node:assert/strict';import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';import {execFileSync} from 'node:child_process';
import createCore from '../.build/wasm/frame-reader.mjs';import {decryptTerrain} from '../client/src/world/terrain.js';import {objectUVScroll} from '../client/src/world/object-motion-policy.js';
const data='/Users/angelale/Downloads/MuClientEX603/Data/',core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
test('Elbeland water retains all 24 live triangles and poses after pruning only the 24 zero-area sentinels',async()=>{
 const b=new Uint8Array(await readFile(data+'Object52/Object90.bmd')),g=core.bmdGeometryResult(b);assert.equal(g.ok,true,g.error);assert.equal(g.meshes.length,1);assert.equal(g.meshes[0].positions.length,24*9);assert.equal(g.meshes[0].texture,'urbanwater_R.jpg');const rig=new core.ModelAnimation(b);try{for(const t of [0,.2,1]){const sample=rig.sampleResult(t);assert.equal(sample.ok,true,sample.error);assert.equal(sample.meshes[0].positions.length,24*9);assert.ok(sample.meshes[0].positions.every(Number.isFinite));}}finally{rig.delete();}
 const plain=b[3]===12?decryptTerrain(b.slice(8)):b.slice(4),d=new DataView(plain.buffer),nv=d.getInt16(38,true),nn=d.getInt16(40,true),nt=d.getInt16(42,true),facesAt=48+nv*16+nn*20+nt*8;
 // Reference the sentinel from a live face: rejecting it must take priority over
 // the presence of other safely discardable triangles in the same mesh.
 d.setInt16(facesAt+24*64+2,0,true);const bad=Uint8Array.from([66,77,68,10,...plain]);assert.equal(core.bmdGeometryResult(bad).ok,false);
});
test('shared C++ sentinel filter never normalizes invalid bones still referenced by a live face',async()=>{
 const dir=await mkdtemp('/tmp/mu-phase4-sentinel-');try{const header=new URL('../../client-core/include/mu/bmd_model.hpp',import.meta.url).pathname;await writeFile(dir+'/test.cpp',`#include "${header}"
#include <cassert>
int main(){using namespace mu::core::bmd;Mesh m;m.vertices={ {-8888,{0,0,0}},{0,{1,0,0}},{0,{0,1,0}},{0,{0,0,1}} };m.normals={{-8888,{0,0,0}},{0,{0,0,1}}};m.faces={{{0,0,0},{0,0,0},{0,0,0}},{{1,2,3},{0,0,0},{1,1,1}}};auto live=m;live.faces[1].vertex[0]=0;assert(mu::bmd_validation::discard_empty_sentinel_faces(live)==1);assert(live.vertices[0].bone==-8888);assert(mu::bmd_validation::discard_empty_sentinel_faces(m)==1);assert(m.faces.size()==1);assert(m.vertices[0].bone==-1&&m.normals[0].bone==-1);assert(m.faces[0].vertex[0]==1);auto unused=m;unused.vertices[0].bone=-8888;assert(mu::bmd_validation::discard_empty_sentinel_faces(unused)==0);assert(unused.vertices[0].bone==-8888);}`);execFileSync('clang++',['-std=c++17','-fsanitize=address,undefined',dir+'/test.cpp','-o',dir+'/test']);execFileSync(dir+'/test');}finally{await rm(dir,{recursive:true,force:true});}
});
test('Elbeland water UV advances at source 25Hz increments and leaves other worlds unchanged',()=>{assert.deepEqual(objectUVScroll(51,89,0),[0,.125]);for(const t of [2,53,55])assert.deepEqual(objectUVScroll(51,t,0),[0,.375]);assert.deepEqual(objectUVScroll(0,89,0),[0,0]);});
