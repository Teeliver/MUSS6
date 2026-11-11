import test from 'node:test';import assert from 'node:assert/strict';import {terrainTexturePaths,terrainCellLayers,terrainTextureIndices,terrainPatch} from '../client/src/world/terrain.js';
const terrain=map=>({map,flags:new Uint16Array(65536).fill(8),layer1:new Uint8Array(65536),layer2:new Uint8Array(65536).fill(255),alpha:new Uint8Array(65536),height:()=>0});
test('terrain binding follows native per-map alpha/lava overrides and all 32 Atlans water frames',()=>{
 for(const [map,slot,path] of [[51,2,'World52/AlphaTileGround01.OZT'],[77,2,'World78/AlphaTileGround01.OZT'],[78,2,'World79/AlphaTileGround01.OZT'],[39,3,'World40/AlphaTileGround02.OZT'],[46,4,'World47/AlphaTileGround03.OZT'],[63,11,'Object64/song_lava1.OZJ'],[66,11,'Object64/song_lava1.OZJ'],[69,10,'World70/AlphaTile01.OZT'],[74,10,'World75/AlphaTile01.OZT'],[80,12,'World81/AlphaTile01.OZT']])assert.deepEqual(terrainTexturePaths(map,slot),[path]);
 assert.deepEqual(terrainTexturePaths(0,11),['World1/TileRock05.OZJ']);assert.deepEqual(terrainTexturePaths(0,14),['World1/ExtTile01.OZJ']);assert.deepEqual(terrainTexturePaths(0,48),['World1/ExtTile35.OZJ']);assert.deepEqual(terrainTexturePaths(0,49),[]);
 for(const map of [7,67]){const paths=terrainTexturePaths(map,256);assert.equal(paths.length,32);assert.equal(paths[0],'Object8/wt00.OZJ');assert.equal(paths.at(-1),'Object8/wt31.OZJ');}assert.deepEqual(terrainTexturePaths(0,256),[]);
});
test('only layers actually drawn require textures: hidden ground, zero-alpha overlays and covered base do not block scenes',()=>{
 const t=terrain(0);t.layer1.fill(12);t.layer2.fill(13);assert.deepEqual(terrainTextureIndices(t),[]);const i=257;t.flags[i]=0;t.layer1[i]=2;t.layer2[i]=5;assert.deepEqual(terrainTextureIndices(t),[2]);t.alpha[i]=100;assert.deepEqual(terrainTextureIndices(t),[2,5]);for(const k of [i,i+1,i+256,i+257])t.alpha[k]=255;assert.deepEqual(terrainTextureIndices(t),[5]);
 assert.deepEqual(terrainCellLayers(t,1,1),[{tile:5,overlay:false}]);
});
test('Atlans water overlay uses separate additive animated binding and source alpha luminance/UV scale',()=>{
 const t=terrain(67),i=257;t.flags[i]=0;t.layer1[i]=0;t.layer2[i]=5;t.alpha[i]=128;const frame={width:64,height:64},frames=[frame, {...frame}];const textures=[];textures[0]=frame;textures[256]={...frame,frames};assert.deepEqual(terrainTextureIndices(t),[0,256]);
 const meshes=terrainPatch(t,1,1,textures,1),water=meshes.find(m=>m.additive);assert.ok(water);assert.equal(water.alphaBlend,false);assert.equal(water.imageFrames,frames);assert.equal(water.frameDuration,40);assert.equal(water.uv[0],.25);assert.ok(Math.abs(water.colors[0]-128/255)<1e-7);assert.equal(water.colors[3],1);
 t.map=0;assert.deepEqual(terrainTextureIndices(t),[0,5]);
});
