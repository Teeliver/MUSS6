import {createModelPreview} from '../platform/rendering/model-preview.js';
import {mapSpriteEmitters,mapSpriteGeometry,mapSpriteLimit,mapSpriteTexture} from './map-sprite-policy.js';
export async function createMapSprites({canvas,texture,entry,terrain,objects}){
 const all=mapSpriteEmitters(entry.map,objects),emitters=all.slice(0,mapSpriteLimit);let renderer,disposed=false;
 if(!emitters.length)return {draw(){},dispose(){}};
 try{const image=await texture(mapSpriteTexture),ground=terrain.height(entry.x,entry.y),geometry=mapSpriteGeometry(emitters,image,0,entry,ground),uv=new Float32Array(emitters.length*12);for(let i=0;i<emitters.length;i++)uv.set([0,0,1,0,1,1,0,0,1,1,0,1],i*12);
 const mesh={...geometry,image,uv,normals:new Float32Array(geometry.positions.length).fill(1),additive:true,clampUV:true,castsShadow:false};renderer=createModelPreview(canvas.getContext('webgl2'),[{meshes:[mesh],bounds:{center:[0,0,0],scale:1}}],{world:true});
 return {draw(w,h,time,focus){if(disposed)return;mapSpriteGeometry(emitters,image,time,entry,ground,mesh);renderer.update(0,{meshes:[mesh]});renderer.setCamera(focus.x-entry.x,focus.y-entry.y,terrain.heightAt(focus.x+.5,focus.y+.5)-ground);renderer.draw(w,h,time,{clear:false,pass:'objects'});canvas.dataset.nativeMapSprites=String(emitters.length);canvas.dataset.nativeMapSpritesDropped=String(all.length-emitters.length);},dispose(){if(disposed)return;disposed=true;renderer.dispose();delete canvas.dataset.nativeMapSprites;delete canvas.dataset.nativeMapSpritesDropped;}};
 }catch(error){renderer?.dispose();throw Error('Map '+entry.map+' sprite '+mapSpriteTexture+': '+error.message);}
}
