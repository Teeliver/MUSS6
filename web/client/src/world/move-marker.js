import {decodeBmdGeometry} from '../platform/assets/decode-model.js';
import {textureReference} from '../platform/assets/decode-texture.js';
import {createModelPreview} from '../platform/rendering/model-preview.js';
import {transformObject} from './resource-policy.js';
// Target selection: original model + cursorpin02 + BITMAP_MAGIC subtype11 (empact01).
// Arrival retains the earlier cursorpin pulse, separately timed after visual movement.
// Source joints/spark particles are not implemented yet.
export async function createMoveMarker({canvas,core,load,texture,terrain,entry}){
 const model=decodeBmdGeometry(core,await load('Effect/MoveTargetPosEffect.bmd'));
 for(const mesh of model.meshes){mesh.image=await texture(textureReference('Effect/MoveTargetPosEffect.bmd',mesh.texture));mesh.additive=true;}
 const positions=[],uv=[];for(let y=0;y<4;y++)for(let x=0;x<4;x++)for(const [dx,dy] of [[0,0],[1,1],[1,0],[0,0],[0,1],[1,1]]){positions.push((x+dx)/2-1,(y+dy)/2-1,0);uv.push((x+dx)/4,(y+dy)/4);}
 const rings=[];for(const name of ['cursorpin01','cursorpin02','empact01','empact01'])rings.push({positions:Float32Array.from(positions),normals:Float32Array.from(Array(positions.length/3).fill([0,0,1]).flat()),uv:Float32Array.from(uv),image:await texture('Effect/'+name+'.OZJ'),additive:true});
 const all=[...model.meshes,...rings];for(const mesh of all){mesh.colors=Float32Array.from(Array(mesh.positions.length/3).fill([1,.7,.3,1]).flat());}const white=model.meshes.map(mesh=>Float32Array.from(Array(mesh.positions.length/3).fill([1,1,1,1]).flat()));for(let i=0;i<model.meshes.length;i++)model.meshes[i].colors=white[i];const initial={meshes:all.map(m=>({...m,opacity:0}))},renderer=createModelPreview(canvas.getContext('webgl2'),[initial],{world:true});let destination,born=0,geometry,phase;
 return {show(point,time=performance.now(),kind='arrival'){destination=point;born=time;phase=kind;},draw(width,height,time,focus){if(!destination)return;const age=(time-born)/1000,lifetime=phase==='target'?1.2:1.4;if(age>lifetime){destination=undefined;return;}const fade=Math.min(1,Math.max(0,(lifetime-age)/.4)),ground=terrain.height(entry.x,entry.y),position={x:(destination.x+.5)*100,y:(destination.y+.5)*100,z:(terrain.heightAt(destination.x+.5,destination.y+.5)+.02)*100,rx:0,ry:0,rz:0,scale:.6};geometry={meshes:all.map((mesh,i)=>{const ring=i-model.meshes.length;let scale=.6,rz=0,opacity=fade;
 if(ring>=0){if(phase==='target'){
  // Native terrain size is full width; our quad spans -1..1, hence half scale.
  if(ring===0)opacity=0;
  if(ring===1){const step=(age*25*.15)%2;scale=(step<1?1.8-step:0.8+step-1)*50;}
  if(ring>=2){scale=.8*(ring===3?1.2:1)*50;rz=(ring===2?-1:1)*age*25*2;if(age>.96)opacity=0;}
 }else{scale=ring===1?100*(1.3+.5*Math.cos(age*25*.15)):100*Math.max(.2,1.2-Math.max(0,age-.6));if(ring>=2)opacity=0;}}
 if(phase!=='target')scale*=.3;
 const result=transformObject(mesh,{...position,scale,rz},entry,ground,geometry?.meshes[i]);
 if(ring>=0)for(let v=0;v<result.positions.length;v+=3)result.positions[v+2]=terrain.heightAt(result.positions[v]+entry.x,result.positions[v+1]+entry.y)+.03-ground;
 return {...result,opacity};})};renderer.update(0,geometry);renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-ground);renderer.draw(width,height,time,{clear:false});},dispose(){renderer.dispose();}};
}
