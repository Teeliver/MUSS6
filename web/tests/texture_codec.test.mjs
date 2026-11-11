import test from 'node:test';
import assert from 'node:assert/strict';
import {inspectTexture,decodeOriginalTexture,textureReference} from '../client/src/platform/assets/decode-texture.js';
import {core} from './helpers/game-fixture.mjs';
function jpeg(){return Uint8Array.from([...new Uint8Array(24),255,216,255,192,0,17,8,0,2,0,2,3,1,17,0,2,17,0,3,17,0,255,218,0,12,3,1,0,2,0,3,0,0,63,0,0,255,217]);}
test('texture references preserve source directory and reject unsafe references',()=>{
 assert.equal(textureReference('Item/Wing01.bmd','elfin_wing.jpg'),'Item/elfin_wing.OZJ');assert.equal(textureReference('Player/HelmMale01.bmd','head_02.tga'),'Player/head_02.OZT');
 for(const name of ['../image.jpg','x/y.jpg','a.png','http://a.jpg'])assert.throws(()=>textureReference('Item/Test.bmd',name));
});
test('texture WASM validates OZJ dimensions and decodes OZT channel/alpha/origin',()=>{
 const info=inspectTexture(core,'OZJ',jpeg());assert.equal(info.width,2);assert.equal(info.height,2);assert.equal(info.jpeg,true);assert.deepEqual([...info.bytes.slice(0,2)],[255,216]);
 const bad=jpeg();bad[34]=0;assert.throws(()=>inspectTexture(core,'OZJ',bad));assert.throws(()=>inspectTexture(core,'OZJ',jpeg().slice(0,-1)));
 const tga=new Uint8Array(26);tga[6]=2;tga[16]=1;tga[18]=1;tga[20]=32;tga[21]=32;tga.set([3,2,1,127],22);
 assert.deepEqual([...inspectTexture(core,'OZT',tga).bytes],[1,2,3,127]);
});
test('browser JPEG adapter checks dimensions, cancellation and bitmap disposal',async()=>{
 let closed=0;const bitmap={width:2,height:2,close(){closed++;}};
 const image=await decodeOriginalTexture(core,'OZJ',jpeg(),{createBitmap:async(blob,options)=>{assert.equal(blob.type,'image/jpeg');assert.equal(options.premultiplyAlpha,'none');assert.equal(options.imageOrientation,'flipY');return bitmap;}});
 image.dispose();image.dispose();assert.equal(closed,1);
 await assert.rejects(decodeOriginalTexture(core,'OZJ',jpeg(),{createBitmap:async()=>({width:3,height:2,close(){closed++;}})}));assert.equal(closed,2);
 const controller=new AbortController();await assert.rejects(decodeOriginalTexture(core,'OZJ',jpeg(),{signal:controller.signal,createBitmap:async()=>{controller.abort();return bitmap;}}));assert.equal(closed,3);
 await assert.rejects(decodeOriginalTexture(core,'OZJ',jpeg(),{signal:controller.signal}));
});
