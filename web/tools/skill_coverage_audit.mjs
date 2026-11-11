import {EvilSpirits,spiritGeometry} from '../client/src/preview/evil-spirit.js';
// Exhaustive catalog/dispatch audit; no claim that missing skills rendered.
import fs from 'node:fs';
import {skillRows,skillsFor,selectionUrl} from '../client/src/preview/menu.js';
import {skillSupport,skillCoverage} from '../client/src/preview/skill-support.js';
import {fireProjectile} from '../client/src/preview/fire-projectile.js';
import {energyProjectile} from '../client/src/preview/energy-projectile.js';
const path=process.argv[2];if(!path)throw new Error('Usage: node tools/skill_coverage_audit.mjs CATALOG_JSON');
const catalog=JSON.parse(fs.readFileSync(path,'utf8')),rows=skillRows(catalog),names=['DW','DK','Elf','MG','DL','Summoner','RF'],cases=[];
for(let cls=0;cls<7;cls++)for(let tier=0;tier<3;tier++){
 const allowed=skillsFor(rows,cls,tier),skills=[];
 for(const row of allowed){
  const support=skillSupport(row.index),url=selectionUrl('http://127.0.0.1:8080/debug/',{cls,tier,skill:row.index},rows);
  if(url.searchParams.get('effect')!==support.effect)throw new Error(`Dispatch mismatch ${cls}/${tier}/${row.index}`);
  let geometryChecked=false;
  if(support.effect){
   // Fire synthetic triangle checks transform only, not real BMD geometry/parity.
   const fixture={meshes:[{positions:Float32Array.of(0,0,0,1,0,0,0,1,0),normals:Float32Array.of(0,0,1,0,0,1,0,0,1),uv:new Float32Array(6)}]};
   for(const time of [0,.05,.12,.1999,.2399,.25,2.1]){const result=support.effect==='spirit'?spiritGeometry(fixture,new EvilSpirits(1),time):support.effect==='fire'?fireProjectile(fixture,time):energyProjectile(time);if(result.meshes.some(mesh=>!mesh.positions.every(Number.isFinite)||!mesh.normals.every(Number.isFinite)))throw new Error('Nonfinite effect');}
   geometryChecked=true;
  }
  skills.push({index:row.index,name:row.name,...support,dispatchChecked:true,syntheticGeometryChecked:geometryChecked,windowsParityVerified:false,gameplayVerified:false,url:url.href});
 }
 cases.push({class:cls,name:names[cls],tier,...skillCoverage(allowed),skills});
}
console.log(JSON.stringify({schema:1,sourceHash:catalog.sourceHash,scope:'Catalog/class/tier/menu dispatch and synthetic transform only. Missing effects are NOT rendered or passed.',catalogSkills:rows.length,completeSkills:0,cases},null,2));
