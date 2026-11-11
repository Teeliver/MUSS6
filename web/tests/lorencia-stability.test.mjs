import {test} from 'node:test';
import assert from 'node:assert/strict';
import {createRoofFade,isRoofObject,indoorRoofTile} from '../client/src/world/roof-fade.js';
import {fetchAsset} from '../client/src/platform/assets/fetch-asset.js';
test('Lorencia roofs follow original indoor tile, independent of frame rate',()=>{
 const terrain={layer1:new Uint8Array(65536)};terrain.layer1[20*256+10]=4;
 const samples=[25,60,120].map(fps=>{const fade=createRoofFade(0,terrain);assert.equal(fade.advance({x:0,y:0},0),1);let value;for(let i=0;i<fps;i++)value=fade.advance({x:10,y:20},1/fps);return value;});
 for(const value of samples)assert.ok(Math.abs(value-Math.pow(.9,25))<1e-12);
 assert.equal(createRoofFade(1,terrain).advance({x:10,y:20},0),1);
 assert.equal(createRoofFade(0,terrain).advance({x:10,y:20},0),0);
});
test('asset congestion retries but missing files do not',async()=>{
 let calls=0;const waits=[];const result=await fetchAsset('asset',{}, {request:async()=>new Response('',{status:++calls<3?429:200}),pause:async ms=>waits.push(ms)});assert.equal(result.status,200);assert.equal(calls,3);assert.equal(waits.length,2);
 calls=0;assert.equal((await fetchAsset('asset',{}, {request:async()=>{calls++;return new Response('',{status:404});}})).status,404);assert.equal(calls,1);
});

test('Devias indoor roofs follow source object types and HeroTile; unrelated maps remain opaque',()=>{for(const type of [81,82,96,98,99])assert.equal(isRoofObject(2,type),true);for(const type of [80,83,95,97,100])assert.equal(isRoofObject(2,type),false);assert.equal(isRoofObject(3,81),false);for(const tile of [3,10,11,12,255])assert.equal(indoorRoofTile(2,tile),true);for(const tile of [0,1,2,4,9])assert.equal(indoorRoofTile(2,tile),false);const terrain={layer1:new Uint8Array(65536)};terrain.layer1[20*256+10]=11;const fade=createRoofFade(2,terrain);assert.equal(fade.advance({x:10,y:20},0),0);const outside=fade.advance({x:0,y:0},1);assert.ok(outside>.9&&outside<1);});
