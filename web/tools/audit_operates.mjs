import {readFile,writeFile} from 'node:fs/promises';
import path from 'node:path';
import {worldObjects} from '../client/src/world/resource-policy.js';
import {operatePoints} from '../client/src/world/operate-policy.js';
const [data,output]=process.argv.slice(2);if(!data||!output)throw Error('Usage DATA OUTPUT');
const maps=JSON.parse(await readFile(new URL('../client/src/world/map-catalog.json',import.meta.url))),rows=[];
for(const map of maps.worlds){const objects=worldObjects(map.map,new Uint8Array(await readFile(path.join(data,`World${map.world}/EncTerrain${map.world}.obj`)))).objects;const points=operatePoints(map.map,objects);if(points.length)rows.push({map:map.map,points:points.map(p=>({type:p.type,x:p.tx,y:p.ty,z:p.z,angle:p.rz,kind:p.kind,cursor:p.cursor,faceObject:p.faceObject,restriction:p.restriction}))});}
await writeFile(output,JSON.stringify({scope:'Native registered operate placements; castle throne gated, live Castle Siege state not available',rows},null,2)+'\n');console.log(rows.map(r=>`${r.map}: ${r.points.length}`).join(', '));
