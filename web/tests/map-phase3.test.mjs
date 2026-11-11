import test from 'node:test';import assert from 'node:assert/strict';
import {mapParticleEmitters,mapEmissionKinds,mapParticleMultiply,spawnMapParticle,stepMapParticle,nativeMapLightNodes,nativeMapLightColor} from '../client/src/world/map-particle-policy.js';
import {objectAnimationRate,objectUVScroll,objectMaterialPulse,objectMaterialTint} from '../client/src/world/object-motion-policy.js';import {hiddenObject,blendMesh} from '../client/src/world/resource-policy.js';
const node=type=>({type,x:10000,y:10000,z:200,rx:0,ry:0,rz:0,scale:2});
test('Raklion uses a random HIK family plus HIK1 each tick, with original blue/red colors and lifetime',()=>{
 for(const map of [57,58]){const emitters=mapParticleEmitters(map,[70,80,247].map(node));assert.equal(emitters.length,2);for(const [random,kinds] of [[0,['hik1','hik1']],[.4,['hik2','hik1']],[.9,['hik3','hik1']]])assert.deepEqual(mapEmissionKinds(emitters[0],()=>random),kinds);
  const p=spawnMapParticle(emitters[0],'hik1',()=>0);assert.deepEqual(p.light,[.1,.4,1]);assert.equal(p.life,27);assert.equal(p.scale,1.44);assert.equal(p.gravity,.1);assert.equal(p.alpha,0);stepMapParticle(p,()=>0);assert.equal(p.life,26);assert.equal(p.alpha,.2);assert.ok(Math.abs(p.x-100-.05)<1e-8);assert.ok(Math.abs(p.y-100+.14)<1e-8);assert.equal(p.z,2.1);assert.ok(Math.abs(p.scale-1.37)<1e-8);
  assert.deepEqual(spawnMapParticle(emitters[1],'hik3',()=>0).light,[.7,.2,.1]);assert.equal(spawnMapParticle(emitters[0],'hik2',()=>0).life,24);assert.equal(spawnMapParticle(emitters[0],'hik3',()=>0).life,17);assert.equal(hiddenObject(map,70),true);assert.equal(!!hiddenObject(map,247),false);
 }
});
test('Kanturu cloud10 uses subtractive blending and cloud11 retains colored drift/distance expiry',()=>{
 const emitters=mapParticleEmitters(38,[48,50,51,45,49,54].map(node));assert.deepEqual(emitters.map(e=>e.type),[48,50,51]);assert.equal(mapParticleMultiply('cloudDark'),true);assert.equal(mapParticleMultiply('cloudTint'),false);
 const dark=spawnMapParticle(emitters[0],'cloudDark',()=>0);assert.equal(dark.life,500);assert.deepEqual(dark.light,[0,0,0]);stepMapParticle(dark,()=>0);assert.deepEqual(dark.light,[.01,.01,.01]);assert.ok(dark.x>dark.start[0]);
 const blue=spawnMapParticle(emitters[1],'cloudTint',()=>0);assert.deepEqual(blue.light,[0,.01,.02]);assert.deepEqual(spawnMapParticle(emitters[2],'cloudTint',()=>0).light,[.01,0,0]);assert.equal(stepMapParticle(blue,()=>0),true);assert.ok(blue.rotation>0);blue.x=blue.start[0]+6;assert.equal(stepMapParticle(blue,()=>0),false);
 assert.equal(hiddenObject(38,51),true);assert.equal(!!hiddenObject(38,22),false);assert.equal(!!hiddenObject(38,69),false);
});
test('Kanturu WATERFALL3 subtype7 respects source temporary scale, absolute height and velocity update',()=>{
 const emitters=mapParticleEmitters(38,[10,52].map(node));assert.equal(emitters[0].z,410);assert.equal(emitters[1].z,200);assert.equal(emitters[0].probability,1/6);const p=spawnMapParticle(emitters[0],'water7',()=>0);assert.equal(p.life,60);assert.equal(p.scale,.06);assert.equal(p.vz,.008);assert.ok(Math.abs(p.z-4.01)<1e-8);stepMapParticle(p,()=>0);assert.equal(p.life,59);assert.ok(Math.abs(p.vz-.0081)<1e-8);assert.deepEqual(p.light,[.97,.97,.97]);
});
test('Tarkan preserves texture slots, missing-object warnings and phase per rotated lava placement',()=>{
 assert.equal(objectAnimationRate(38,10),1);assert.equal(objectAnimationRate(38,44),.5);assert.deepEqual(objectUVScroll(38,42,0),[-.2,-.2]);assert.equal(blendMesh(8,2),0);assert.equal(blendMesh(8,7),0);assert.equal(blendMesh(8,61),1);assert.equal(blendMesh(8,11),-1);assert.deepEqual(objectUVScroll(8,12,0),[-.05,-.05]);assert.deepEqual(objectUVScroll(8,61,1),[0,-1]);assert.deepEqual(objectUVScroll(8,61,0),[0,0]);
 const a=objectMaterialPulse(8,node(7),0),b=objectMaterialPulse(8,{...node(7),rz:90},0);assert.equal(a,0);assert.equal(b,18);assert.notDeepEqual(objectMaterialTint(a,0),objectMaterialTint(b,0));assert.equal(objectMaterialPulse(8,node(7),1),undefined);assert.equal(objectMaterialPulse(0,node(7),0),undefined);assert.equal(!!hiddenObject(8,35),false);
 const lamps=nativeMapLightNodes(8,[4,7,61].map(node));assert.deepEqual(lamps.map(l=>l.range),[3,3,2]);assert.deepEqual(nativeMapLightColor(lamps[0],0),[.65,.65,.65]);assert.deepEqual(nativeMapLightColor(lamps[1],0),[.65,.39,.13]);
});
test('environment keeps rotated lava pulses in separate batches and exposes their live material',async()=>{
 const {core}=await import('./helpers/game-fixture.mjs'),{fixture}=await import('./helpers/bmd-fixture.mjs'),{loadEnvironment}=await import('../client/src/world/environment.js');
 const plain=new Uint8Array(64);plain[1]=9;const d=new DataView(plain.buffer);for(let i=0;i<2;i++){const p=4+i*30;d.setInt16(p,7,true);for(const [n,value] of [10000,10000,0,0,0,i*90,1].entries())d.setFloat32(p+2+n*4,value,true);}const key=[209,115,82,246,210,154,203,39,62,175,89,49,55,179,231,162];let prev=94;const bytes=Uint8Array.from(plain,(v,i)=>{const value=((v+prev)&255)^key[i%16];prev=(value+61)&255;return value;});
 const env=await loadEnvironment({core,entry:{map:8,x:100,y:100},terrain:{height:()=>0},load:async p=>p.endsWith('.obj')?bytes:fixture(),texture:async()=>({width:1,height:1}),resources:{worlds:[{map:8,objectTypes:{7:{status:'ready',path:'Object9/Object08.bmd',meshes:[{texture:'Object9/t.OZT'}]}}}]}});
 try{assert.equal(env.meshes.length,2);assert.ok(env.meshes.every(m=>m.additive));assert.notDeepEqual(env.meshes[0].materialAt(0),env.meshes[1].materialAt(0));assert.notDeepEqual(env.meshes[0].materialAt(0),env.meshes[0].materialAt(1000));}finally{env.dispose();}
});
