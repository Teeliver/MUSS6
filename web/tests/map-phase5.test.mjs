import test from 'node:test';
import assert from 'node:assert/strict';
import {mapParticleEmitters,mapEmissionKinds,spawnMapParticle,stepMapParticle} from '../client/src/world/map-particle-policy.js';
import {hiddenObject} from '../client/src/world/resource-policy.js';
import {objectAnimationRate,objectUVScroll} from '../client/src/world/object-motion-policy.js';
const object=type=>({type,x:100,y:100,z:0,rx:0,ry:0,rz:0,scale:1});
test('DoppelGanger1 and 4 use their own source HIK markers, colors and double emission',()=>{
 for(const [map,types] of [[65,[70,80]],[68,[47]]]){
  const emitters=mapParticleEmitters(map,[6,32,47,70,80,96,247].map(object));
  assert.deepEqual(emitters.map(e=>e.type),types);
  for(const e of emitters){assert.equal(e.probability,1);assert.equal(hiddenObject(map,e.type),true);assert.deepEqual(mapEmissionKinds(e,()=>0),['hik1','hik1']);assert.deepEqual(mapEmissionKinds(e,()=>.99),['hik3','hik1']);assert.deepEqual(spawnMapParticle(e,'hik1',()=>0).light,e.type===80?[.7,.2,.1]:[.1,.4,1]);}
 }
 for(const type of [6,32,96,247])assert.equal(!!hiddenObject(65,type),false);
 assert.equal(!!hiddenObject(68,70),false);
 assert.equal(mapParticleEmitters(66,[70,80,47].map(object)).length,0);
});
test('event object rates and stream slots follow 25 Hz source increments',()=>{
 assert.equal(objectAnimationRate(67,40),1.25);
 for(const [type,rate] of [[44,.5],[46,.25],[70,1],[90,1]])assert.equal(objectAnimationRate(68,type),rate);
 assert.equal(objectAnimationRate(79,8),5);
 assert.deepEqual(objectUVScroll(68,77,2),[0,-.2]);
 assert.deepEqual(objectUVScroll(68,96,0),[0,-.05]);
 assert.deepEqual(objectUVScroll(68,96,1),[0,0]);
 assert.deepEqual(objectUVScroll(65,77,0),[0,0]);
});

test('Empire smoke132 emits two subtype60 particles and double-scale smoke21',()=>{
 const [e]=mapParticleEmitters(74,[132].map(object));assert.equal(e.probability,1/3);assert.equal(hiddenObject(74,132),true);assert.equal(hiddenObject(71,132),true);assert.deepEqual(mapEmissionKinds(e),['smoke60','smoke60','smoke']);const p=spawnMapParticle(e,'smoke60',()=>0);assert.equal(p.life,60);assert.equal(p.scale,.32);assert.deepEqual(p.light,[.4,.4,.4]);stepMapParticle(p,()=>0,0);assert.equal(p.life,59);assert.ok(Math.abs(p.scale-.36)<1e-8);assert.ok(Math.abs(p.light[0]-59/50*.4)<1e-8);assert.equal(spawnMapParticle(e,'smoke',()=>0).scale,.64);
});
