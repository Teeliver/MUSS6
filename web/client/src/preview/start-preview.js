import {EvilSpirits,spiritGeometry} from './evil-spirit.js';
import {energyProjectile} from './energy-projectile.js';
import {frameSamples} from '../diagnostics/frame-samples.js';
import {fireProjectile} from './fire-projectile.js';
import {fixtureMode} from './motion-fixture.js';
import {previewLoadout} from './loadouts.js';
import {createResourceManifest} from '../platform/assets/manifest.js';
import {createResourceLoader} from '../platform/assets/load-resource.js';
import {decodeBmdGeometry,decodeAttachmentGeometry} from '../platform/assets/decode-model.js';
import {textureReference,decodeOriginalTexture} from '../platform/assets/decode-texture.js';
import {createModelPreview,geometryBounds} from '../platform/rendering/model-preview.js';
import {bodyPaths,assembleBody,parseArmorFixture} from './body-geometry.js';
import {drawingSize} from '../platform/rendering/viewport.js';
export async function startPreview(canvas,core,status){
 const controller=new AbortController(),images=[];let spiritEffect,spiritHead,spiritAge=0,spiritBuffers,energyEffect=false,effectFrozen,projectileBuffers,bodyIndex=0,effectAnimation,effectCount=0,motionMode,animation,animationStart,renderer,frame=0,disposed=false,lost=false,count=0,last=performance.now(),cpuSamples=frameSamples(240),intervalSamples=frameSamples(3600),lastTick;
 const gl=canvas.getContext('webgl2',{alpha:false,antialias:false});if(!gl)throw new Error('WebGL2 unavailable');
 const dispose=()=>{if(disposed)return;disposed=true;controller.abort();cancelAnimationFrame(frame);document.removeEventListener('visibilitychange',visibility);canvas.removeEventListener('webglcontextlost',onLost);canvas.removeEventListener('webglcontextrestored',onRestored);renderer?.dispose();animation?.delete();effectAnimation?.delete();effectAnimation=undefined;animation=undefined;for(const image of images)image.dispose();};
 const tick=time=>{if(disposed||lost||document.hidden)return;if(lastTick!==undefined){intervalSamples.push(time-lastTick);}lastTick=time;const rect=canvas.getBoundingClientRect();if(rect.width&&rect.height){const [w,h]=drawingSize(rect.width,rect.height,window.devicePixelRatio||1);if(canvas.width!==w)canvas.width=w;if(canvas.height!==h)canvas.height=h;const started=performance.now();if(animation){if(animationStart===undefined)animationStart=time;const seconds=(time-animationStart)/1000,mode=fixtureMode(motionMode,seconds);const next=motionMode?animation.sampleMotionResult(seconds,mode):animation.sampleResult(seconds);if(!next.ok){dispose();status(`Animation lỗi: ${next.error}`);return;}if(effectAnimation){const effectTime=effectFrozen??seconds,effect=effectAnimation.sampleResult(effectTime%2);if(!effect.ok){dispose();status(effect.error);return;}projectileBuffers=fireProjectile(effect,effectTime,projectileBuffers);next.meshes.push(...projectileBuffers.meshes);}if(spiritEffect){const age=effectFrozen??(seconds%2);if(age<spiritAge){spiritEffect=new EvilSpirits(1);spiritAge=0;}spiritEffect.advance(age-spiritAge);spiritAge=age;spiritBuffers=spiritGeometry(spiritHead,spiritEffect,age,spiritBuffers);next.meshes.push(...spiritBuffers.meshes);}if(energyEffect){projectileBuffers=energyProjectile(effectFrozen??seconds,projectileBuffers);next.meshes.push(...projectileBuffers.meshes);}renderer.update(bodyIndex,next);}renderer.draw(w,h,time);cpuSamples.push(performance.now()-started);count++;if(time-last>=1000){document.querySelector('#fps').textContent=`FPS: ${(count*1000/(time-last)).toFixed(1)} · ${motionMode?(['idle','walk','run','attack','cast146','cast147'][fixtureMode(motionMode,(time-animationStart)/1000)]):'model preview'}`;document.querySelector('#render-metrics').textContent=`Buffer ${w} × ${h} · CPU frame p95 ${cpuSamples.percentile(.95).toFixed(2)} ms · interval p95 ${intervalSamples.length?intervalSamples.percentile(.95).toFixed(2):'—'} ms / ${intervalSamples.length} frames`;count=0;last=time;}}frame=requestAnimationFrame(tick);};
 const visibility=()=>{cancelAnimationFrame(frame);count=0;cpuSamples.clear();lastTick=undefined;last=performance.now();if(!document.hidden&&!lost&&!disposed)frame=requestAnimationFrame(tick);};
 const onLost=e=>{e.preventDefault();lost=true;cancelAnimationFrame(frame);status('Context lost — reload preview');};
 const onRestored=()=>{dispose();status('Context restored — reload preview');};
 try{
  const response=await fetch(new URL('./preview-assets/manifest.json',location.href),{cache:'no-store',signal:controller.signal});if(!response.ok)throw new Error('Run npm run stage:preview after build:debug');
  const text=await response.text();if(text.length>65536)throw new Error('Preview manifest too large');
  const manifest=createResourceManifest(JSON.parse(text),new URL('./preview-assets/Data/',location.href).href),loader=createResourceLoader({manifest});
  const models=[],imageCache=new Map(),byteCache=new Map();
  const load=async path=>{if(!byteCache.has(path))byteCache.set(path,await loader.load(path,{signal:controller.signal}));return byteCache.get(path);};
  const skin=async geometry=>{for(const mesh of geometry.meshes){const target=mesh.texturePath??textureReference(mesh.sourcePath,mesh.texture);if(!imageCache.has(target)){const bytes=await load(target),image=await decodeOriginalTexture(core,target.endsWith('.OZJ')?'OZJ':'OZT',bytes,{signal:controller.signal});images.push(image);imageCache.set(target,image);}mesh.image=imageCache.get(target);}};
  const query=new URLSearchParams(location.search),outfit=query.get('outfit')||'base',baseClass=Number(query.get('class')??0),tier=Number(query.get('tier')??0),armorTypes=parseArmorFixture(query.get('armor')),parts=[];
  for(const path of bodyPaths(outfit,{baseClass,tier,core,armorTypes})){status(`Preview tải ${path}`);parts.push({path,bytes:await load(path)});}
  const attachments=previewLoadout(core,query,baseClass);
  let action=baseClass===2?2:baseClass===5?3:1,safe=query.get('safe')==='1',walk=(baseClass===2||baseClass===5)?16:15,run=25,attack=38;
  motionMode=query.get('motion')||undefined;if(motionMode&&!['idle','walk','run','cycle','combat','attack','cast'].includes(motionMode))throw new Error('Invalid motion fixture');
  if(attachments.length){
   const right=attachments.find(item=>item.slot===0)?.index??-1,left=attachments.find(item=>item.slot===1)?.index??-1;
   const stance=core.equipmentActionsResult(baseClass,right,left,await load('Local/Eng/Item_eng.bmd'));
   if(!stance.ok)throw new Error(stance.error);attack=stance.attack;if(!safe){action=stance.idle;walk=stance.walk;run=stance.run;}
  }
  const body=assembleBody(core,parts,await load('Player/Player.bmd'),{outfit,baseClass,tier,armorTypes,action});
  for(const item of attachments){
   const geometry=decodeAttachmentGeometry(core,await load(item.path),await load('Player/Player.bmd'),{action,slot:item.slot,index:item.index,safe});
   for(const mesh of geometry.meshes)body.meshes.push({...mesh,sourcePath:item.path});
  }
  animation=new core.BodyAnimation(await load('Player/Player.bmd'),action);
  for(const part of parts)animation.addPart(part.bytes);
  for(const item of attachments)animation.addWeapon(await load(item.path),item.slot,item.index,safe);
  if(motionMode==='cast')animation.configureSpell(await load('Player/Player.bmd'),walk,run,baseClass===2?2:baseClass===5?3:1,attack);
  else if(motionMode==='combat'||motionMode==='attack')animation.configureCombat(await load('Player/Player.bmd'),walk,run,baseClass===2?2:baseClass===5?3:1,attack);
  else if(motionMode)animation.configureMotion(await load('Player/Player.bmd'),walk,run,baseClass===2?2:baseClass===5?3:1);
  const first=animation.sampleResult(0);if(!first.ok)throw new Error(first.error);
  if(first.meshes.length!==body.meshes.length)throw new Error('Animation topology mismatch');
  if(query.has('effect')){
   if(!['fire','energy','spirit'].includes(query.get('effect')))throw new Error('Unsupported preview effect');
   if(query.has('effect-time')){effectFrozen=Number(query.get('effect-time'));if(!Number.isFinite(effectFrozen)||effectFrozen<0||effectFrozen>=(query.get('effect')==='spirit'?2:query.get('effect')==='energy'?.2:.24))throw new Error('Effect inspection time exceeds lifetime');}
   if(query.get('effect')==='spirit'){spiritHead=decodeBmdGeometry(core,await load('Skill/Laser01.bmd'));for(const mesh of spiritHead.meshes)mesh.sourcePath='Skill/Laser01.bmd';spiritEffect=new EvilSpirits(1);spiritEffect.advance(effectFrozen??0);spiritAge=effectFrozen??0;const initial=spiritGeometry(spiritHead,spiritEffect,spiritAge);effectCount=initial.meshes.length;const bounds=[];const audit=new EvilSpirits(1);for(let i=0;i<50;i++){audit.step();bounds.push(...spiritGeometry(spiritHead,audit,i*.04).meshes);}body.bounds=geometryBounds([...body.meshes,...(effectFrozen===undefined?bounds:initial.meshes)]);body.meshes.push(...initial.meshes);}else if(query.get('effect')==='energy'){energyEffect=true;effectCount=1;const envelope=[...body.meshes,...energyProjectile(0).meshes,...energyProjectile(.1999).meshes];body.bounds=geometryBounds(envelope);body.meshes.push(...energyProjectile(effectFrozen??0).meshes);}else{
   effectAnimation=new core.ModelAnimation(await load('Skill/Fire01.bmd'));const effect=effectAnimation.sampleResult(effectFrozen??0);if(!effect.ok)throw new Error(effect.error);effectCount=effect.meshes.length;
   const envelope=[...body.meshes,...fireProjectile(effect,0).meshes,...fireProjectile(effect,.2399).meshes];body.bounds=geometryBounds(envelope);
   body.meshes.push(...fireProjectile(effect,effectFrozen??0).meshes);
   }
  }
  await skin(body);bodyIndex=models.length;models.push(body);
  byteCache.clear();imageCache.clear();
  renderer=createModelPreview(gl,models);for(const image of images)image.dispose();images.length=0;
  document.addEventListener('visibilitychange',visibility);canvas.addEventListener('webglcontextlost',onLost);canvas.addEventListener('webglcontextrestored',onRestored);
  status(`Preview nhân vật: class ${baseClass} / tier ${tier} clips ${action}/${walk}/${run}${motionMode==='combat'||motionMode==='attack'?` / attack ${attack}`:''}${motionMode==='cast'?' / cast 146/147':''}${motionMode?` / motion ${motionMode}`:''}${effectCount?` / ${spiritEffect?'Evil Spirit':energyEffect?'Energy':'Fire01'} visual fixture${effectFrozen===undefined?'':' (frozen)'}`:''} đủ 5 phần${attachments.length?` + ${attachments.map(item=>item.path.split('/').at(-1)).join(' / ')}`:''} (${armorTypes===undefined?outfit:`armor ${armorTypes.join(",")}`})`);visibility();return dispose;
 }catch(error){dispose();throw error;}
}
