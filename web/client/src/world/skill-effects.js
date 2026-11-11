import {decodeBmdGeometry} from '../platform/assets/decode-model.js';
import {decodeOriginalTexture,textureReference} from '../platform/assets/decode-texture.js';
import {createModelPreview} from '../platform/rendering/model-preview.js';
import {EvilSpirits,spiritGeometry} from '../preview/evil-spirit.js';
import {skillEffectSpec} from './skill-effect-catalog.js';
import {createEffectPlan,sampleEffectGeometry,cameraSource} from './skill-effect-runtime.js';

// W0115: 3 prototype gốc (Fire Ball/Evil Spirit/Energy Ball) giữ nguyên hình học đã
// kiểm chứng; 111 skill chủ động còn lại chạy qua catalog + runtime tổng quát.
// Effect không bao giờ được suy từ tên skill: catalog phải có family/mode tường minh.
export function createSkillEffects({core,canvas,entry,terrain,load,report=()=>{}}){
 const effects=new Set(),images=new Map();let disposed=false,seed=1;
 const texture=async path=>{if(disposed)throw Error('Disposed skill effects');if(!images.has(path))images.set(path,decodeOriginalTexture(core,/\.ozt$/i.test(path)?'OZT':'OZJ',await load(path)));return images.get(path);};
 const place=(model,from)=>{const ground=terrain.height(entry.x,entry.y),z=terrain.heightAt(from.x+.5,from.y+.5)-ground;for(const mesh of model.meshes)for(let i=0;i<mesh.positions.length;i+=3){mesh.positions[i]=mesh.positions[i]*.009+from.x-entry.x+.5;mesh.positions[i+1]=mesh.positions[i+1]*.009+from.y-entry.y+.5;mesh.positions[i+2]=mesh.positions[i+2]*.009+z;}return model;};
 const quad=()=>({meshes:[{positions:new Float32Array(18),normals:new Float32Array(18).fill(1),uv:Float32Array.of(0,0,1,0,1,1,0,0,1,1,0,1),additive:true}]});
 const geometry=(e,age)=>{if(e.id===9){e.sim.advance(Math.max(0,age-e.age));e.age=age;return place(spiritGeometry(e.model,e.sim,age),e.from);}const distance=Math.max(.01,Math.hypot(e.to.x-e.from.x,e.to.y-e.from.y)),f=Math.min(1,age/(distance/(e.id===4?12.5:15))),dx=e.to.x-e.from.x,dy=e.to.y-e.from.y,z0=terrain.heightAt(e.from.x+.5,e.from.y+.5)-terrain.height(entry.x,entry.y)+1.2,z1=terrain.heightAt(e.to.x+.5,e.to.y+.5)-terrain.height(entry.x,entry.y)+1;if(e.id===4){const model=e.rig.sampleResult(age);if(!model.ok)throw Error(model.error);const angle=Math.atan2(dy,dx)+Math.PI/2,c=Math.cos(angle),s=Math.sin(angle);for(const [n,mesh] of model.meshes.entries()){mesh.image=e.model.meshes[n].image;mesh.additive=n===1;for(let i=0;i<mesh.positions.length;i+=3){const x=mesh.positions[i]*.01,y=mesh.positions[i+1]*.01;mesh.positions[i]=c*x-s*y+e.from.x-entry.x+.5+dx*f;mesh.positions[i+1]=s*x+c*y+e.from.y-entry.y+.5+dy*f;mesh.positions[i+2]=mesh.positions[i+2]*.01+z0+(z1-z0)*f;const nx=mesh.normals[i],ny=mesh.normals[i+1];mesh.normals[i]=c*nx-s*ny;mesh.normals[i+1]=s*nx+c*ny;}}return model;}const m=quad(),corners=[[-1,-1],[1,-1],[1,1],[-1,-1],[1,1],[-1,1]];m.meshes[0].image=e.image;for(let i=0;i<6;i++){const [u,v]=corners[i];m.meshes[0].positions.set([e.from.x-entry.x+.5+dx*f+(u*.707-v*.514)*.3,e.from.y-entry.y+.5+dy*f+(u*.707+v*.514)*.3,z0+(z1-z0)*f+v*.686*.3],i*3);}return m;};
 const ground=from=>terrain.heightAt(from.x+.5,from.y+.5)-terrain.height(entry.x,entry.y);

 const spawnLegacy=async(id,from,to)=>{if(![4,9,17].includes(id)||disposed)return;const e={id,from:{...from},to:{...to},age:0,born:performance.now()};try{if(id===17)e.image=await texture('Effect/Thunder01.OZJ');else{const path=id===4?'Skill/Fire01.bmd':'Skill/Laser01.bmd',bytes=await load(path);e.model=decodeBmdGeometry(core,bytes);for(const mesh of e.model.meshes){mesh.sourcePath=path;mesh.image=await texture(textureReference(path,mesh.texture));}if(id===4)e.rig=new core.ModelAnimation(bytes);else{e.sim=new EvilSpirits(seed++);e.ribbonImage=await texture('Effect/JointSpirit01.OZJ');const initial=spiritGeometry(e.model,e.sim,0);for(const mesh of initial.meshes)if(mesh.texturePath)await texture(mesh.texturePath);}}if(disposed){e.rig?.delete();return;}e.born=performance.now();const initial=geometry(e,0);for(const mesh of initial.meshes)if(mesh.texturePath)mesh.image=await texture(mesh.texturePath);e.renderer=createModelPreview(canvas.getContext('webgl2'),[initial],{world:true});e.duration=id===9?2:Math.max(.04,Math.hypot(to.x-from.x,to.y-from.y)/(id===4?12.5:15));while(effects.size>=12){const old=effects.values().next().value;old.renderer.dispose();old.rig?.delete();effects.delete(old);}effects.add(e);}catch(error){e.renderer?.dispose();e.rig?.delete();if(!disposed)report('Skill effect: '+error.message);}};

 const spawnGeneric=async(id,from,to)=>{
  const spec=skillEffectSpec(id);
  if(!spec||spec.noVisual||disposed)return;
  let e;
  try{
   if(spec.model){
    const anchor=spec.anchor==='from'?from:to;
    const bytes=await load(spec.model),model=decodeBmdGeometry(core,bytes),scale=spec.modelScale??.01,z=ground(anchor),cx=anchor.x-entry.x+.5,cy=anchor.y-entry.y+.5;
    for(const mesh of model.meshes){mesh.image=await texture(textureReference(spec.model,mesh.texture));for(let i=0;i<mesh.positions.length;i+=3){mesh.positions[i]=mesh.positions[i]*scale+cx;mesh.positions[i+1]=mesh.positions[i+1]*scale+cy;mesh.positions[i+2]=mesh.positions[i+2]*scale+z;}}
    if(disposed)return;
    const renderer=createModelPreview(canvas.getContext('webgl2'),[model],{world:true});
    e={id,generic:true,staticModel:model,renderer,born:performance.now(),duration:spec.duration??1};
    while(effects.size>=12){const old=effects.values().next().value;old.renderer.dispose();old.rig?.delete();effects.delete(old);}
    effects.add(e);return;
   }
   const image=await texture(spec.texture);
   if(disposed)return;
   const plan=createEffectPlan({spec,from:{x:from.x-entry.x+.5,y:from.y-entry.y+.5,z:ground(from)},to:{x:to.x-entry.x+.5,y:to.y-entry.y+.5,z:ground(to)},seed:seed++});
   const initial=sampleEffectGeometry(plan,0,cameraSource({entry,terrain,focus:to}));
   for(const mesh of initial)mesh.image=image;
   const renderer=createModelPreview(canvas.getContext('webgl2'),[{meshes:initial}],{world:true});
   e={id,generic:true,plan,image,renderer,born:performance.now(),duration:plan.duration};
   while(effects.size>=12){const old=effects.values().next().value;old.renderer.dispose();old.rig?.delete();effects.delete(old);}
   effects.add(e);
  }catch(error){e?.renderer?.dispose();if(!disposed)report('Skill effect '+id+': '+error.message);}
 };

 return {async spawn(id,from,to){if(disposed)return;if([4,9,17].includes(id))return spawnLegacy(id,from,to);return spawnGeneric(id,from,to);},
  draw(w,h,time,focus){for(const e of effects){const age=(time-e.born)/1000;if(age>e.duration){e.renderer.dispose();e.rig?.delete();effects.delete(e);continue;}if(e.staticModel){e.renderer.update(0,e.staticModel);e.renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-terrain.height(entry.x,entry.y));e.renderer.draw(w,h,time,{clear:false,pass:'objects'});continue;}if(e.generic){const cam=cameraSource({entry,terrain,focus});const meshes=sampleEffectGeometry(e.plan,age,cam);for(const mesh of meshes)mesh.image=e.image;e.renderer.update(0,{meshes});e.renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-terrain.height(entry.x,entry.y));e.renderer.draw(w,h,time,{clear:false,pass:'objects'});continue;}const model=geometry(e,age);if(e.id===9)for(const mesh of model.meshes)if(mesh.texturePath)mesh.image=e.ribbonImage;e.renderer.update(0,model);e.renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-terrain.height(entry.x,entry.y));e.renderer.draw(w,h,time,{clear:false,pass:'objects'});}},
  dispose(){disposed=true;for(const e of effects){e.renderer.dispose();e.rig?.delete();}effects.clear();for(const promise of images.values())void promise.then(i=>i.dispose(),()=>{});images.clear();}};
}
