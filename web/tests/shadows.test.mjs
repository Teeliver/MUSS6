import test from 'node:test';import assert from 'node:assert/strict';
import {createShadowField,projectShadow} from '../client/src/world/shadow-field.js';
test('shadow projection matches original CalcShadowPosition and samples receiving terrain',()=>{
 const point=[1,2,3],origin=[0,0,0],height=(x,y)=>x*.2+y*.3,result=projectShadow(point,origin,height);
 assert.ok(Math.abs(result[0]-(1+3*21/(3-40)))<1e-12);assert.equal(result[1],2);assert.equal(result[2],height(result[0],2)+.05);
 assert.ok(projectShadow([1,2,40],origin,height).every(Number.isFinite));const moved=projectShadow([11,12,8],[10,10,5],()=>5);assert.ok(Math.abs(moved[0]-(10+result[0]))<1e-12);
});
test('one shared height texture owns original terrain heights, restores texture unit, disposes once',()=>{
 const calls=[],gl=new Proxy({NO_ERROR:0,getError:()=>0,createTexture:()=>({}),texImage2D:(...args)=>calls.push(args),deleteTexture:()=>calls.push('deleted'),activeTexture:unit=>calls.push(unit),TEXTURE0:10,TEXTURE1:11},{get:(o,k)=>k in o?o[k]:()=>{}}),field=createShadowField(gl,{height:(x,y)=>x+y*2},{map:0,x:100,y:100});const image=calls.find(Array.isArray),heights=image.at(-1);assert.equal(heights.length,65536);assert.equal(heights[2*256+3],7);assert.deepEqual(field.entry,[100,100,300]);assert.equal(calls.at(-1),10);field.dispose();field.dispose();assert.equal(calls.filter(c=>c==='deleted').length,1);
});
