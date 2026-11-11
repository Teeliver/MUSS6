import test from 'node:test';import assert from 'node:assert/strict';
import {mapParticleTextures,mapParticleEmitters,spawnMapParticle,stepMapParticle,stepMapEmitter,mapParticleUV,mapParticleCapacity,mapParticleLimit} from '../client/src/world/map-particle-policy.js';
import {hiddenObject,blendMesh} from '../client/src/world/resource-policy.js';import {objectMaterialPulse,objectMaterialTint} from '../client/src/world/object-motion-policy.js';
const node=type=>({type,x:100,y:200,z:300,rx:0,ry:0,rz:0,scale:5});
test('Atlans additive slots and luminosity match its own native MoveObject switch',()=>{
 for(const [type,slot] of [[23,0],[32,1],[34,1],[38,0],[40,0]]){assert.equal(blendMesh(7,type),slot);assert.equal(blendMesh(67,type),slot);assert.deepEqual(objectMaterialPulse(7,{type},slot),objectMaterialPulse(67,{type},slot));}
 assert.deepEqual(objectMaterialTint(objectMaterialPulse(7,node(23),0),0),[.5,.5,.5]);assert.equal(blendMesh(6,23),-1);assert.equal(objectMaterialPulse(7,node(23),1),undefined);
});
test('bubble markers are scoped by map and use the float32 native timer burst, without mutating placements',()=>{
 for(const [map,type] of [[7,22],[67,22],[68,97]]){const original=node(type),[e]=mapParticleEmitters(map,[original]);assert.deepEqual(e.kinds,['bubble']);assert.equal(e.probability,1);assert.equal(hiddenObject(map,type),true);const active=[];for(let tick=1;tick<=100;tick++)if(stepMapEmitter(e))active.push(tick);assert.equal(active[0],51);assert.equal(active.at(-1),99);assert.equal(active.length,49);assert.equal(e.timer,0);assert.equal(original.timer,undefined);}
 assert.equal(mapParticleEmitters(66,[node(22)]).length,0);assert.equal(!!hiddenObject(8,22),false);assert.equal(stepMapEmitter({kinds:['fire']}),true);
});
test('bubble subtype0 retains original atlas, scale, lifetime, random drift and frame wrap',()=>{
 assert.equal(mapParticleTextures.bubble,'Object8/drop01.OZJ');const [e]=mapParticleEmitters(7,[node(22)]),p=spawnMapParticle(e,'bubble',()=>0);assert.equal(p.life,30);assert.equal(p.scale,.12);assert.equal(p.frame,0);assert.deepEqual(mapParticleUV(p),[.005,.005,.24,.24]);assert.equal(stepMapParticle(p,()=>0),true);assert.ok(Math.abs(p.x-.97)<1e-8);assert.ok(Math.abs(p.y-1.97)<1e-8);assert.ok(Math.abs(p.z-3.03)<1e-8);assert.equal(p.frame,1);p.frame=8;assert.deepEqual(mapParticleUV(p),[.505,.505,.24,.24]);p.frame=9;assert.deepEqual(mapParticleUV(p),[.005,.005,.24,.24]);p.life=1;assert.equal(stepMapParticle(p),false);
});
test('DG4 subtype5 uses larger scale and expires only above source world height550',()=>{
 const [e]=mapParticleEmitters(68,[node(97)]),p=spawnMapParticle(e,'bubble',()=>0);assert.equal(p.scale,.16);p.z=5.47;assert.equal(stepMapParticle(p,()=>0),false);assert.equal(p.bubbleSubtype,5);
 const [atlans]=mapParticleEmitters(7,[node(22)]),a=spawnMapParticle(atlans,'bubble',()=>0);a.z=6;assert.equal(stepMapParticle(a,()=>0),true);
});

test('bubble capacity respects the native3000-particle map pool instead of truncating at256',()=>{assert.equal(mapParticleLimit,3000);assert.equal(mapParticleCapacity('bubble'),3000);assert.equal(mapParticleCapacity('fire'),256);});
