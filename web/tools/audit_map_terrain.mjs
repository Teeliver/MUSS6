// Reproducible decode/geometry audit for every physical map in the catalog.
import {readFile,writeFile} from 'node:fs/promises';import path from 'node:path';
import createCore from '../.build/wasm/frame-reader.mjs';
import {terrainData,terrainPatch} from '../client/src/world/terrain.js';
import {inspectTexture} from '../client/src/platform/assets/decode-texture.js';
const [data,output]=process.argv.slice(2);if(!data||!output)throw Error('Usage: DATA OUTPUT');
const catalog=JSON.parse(await readFile(new URL('../client/src/world/map-catalog.json',import.meta.url)));
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const read=async p=>new Uint8Array(await readFile(path.join(data,p))),rows=[];
for(const row of catalog.worlds){const result={map:row.map,world:row.world,terrain:row.terrain,issues:[...row.issues],visual:'not tested',objects:'not rendered'};try{
 const t=terrainData(row.map,await read(`World${row.world}/TerrainHeight.OZB`),await read(`World${row.world}/EncTerrain${row.world}.map`),await read(`World${row.world}/EncTerrain${row.world}.att`));result.decode='pass';
 if(row.terrain==='loadable'){const textures=[];result.animatedTextureFrames=0;for(const [index,p] of Object.entries(row.textures)){textures[index]=inspectTexture(core,p.toUpperCase().endsWith('.OZT')?'OZT':'OZJ',await read(p));for(const frame of row.textureAnimations?.[index]?.paths??[]){const image=inspectTexture(core,frame.toUpperCase().endsWith('.OZT')?'OZT':'OZJ',await read(frame));if(image.width!==textures[index].width||image.height!==textures[index].height)throw Error('Animated terrain dimensions mismatch: '+frame);result.animatedTextureFrames++;}}const meshes=terrainPatch(t,row.spawn.x,row.spawn.y,textures);if(!meshes.length||meshes.some(m=>!m.positions.every(Number.isFinite)))throw Error('Empty or nonfinite terrain patch');result.geometry='pass';result.drawGroups=meshes.length;result.vertices=meshes.reduce((n,m)=>n+m.positions.length/3,0);}
 }catch(e){result.issues.push(e.message);result.geometry='failed';}rows.push(result);
}
await writeFile(output,JSON.stringify({schema:1,scope:'terrain decoder + texture wrapper + CPU patch geometry; not browser/objects/parity',rows},null,2)+'\n');
console.log(`${rows.filter(r=>r.decode==='pass').length}/${rows.length} terrain decode; ${rows.filter(r=>r.geometry==='pass').length} texture/CPU patch pass; ${rows.filter(r=>r.geometry==='failed').length} decode/geometry failures. Remaining catalog issues retained.`);
