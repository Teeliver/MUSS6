import test from 'node:test';import assert from 'node:assert/strict';
import {objectMaterialPulse,objectMaterialTint,objectOpacity} from '../client/src/world/object-motion-policy.js';
import {blendMesh} from '../client/src/world/resource-policy.js';
import {loadEnvironment} from '../client/src/world/environment.js';
import {core} from './helpers/game-fixture.mjs';import {fixture} from './helpers/bmd-fixture.mjs';
test('DoppelGanger3 binds source luminosity to the specified texture slot',()=>{
 for(const [type,slot,frequency,amplitude,base] of [[23,0,2,.3,.5],[32,1,4,.5,.5],[34,1,4,.5,.5],[40,0,4,.3,.5]]){
 assert.equal(blendMesh(67,type),slot);const pulse=objectMaterialPulse(67,{type},slot);assert.deepEqual(pulse,[frequency,amplitude,base]);assert.deepEqual(objectMaterialTint(pulse,0),[base,base,base]);const peak=objectMaterialTint(pulse,Math.PI/2/frequency);assert.ok(Math.abs(peak[0]-base-amplitude)<1e-10);assert.equal(objectMaterialPulse(67,{type},1-slot),undefined);
 }
 assert.equal(blendMesh(67,38),0);assert.equal(objectMaterialPulse(67,{type:38},0),undefined);assert.equal(blendMesh(66,23),-1);
});
test('DoppelGanger4 preserves the render override of type96 instead of applying MoveObject alpha',()=>{
 assert.equal(objectOpacity(68,76),.5);assert.equal(objectOpacity(68,96),1);assert.equal(objectOpacity(65,76),1);
});
function placements(map,types){const bytes=new Uint8Array(4+30*types.length);bytes[1]=map+1;const d=new DataView(bytes.buffer);types.forEach((type,i)=>{let p=4+i*30;d.setInt16(p,type,true);[10000,10000,0,0,0,0,1].forEach((v,j)=>d.setFloat32(p+2+j*4,v,true));});const key=[209,115,82,246,210,154,203,39,62,175,89,49,55,179,231,162];let prev=94;return Uint8Array.from(bytes,(v,i)=>{const n=((v+prev)&255)^key[i%16];prev=(n+61)&255;return n;});}
test('environment keeps shared textures apart when pulse or opacity differs',async()=>{
 for(const [map,types] of [[67,[23,38]],[68,[76,77]]]){
 const env=await loadEnvironment({core,entry:{map,x:100,y:100},terrain:{height:()=>0},resources:{worlds:[{map,objectTypes:Object.fromEntries(types.map(t=>[t,{status:'ready',path:'Object/model.bmd',meshes:[{texture:'Object/shared.OZT'}]}]))}]},load:async p=>p.endsWith('.obj')?placements(map,types):fixture(),texture:async()=>({width:1,height:1})});
 try{assert.equal(env.meshes.length,2);if(map===67){assert.ok(env.meshes.every(m=>m.additive));assert.equal(env.meshes.filter(m=>m.materialAt).length,1);const m=env.meshes.find(m=>m.materialAt);assert.notDeepEqual(m.materialAt(0),m.materialAt(500));}else assert.deepEqual(env.meshes.map(m=>m.opacity),[.5,1]);}finally{env.dispose();}
 }
});
