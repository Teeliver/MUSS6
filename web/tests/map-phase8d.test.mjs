import test from 'node:test';import assert from 'node:assert/strict';
import {mapParticleEmitters,mapEmissionKinds,mapParticleTextures,spawnMapParticle,stepMapParticle,mapParticleCapacity,mapParticleMultiply} from '../client/src/world/map-particle-policy.js';import {mapSpriteEmitters,mapSpriteState} from '../client/src/world/map-sprite-policy.js';import {hiddenObject} from '../client/src/world/resource-policy.js';
const node=type=>({type,x:100,y:200,z:300,rx:45,ry:60,rz:90,scale:2});
test('Market flame picks exactly one original colored family each tick, without Raklion extra HIK1',()=>{
 const [e]=mapParticleEmitters(79,[node(57),node(58)]);assert.equal(e.probability,1);assert.deepEqual(e.kinds,['market1','market2','market3']);for(const [value,kind] of [[0,'market1'],[.4,'market2'],[.99,'market3']])assert.deepEqual(mapEmissionKinds(e,()=>value),[kind]);assert.equal(hiddenObject(79,57),true);assert.equal(hiddenObject(79,58),true);assert.equal(!!hiddenObject(78,57),false);
 for(const [kind,file] of [['market1','firehik01'],['market2','firehik02'],['market3','firehik03']]){assert.equal(mapParticleTextures[kind],`Effect/${file}.OZJ`);assert.equal(mapParticleMultiply(kind),false);}
});
test('colored HIK initialization keeps native life, scale, zero drift and world vertical gravity',()=>{
 const [e]=mapParticleEmitters(79,[node(57)]);for(const [kind,life,shrink] of [['market1',27,.05],['market2',12,.06],['market3',17,.07]]){const p=spawnMapParticle(e,kind,()=>0);assert.equal(p.life,life);assert.equal(p.scale,1.44);assert.equal(p.gravity,.064);assert.equal(p.alpha,0);assert.deepEqual([p.vx,p.vy,p.vz],[0,0,0]);stepMapParticle(p,()=>0);assert.equal(p.alpha,.2);assert.equal(p.x,1);assert.equal(p.y,2);assert.equal(p.z,3.064);assert.ok(Math.abs(p.scale-(1.44-shrink))<1e-8);assert.equal(p.rotation,3*Math.PI/180);}
});
test('HIK1 fades below15 ticks while cursedlich4/HIK3 fade below10; alpha is applied once',()=>{
 const [e]=mapParticleEmitters(79,[node(57)]);for(const kind of ['market1','market2','market3']){const p=spawnMapParticle(e,kind,()=>0);p.alpha=1;p.life=15;stepMapParticle(p,()=>0);assert.equal(p.alpha,kind==='market1'?.8:1);assert.deepEqual(p.light,[1,1,1]);p.life=10;p.alpha=1;stepMapParticle(p,()=>0);assert.equal(p.alpha,.8);p.alpha=.2;p.life=9;assert.equal(stepMapParticle(p,()=>0),false);}
});
test('Market light sprites preserve constant orange scale2 and subtype58 pulse scale20',()=>{
 const emitters=mapSpriteEmitters(79,[node(57),node(58),node(30)]);assert.equal(emitters.length,2);assert.deepEqual(mapSpriteState(emitters[0],123),{x:1,y:2,z:3,scale:4,light:[1,.2,0]});const p=mapSpriteState(emitters[1],0);assert.equal(p.scale,40);assert.ok(Math.abs(p.light[0]-.32)<1e-9);const low=mapSpriteState(emitters[1],3*Math.PI/2/.005);assert.deepEqual(low.light,[.2,.2,.1]);
});
test('family buffer bounds include worst-case RNG streak without changing old particle budgets',()=>{
 const emitters=mapParticleEmitters(79,Array.from({length:12},()=>node(57)));assert.equal(mapParticleCapacity('market1',emitters),372);assert.equal(mapParticleCapacity('market2',emitters),192);assert.equal(mapParticleCapacity('market3',emitters),252);assert.equal(mapParticleCapacity('market1',Array.from({length:1000},()=>emitters[0])),3000);assert.equal(mapParticleCapacity('fire',emitters),256);
});
