import test from 'node:test';import assert from 'node:assert/strict';import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';import {execFileSync} from 'node:child_process';
import createCore from '../.build/wasm/frame-reader.mjs';import {decryptTerrain} from '../client/src/world/terrain.js';
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))}),data='/Users/angelale/Downloads/MuClientEX603/Data/';
test('original Crywolf/Swamp trees retain live branches and safely discard only the zero-area sentinel mesh',async()=>{
 for(const path of ['Object35/Object36.bmd','Object57/Object36.bmd']){
  const bytes=new Uint8Array(await readFile(data+path)),geometry=core.bmdGeometryResult(bytes);assert.equal(geometry.ok,true,geometry.error);assert.equal(geometry.meshes.length,2);assert.equal(geometry.meshes[0].positions.length,112*9);assert.equal(geometry.meshes[1].positions.length,0);assert.equal(geometry.meshes[0].texture,'cr1_branch06.tga');const rig=new core.ModelAnimation(bytes);try{for(const time of [0,.2,1.1]){const sample=rig.sampleResult(time);assert.equal(sample.ok,true,sample.error);assert.equal(sample.meshes[1].positions.length,0);assert.ok(sample.meshes[0].positions.every(Number.isFinite));}}finally{rig.delete();}
  const plain=bytes[3]===12?decryptTerrain(bytes.slice(8)):bytes.slice(4),view=new DataView(plain.buffer);view.setInt16(48,-8888,true);const corrupt=Uint8Array.from([66,77,68,10,...plain]);assert.equal(core.bmdGeometryResult(corrupt).ok,false,'nondegenerate mesh must still reject invalid bones');
 }
});
test('empty-mesh exception rejects near misses instead of accepting arbitrary damaged skin',async()=>{
 const dir=await mkdtemp('/tmp/mu-empty-mesh-test-');try{const header=new URL('../../client-core/include/mu/bmd_model.hpp',import.meta.url).pathname;await writeFile(dir+'/test.cpp',`#include "${header}"
#include <cassert>
int main(){using namespace mu::core::bmd;Mesh m;m.vertices.push_back({-8888,{0,0,0}});m.normals.push_back({-8888,{0,0,0}});m.faces.push_back({{0,0,0},{0,0,0},{0,0,0}});auto bad=m;bad.vertices[0].position.x=1;assert(!mu::bmd_validation::discard_empty_sentinel_mesh(bad));bad=m;bad.normals[0].bone=999;assert(!mu::bmd_validation::discard_empty_sentinel_mesh(bad));bad=m;bad.faces[0].normal[1]=1;assert(!mu::bmd_validation::discard_empty_sentinel_mesh(bad));assert(mu::bmd_validation::discard_empty_sentinel_mesh(m));assert(m.faces.empty()&&m.vertices.empty()&&m.normals.empty());}`);execFileSync('clang++',['-std=c++17','-fsanitize=address,undefined',dir+'/test.cpp','-o',dir+'/test']);execFileSync(dir+'/test');}finally{await rm(dir,{recursive:true,force:true});}
});
test('original padded clouds JPEG keeps the outer 256x256 image and accepts native trailers but rejects missing EOI',async()=>{
 const b=new Uint8Array(await readFile(data+'Effect/clouds.OZJ')),result=core.textureResult('OZJ',b);assert.equal(result.ok,true,result.error);assert.equal(result.width,256);assert.equal(result.height,256);assert.equal(result.bytes.at(-2),255);assert.equal(result.bytes.at(-1),217);assert.ok(result.bytes.length<b.length-24);
 const junk=b.slice();junk[junk.length-1]=7;assert.equal(core.textureResult('OZJ',junk).ok,true);
 const truncated=b.slice(0,b.length-50);assert.equal(core.textureResult('OZJ',truncated).ok,false);
});
