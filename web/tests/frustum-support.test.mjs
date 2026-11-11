import test from 'node:test';import assert from 'node:assert/strict';import {performance} from 'node:perf_hooks';import {createFrustumTest} from '../client/src/world/frustum.js';import {worldCamera} from '../client/src/world/camera.js';
function reference(bounds,matrix,margin){const clips=[];for(const x of [bounds.min[0]-margin,bounds.max[0]+margin])for(const y of [bounds.min[1]-margin,bounds.max[1]+margin])for(const z of [bounds.min[2]-margin,bounds.max[2]+margin]){const p=[x,z,-y,1];clips.push([0,1,2,3].map(r=>p.reduce((s,v,c)=>s+v*matrix[c*4+r],0)));}for(let a=0;a<3;a++)if(clips.every(p=>p[a]<-p[3])||clips.every(p=>p[a]>p[3]))return false;return true;}
let seed=71683;const random=()=>((seed=(Math.imul(seed,1664525)+1013904223)>>>0)/4294967296);
test('support-plane culling preserves the eight-corner reference across cameras and margins',()=>{
 for(const matrix of [worldCamera(16/9,3,100,100),worldCamera(9/16,0,-10,20),worldCamera(1,20,200,80),Float32Array.of(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)])for(const margin of [0,2,8]){const visible=createFrustumTest(matrix,margin);for(let i=0;i<1500;i++){const min=[random()*500-150,random()*500-150,random()*40-20],max=min.map(v=>v+random()*30),bounds={min,max};assert.equal(visible(bounds),reference(bounds,matrix,margin));}}
});
test('support-plane culling retains camera-plane touching and straddling bounds',()=>{
 const visible=createFrustumTest(Float32Array.of(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),0);assert.equal(visible({min:[1,0,0],max:[2,0,0]}),true);assert.equal(visible({min:[1.01,0,0],max:[2,0,0]}),false);assert.equal(visible({min:[-2,-2,-2],max:[2,2,2]}),true);
});
