// Read-only local Data/server catalog; no server configuration is published.
import {readFile,readdir,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';import path from 'node:path';
import {terrainData,terrainTexturePaths,terrainTextureIndices,decryptTerrain} from '../client/src/world/terrain.js';
const [data,server,output]=process.argv.slice(2);if(!data||!server||!output)throw Error('Usage: DATA SERVER_DATA OUTPUT');
const hash=b=>createHash('sha256').update(b).digest('hex');
const tokenize=text=>text.split(/\r?\n/).map(line=>line.split('//')[0].match(/"[^"]*"|\S+/g)).filter(row=>row&&row[0]!=='end');
const moveBytes=await readFile(path.join(server,'Move/Move.txt')),gateBytes=await readFile(path.join(server,'Move/Gate.txt'));
const gates=new Map(tokenize(gateBytes.toString('latin1')).map(r=>[Number(r[0]),r.map(Number)]));
const moves=tokenize(moveBytes.toString('latin1')).map(r=>{const gate=gates.get(Number(r[11]));if(r.length!==12||!gate)throw Error('Unsupported move/gate row');return {id:Number(r[0]),name:r[1].replaceAll('"',''),money:Number(r[2]),minLevel:r[3]==='*'?null:Number(r[3]),maxLevel:r[4]==='*'?null:Number(r[4]),minReset:r[5]==='*'?null:Number(r[5]),maxReset:r[6]==='*'?null:Number(r[6]),enabled:r.slice(7,11).map(Number),gate:Number(r[11]),map:gate[2],x:Math.floor((gate[3]+gate[5])/2),y:Math.floor((gate[4]+gate[6])/2)};});
const worlds=[];
for(const directory of (await readdir(data)).filter(n=>/^World[1-9]\d*$/.test(n)).sort((a,b)=>Number(a.slice(5))-Number(b.slice(5)))){
 const world=Number(directory.slice(5)),map=world-1,dir=path.join(data,directory),names=await readdir(dir),row={map,world,names:[...new Set(moves.filter(m=>m.map===map).map(m=>m.name))],files:[],textures:{},issues:[],texturePolicy:'MapManager::OpenFileTile overrides + rendered terrain cells; water frame binding, UV wind deformation not yet ported',objects:'not rendered'};
 const read=async name=>{const bytes=new Uint8Array(await readFile(path.join(dir,name)));row.files.push({path:directory+'/'+name,bytes:bytes.length,sha256:hash(bytes)});return bytes;};
 try{const bytes=await read(`EncTerrain${world}.obj`),plain=decryptTerrain(bytes);if(plain.length<4||(plain.length-4)%30)throw Error('Unsupported object layout');const d=new DataView(plain.buffer);let nonfinite=0,negativeScale=0;for(let i=4;i<plain.length;i+=30){for(let j=0;j<7;j++)if(!Number.isFinite(d.getFloat32(i+2+j*4,true)))nonfinite++;if(d.getFloat32(i+26,true)<0)negativeScale++;}row.objectAudit={records:(plain.length-4)/30,nonfinite,negativeScale,rendered:false};}catch(e){row.objectAudit={error:e.message,rendered:false};}
 try{await read('TerrainLight.OZJ');}catch{row.lightIssue='TerrainLight.OZJ unavailable; renderer not yet using original lightmap';}
 try{
  const t=terrainData(map,await read('TerrainHeight.OZB'),await read(`EncTerrain${world}.map`),await read(`EncTerrain${world}.att`));
  const indices=terrainTextureIndices(t);row.textureAnimations={};row.ignoredTextureIndices=[...new Set([...t.layer1,...t.layer2])].filter(n=>n!==255&&!indices.includes(n));
  for(const index of indices){const logical=terrainTexturePaths(map,index);if(!logical.length){row.issues.push('No verified texture mapping for tile '+index);continue;}const resolved=[];
   for(let target of logical){let parent=path.dirname(target),expected=path.basename(target),entries=parent===directory?names:await readdir(path.join(data,parent)).catch(()=>[]),matches=entries.filter(n=>n.toLowerCase()===expected.toLowerCase());
    if(matches.length!==1&&map===3){parent='World1';entries=await readdir(path.join(data,parent));matches=entries.filter(n=>n.toLowerCase()===expected.toLowerCase());}
    if(matches.length!==1){row.issues.push('Missing/ambiguous texture '+target);continue;}
    const actual=parent+'/'+matches[0],bytes=await readFile(path.join(data,actual));row.files.push({path:actual,bytes:bytes.length,sha256:hash(bytes)});resolved.push(actual);
   }
   if(resolved.length===logical.length){row.textures[index]=resolved[0];if(resolved.length>1)row.textureAnimations[index]={paths:resolved,frameDuration:40};}
  }
  let spawn=moves.find(m=>m.map===map);if(!spawn){let found=128*256+128;for(let r=0;r<128;r++){let done=false;for(let y=Math.max(1,128-r);y<=Math.min(254,128+r);y++)for(let x=Math.max(1,128-r);x<=Math.min(254,128+r);x++)if(!(t.flags[y*256+x]&12)){found=y*256+x;done=true;break;}if(done)break;}spawn={x:found%256,y:Math.floor(found/256)};}
  row.spawn={x:spawn.x,y:spawn.y};row.heightMode=[42,63,66].includes(map)?'source-24bit':'source-raw8';row.terrain=row.issues.length?'blocked':'loadable';
 }catch(e){row.terrain='blocked';row.issues.push(e.message);row.spawn={x:128,y:128};}
 worlds.push(row);
}
await writeFile(output,JSON.stringify({schema:1,source:{moveSha256:hash(moveBytes),gateSha256:hash(gateBytes)},worlds,moves},null,2)+'\n');
console.log(`${worlds.length} physical worlds; ${worlds.filter(w=>w.terrain==='loadable').length} terrain-loadable; ${moves.length} server move destinations. Object/parity coverage not implied.`);
