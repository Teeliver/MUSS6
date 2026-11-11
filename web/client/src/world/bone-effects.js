import {createModelPreview} from '../platform/rendering/model-preview.js';
import {objectAnimationRate} from './object-motion-policy.js';
import {boneEffectRecipes,boneParticleRecipes,boneEffectPoint,boneEffectStyle} from './bone-effect-policy.js';
export async function createBoneMapEffects({core,canvas,load,texture,entry,terrain,objects,resources,report=()=>{}}){
 const recipes=boneEffectRecipes(entry.map,objects),particleRecipes=boneParticleRecipes(entry.map,objects),all=[...recipes,...particleRecipes];
 if(!all.length)return {particleEmitters:[],emit(){return [];},draw(){},lights(){return [];},dispose(){}};
 const rigs=new Map(),issues=[],groups=new Map();let disposed=false,lastTick,states=[],particleStates=[],renderer;
 try{
  for(const type of new Set(all.map(r=>r.object.type))){let rig;try{
   const spec=resources.worlds.find(w=>w.map===entry.map)?.objectTypes[type];if(spec?.status!=='ready')throw Error('Model not ready');
   rig=new core.ModelAnimation(await load(spec.path));const bones=[...new Set(all.filter(r=>r.object.type===type).map(r=>r.bone))],test=rig.sampleBonesResult(0,bones);if(!test.ok)throw Error(test.error);rigs.set(type,{rig,bones});
  }catch(e){rig?.delete();issues.push('type'+type+': '+e.message);}}
  const valid=recipes.filter(r=>rigs.has(r.object.type)),dropped=Math.max(0,valid.length-1000),initial=valid.slice(0,1000),ground=terrain.height(entry.x,entry.y),activeParticles=particleRecipes.filter(r=>rigs.has(r.object.type)).slice(0,3000);
  let active=initial;const failedTextures=new Set();for(const r of active){if(failedTextures.has(r.texture))continue;if(!groups.has(r.texture)){try{groups.set(r.texture,{recipes:[],image:await texture(r.texture)});}catch(e){issues.push(r.texture+': '+e.message);failedTextures.add(r.texture);continue;}}groups.get(r.texture).recipes.push(r);}active=active.filter(r=>groups.has(r.texture));
  for(const g of groups.values()){const n=g.recipes.length,uv=new Float32Array(n*12);for(let i=0;i<n;i++)uv.set([0,0,1,0,1,1,0,0,1,1,0,1],i*12);g.mesh={positions:new Float32Array(n*18),normals:new Float32Array(n*18).fill(1),colors:new Float32Array(n*24),uv,image:g.image,additive:true,clampUV:true,castsShadow:false};}
  if(groups.size)renderer=createModelPreview(canvas.getContext('webgl2'),[{meshes:[...groups.values()].map(g=>g.mesh),bounds:{center:[0,0,0],scale:1}}],{world:true});
  if(issues.length)report('Bone map effects: '+issues.join('; '));
  const refresh=time=>{const tick=Math.floor(time/40);if(tick===lastTick)return;lastTick=tick;const cache=new Map();
   const point=r=>{const key=r.object.type+':'+objectAnimationRate(entry.map,r.object.type,r.object.scale);if(!cache.has(key)){const {rig,bones}=rigs.get(r.object.type),sample=rig.sampleBonesResult(time/1000*objectAnimationRate(entry.map,r.object.type,r.object.scale)/6.25,bones);if(!sample.ok)throw Error('Bone map pose: '+sample.error);cache.set(key,new Map(bones.map((b,i)=>[b,sample.matrices[i]])));}return boneEffectPoint(r.object,cache.get(key).get(r.bone),r.offset);};
   states=active.map(r=>({recipe:r,point:point(r),...boneEffectStyle(r,time)}));
   particleStates=activeParticles.map(r=>{const p=point(r);return {map:entry.map,type:r.object.type,x:p.x*100,y:p.y*100,z:p.z*100,rx:0,ry:-1,rz:0,scale:r.object.scale*r.ratio,light:r.light,kinds:r.kinds,probability:0};});
  };
  const corners=[[-1,-1],[1,-1],[1,1],[-1,-1],[1,1],[-1,1]];
  return {
   particleEmitters:activeParticles,
   emit(time,focus){if(disposed)return [];refresh(time);return particleStates.filter(p=>Math.abs(p.x*.01-focus.x)<24&&Math.abs(p.y*.01-focus.y)<24);},
   lights(time,focus){if(disposed)return [];refresh(time);return states.filter(s=>s.terrain&&(s.recipe.style!=='market30'||s.recipe.bone===1)&&Math.hypot(s.point.x-focus.x,s.point.y-focus.y)<32).map(s=>({x:s.point.x-entry.x,y:s.point.y-entry.y,range:s.terrain.range,color:s.terrain.color,intensity:1}));},
   draw(w,h,time,focus){if(disposed)return;refresh(time);for(const g of groups.values()){let n=0;for(const s of states.filter(s=>s.recipe.texture===g.recipes[0].texture)){const width=g.image.width*s.scale*.005,height=g.image.height*s.scale*.005;for(let i=0;i<6;i++){const [u,v]=corners[i];g.mesh.positions.set([s.point.x-entry.x+u*.707*width-v*.514*height,s.point.y-entry.y+u*.707*width+v*.514*height,s.point.z-ground+v*.686*height],n*18+i*3);g.mesh.colors.set([...s.light,1],n*24+i*4);}n++;}}
    if(renderer){renderer.update(0,{meshes:[...groups.values()].map(g=>g.mesh)});renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-ground);renderer.draw(w,h,time,{clear:false,pass:'objects'});}
    canvas.dataset.mapBoneSprites=String(active.length);canvas.dataset.mapBoneParticleEmitters=String(activeParticles.length);canvas.dataset.mapBoneIssues=String(issues.length);canvas.dataset.mapBoneDropped=String(dropped+Math.max(0,particleRecipes.filter(r=>rigs.has(r.object.type)).length-3000));
   },
   dispose(){if(disposed)return;disposed=true;renderer?.dispose();for(const {rig} of rigs.values())rig.delete();rigs.clear();states=[];particleStates=[];delete canvas.dataset.mapBoneSprites;delete canvas.dataset.mapBoneParticleEmitters;delete canvas.dataset.mapBoneIssues;delete canvas.dataset.mapBoneDropped;}
  };
 }catch(e){renderer?.dispose();for(const {rig} of rigs.values())rig.delete();throw e;}
}
