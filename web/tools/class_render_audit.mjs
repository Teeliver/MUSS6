// Seven-class render/clip audit; not a skill eligibility or combat certification.
import {readFile} from 'node:fs/promises';
import {resolve} from 'node:path';
import createCore from '../.build/wasm/frame-reader.mjs';
import {bodyPaths} from '../client/src/preview/body-geometry.js';
import {fireProjectile} from '../client/src/preview/fire-projectile.js';
const root=process.argv[2];if(!root)throw Error('Explicit Data root required');
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const rig=await readFile(resolve(root,'Player/Player.bmd')),fire=new core.ModelAnimation(await readFile(resolve(root,'Skill/Fire01.bmd'))),classes=[];
try{for(const [baseClass,name] of ['DW','DK','Elf','MG','DL','Summoner','RF'].entries()){
 const idle=baseClass===2?2:baseClass===5?3:1,walk=baseClass===2||baseClass===5?16:15,body=new core.BodyAnimation(rig,idle);
 try{for(const path of bodyPaths('base',{core,baseClass}))body.addPart(await readFile(resolve(root,path)));body.configureSpell(rig,walk,25,idle,38);
 let vertices=0;for(const [time,mode] of [[0,0],[.06,5],[.12,5],[.3,4],[.5,1],[.7,2],[1,3],[1.3,0]]){
 const sample=body.sampleMotionResult(time,mode),effect=fire.sampleResult(time%2);if(!sample.ok||!effect.ok)throw Error(sample.error||effect.error);
 const combined=[...sample.meshes,...fireProjectile(effect,time).meshes];for(const mesh of combined)for(const value of mesh.positions)if(!Number.isFinite(value))throw Error('Nonfinite class/effect geometry');vertices=combined.reduce((n,m)=>n+m.positions.length/3,0);
 }
 const times=[];for(let tick=0;tick<630;tick++){const time=2+tick/120,mode=time%2<.6?(Math.floor(time/2)%2?4:5):0,start=performance.now();const sample=body.sampleMotionResult(time,mode),effect=fire.sampleResult(time%2);if(!sample.ok||!effect.ok)throw Error(sample.error||effect.error);fireProjectile(effect,time);if(tick>=30)times.push(performance.now()-start);}times.sort((a,b)=>a-b);const cpu={samples:times.length,p95Ms:times[Math.ceil(times.length*.95)-1],maxMs:times.at(-1),scope:'body + Fire sample/copy/transform, excludes WebGL/GPU'};
 classes.push({baseClass,name,renderClips:'pass',fireGeometry:'pass',vertices,cpu,skills:'NOT VERIFIED: eligibility, per-class skill set, packets, hits/effects beyond Fire01',warnings:baseClass===6?['Native male idle fallback remains unverified for RF']:[]});
 }catch(error){classes.push({baseClass,name,renderClips:'FAIL',error:error.message});}finally{body.delete();}
}}finally{fire.delete();}
console.log(JSON.stringify({scope:'base tier0 bodies; shared native cast146/147 and synthetic Fire01 target, no skill grants',classes},null,2));
if(classes.some(c=>c.renderClips==='FAIL'))process.exitCode=1;
