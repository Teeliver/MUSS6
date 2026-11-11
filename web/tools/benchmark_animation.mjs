import {fixtureMode} from '../client/src/preview/motion-fixture.js';
// CPU-only repeatable workload; does not measure browser draw/GPU or screen refresh.
import {readFile} from 'node:fs/promises';
import {resolve} from 'node:path';
import createCore from '../.build/wasm/frame-reader.mjs';
import {previewLoadout} from '../client/src/preview/loadouts.js';
import {bodyPaths} from '../client/src/preview/body-geometry.js';
const root=process.argv[2];if(!root)throw Error('Explicit Data root required');
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const fixture=process.argv.find(arg=>arg.startsWith('--loadout='))?.split('=')[1],baseClass=fixture==='bow'?2:0;
const attachments=previewLoadout(core,new URLSearchParams(fixture?{loadout:fixture}:{weapon:'0'}),baseClass);
const right=attachments.find(i=>i.slot===0)?.index??-1,left=attachments.find(i=>i.slot===1)?.index??-1;
const stance=core.equipmentActionsResult(baseClass,right,left,await readFile(resolve(root,'Local/Eng/Item_eng.bmd')));if(!stance.ok)throw Error(stance.error);
const animation=new core.BodyAnimation(await readFile(resolve(root,'Player/Player.bmd')),stance.idle);
try{
 for(const path of bodyPaths('base',{core,baseClass}))animation.addPart(await readFile(resolve(root,path)));
 for(const item of attachments)animation.addWeapon(await readFile(resolve(root,item.path)),item.slot,item.index,false);
 const cast=process.argv.includes('--cast'),combat=process.argv.includes('--combat'),motion=cast||combat||process.argv.includes('--motion');if(cast)animation.configureSpell(await readFile(resolve(root,'Player/Player.bmd')),stance.walk,stance.run,baseClass===2?2:1,stance.attack);else if(combat)animation.configureCombat(await readFile(resolve(root,'Player/Player.bmd')),stance.walk,stance.run,baseClass===2?2:1,stance.attack);else if(motion)animation.configureMotion(await readFile(resolve(root,'Player/Player.bmd')),stance.walk,stance.run,baseClass===2?2:1);
 const timings=[];let first,last,vertices=0;
 for(let tick=0;tick<(motion?6030:630);tick++){const start=performance.now(),sample=motion?animation.sampleMotionResult(tick/120,cast?fixtureMode('cast',tick/120):combat?fixtureMode('combat',tick/120):Math.floor(tick/360)%3):animation.sampleResult(tick/120),elapsed=performance.now()-start;if(!sample.ok)throw Error(sample.error);if(tick===30)first=sample.meshes[1].positions;if(tick===(motion?6029:629))last=sample.meshes[1].positions;vertices=sample.meshes.reduce((n,m)=>n+m.positions.length/3,0);if(tick>=30)timings.push(elapsed);}
 if(!first.some((v,i)=>Math.abs(v-last[i])>.0001))throw Error('Fixture pose did not animate');
 timings.sort((a,b)=>a-b);console.log(JSON.stringify({workload:`${fixture??'sword'} ${cast?'cast147/146/idle':combat?'idle/walk/run/attack':motion?'idle/walk/run':'idle'} CPU sample/owned WASM-JS copy only`,samples:timings.length,timelineHz:120,vertices,p50Ms:timings[Math.ceil(timings.length*.5)-1],p95Ms:timings[Math.ceil(timings.length*.95)-1],maxMs:timings.at(-1),note:'Excludes WebGL buffer updates, render/GPU/compositor; not an FPS benchmark'},null,2));
}finally{animation.delete();}
