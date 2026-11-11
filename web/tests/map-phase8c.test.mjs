import test from 'node:test';import assert from 'node:assert/strict';
import {waterfallEmitters,spawnWaterfall,stepWaterfall,waterfallCapacity,waterfallLimit} from '../client/src/world/waterfall-policy.js';import {createWaterfallEffects} from '../client/src/world/waterfall-effects.js';import {hiddenObject} from '../client/src/world/resource-policy.js';
const node=type=>({type,x:10000,y:10000,z:0,rx:0,ry:0,rz:0,scale:2});
test('market markers bind the exact source waterfall family and probability while water dispatch remains separate from fire',()=>{
 const e=waterfallEmitters(79,[54,55,56,57,58].map(node));assert.deepEqual(e.map(e=>[e.kind,e.probability]),[[5,1],[3,1],[2,1/8]]);assert.deepEqual(waterfallEmitters(51,[54,58,59].map(node)).map(e=>[e.kind,e.probability]),[[2,.25],[5,1],[3,1]]);for(const t of [54,55,56])assert.equal(hiddenObject(79,t),true);assert.equal(hiddenObject(79,57),true);assert.equal(!!hiddenObject(78,54),false);
});
test('WATERFALL2 subtype4 preserves the final scale overwrite after native initialization/scatter RNG',()=>{
 const [e]=waterfallEmitters(79,[node(56)]),values=[0,.1,.2,.3,.4,.5,.9];let calls=0;const p=spawnWaterfall(e,()=>values[calls++]);assert.equal(calls,7);assert.equal(p.life,70);assert.equal(p.scale,3.1);assert.equal(p.x,99.96);assert.equal(p.y,99.98);assert.equal(p.z,0);assert.equal(p.vz,-.03);assert.equal(p.light,.4);stepWaterfall(p,()=>0);assert.equal(p.life,69);assert.equal(p.rotation,-1.1*Math.PI/180);assert.ok(Math.abs(p.scale-3.13)<1e-8);
});
test('WATERFALL3 subtype8 ignores placement scale; falling spray rotates velocity before gravity update',()=>{
 const [e]=waterfallEmitters(79,[node(55)]),values=[0,.1,.6,.8,.5,.5,.5];let calls=0;const p=spawnWaterfall(e,()=>values[calls++]);assert.equal(calls,7);assert.equal(p.scale,.9);assert.equal(p.life,30);assert.equal(p.vz,.08);stepWaterfall(p);assert.equal(p.z,.08);assert.equal(p.vz,.074);assert.equal(p.light,1/1.1);
 const [fall]=waterfallEmitters(79,[{...node(54),rx:90}]),f=spawnWaterfall(fall,()=>0);stepWaterfall(f);assert.ok(Math.abs(f.y-100.07)<1e-8);assert.ok(Math.abs(f.z)<1e-8);assert.ok(Math.abs(f.scale-1.595)<1e-8);
});
function fake(){const counts={draw:0,deleted:0},gl=new Proxy({COMPILE_STATUS:1,LINK_STATUS:2,NO_ERROR:0,getShaderParameter:()=>true,getProgramParameter:()=>true,getError:()=>0,getUniformLocation:()=>0,createShader:()=>({}),createProgram:()=>({}),createBuffer:()=>({}),createVertexArray:()=>({}),createTexture:()=>({}),deleteTexture:()=>counts.deleted++,drawArrays:()=>counts.draw++},{get:(o,k)=>k in o?o[k]:()=>{}});return {gl,counts};}
test('per-family capacity renders more than256 without silent clipping and disposes once',async()=>{
 const objects=Array.from({length:20},()=>node(55)),e=waterfallEmitters(79,objects);assert.equal(waterfallCapacity(e,3),600);assert.equal(waterfallCapacity(e,2),0);assert.equal(waterfallLimit,3000);assert.equal(waterfallCapacity(Array.from({length:1000},()=>({kind:2})),2),3000);
 const {gl,counts}=fake(),canvas={dataset:{},getContext:()=>gl},paths=[],layer=await createWaterfallEffects({canvas,entry:{map:79,x:100,y:100},terrain:{height:()=>0,heightAt:()=>0},objects,texture:async p=>{paths.push(p);return {kind:'rgba',width:1,height:1,pixels:new Uint8Array(4)};}});for(let i=0;i<60;i++)layer.draw(800,600,i*40,{x:100,y:100});assert.deepEqual(paths,['Effect/WATERFALL3.OZJ']);assert.ok(Number(canvas.dataset.mapWaterfallParticles)>256);assert.equal(canvas.dataset.mapWaterfallDropped,'0');layer.dispose();layer.dispose();const draws=counts.draw;layer.draw(800,600,2500,{x:100,y:100});assert.equal(counts.draw,draws);assert.equal(counts.deleted,1);assert.equal(canvas.dataset.mapWaterfallParticles,undefined);
});
test('waterfall load errors identify map and texture; empty maps allocate nothing',async()=>{
 const options={canvas:{getContext(){throw Error('unexpected GL');}},entry:{map:79},objects:[node(54)],texture:async()=>{throw Error('missing');}};await assert.rejects(createWaterfallEffects(options),/Map 79 waterfall Effect\/WATERFALL5.OZJ: missing/);const empty=await createWaterfallEffects({...options,entry:{map:0}});empty.draw();empty.dispose();
});
