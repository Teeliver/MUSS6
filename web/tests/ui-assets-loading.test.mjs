import test from 'node:test';import assert from 'node:assert/strict';import {readFile} from 'node:fs/promises';
import {loadUiAssets} from '../client/src/ui/load-assets.js';import {uiAssets} from '../client/src/ui/asset-manifest.js';
test('UI loading bounds decoding to three workers and cleans completed images after another decoder fails',async()=>{
 const originalFetch=globalThis.fetch,originalBitmap=globalThis.createImageBitmap;let active=0,peak=0,created=0,closed=0,fail=false,decodes=0;let releaseFirst;let firstThree=new Promise(resolve=>{releaseFirst=resolve;});
 globalThis.fetch=async url=>new Response(await readFile('/Users/angelale/Downloads/MuClientEX603/Data/'+new URL(url).searchParams.get('path')));
 globalThis.createImageBitmap=async()=>{active++;peak=Math.max(peak,active);const n=++decodes;if(n===3)releaseFirst();if(n<=3)await firstThree;else await new Promise(r=>setImmediate(r));active--;if(fail&&n===2)throw Error('decoder failure');created++;return {close(){closed++;}};};
 const core={textureResult:()=>({ok:true,jpeg:true,bytes:new Uint8Array([1]),width:1,height:1})};
 try{const assets=await loadUiAssets(core);assert.equal(peak,3);assert.ok(assets.catalog.length);assert.ok(assets.texts.size);assert.equal(assets.images.size,Object.keys(uiAssets).length-2);assert.equal(active,0);assets.dispose();assert.equal(created,closed);
  fail=true;decodes=0;firstThree=new Promise(resolve=>{releaseFirst=resolve;});await assert.rejects(loadUiAssets(core),/decoder failure/);assert.equal(active,0);assert.equal(created,closed);await new Promise(r=>setImmediate(r));assert.equal(created,closed);
 }finally{globalThis.fetch=originalFetch;globalThis.createImageBitmap=originalBitmap;}
});
