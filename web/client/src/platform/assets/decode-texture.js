import {itemTextureBindings} from '../../protocol/item-model-bindings.js';
export function textureReference(modelPath,name){
 if(!/^(Item|Player|Skill|Effect|Npc|Monster|Object[1-9][0-9]{0,2})\/(?:[A-Za-z0-9_-]+\/){0,3}[A-Za-z0-9_-]+\.bmd$/.test(modelPath)||typeof name!=='string'||!(/^[A-Za-z0-9_-][A-Za-z0-9_ .-]*\.(jpg|tga)$/i.test(name)))throw new Error('Unsupported model texture reference');
 return modelPath.slice(0,modelPath.lastIndexOf('/')+1)+name.replace(/\.(jpg|tga)$/i,ext=>ext.toLowerCase()==='.jpg'?'.OZJ':'.OZT');
}
export function inspectTexture(core,format,bytes){
 if(!['OZT','OZJ'].includes(format)||!(bytes instanceof Uint8Array)||bytes.length>16*1024*1024)throw new Error('Invalid texture input');
 const result=core.textureResult(format,bytes);if(!result.ok)throw new Error(result.error);const {ok,...texture}=result;return texture;
}
export async function decodeOriginalTexture(core,format,bytes,{createBitmap=globalThis.createImageBitmap,signal}={}){
 if(signal?.aborted)throw new Error('Texture decode cancelled');
 const texture=inspectTexture(core,format,bytes);
 if(!texture.jpeg)return {kind:'rgba',width:texture.width,height:texture.height,pixels:texture.bytes,dispose(){texture.bytes.fill(0);}};
 if(typeof createBitmap!=='function')throw new Error('Browser JPEG decoder unavailable');
 // Native decode_texture(OZJ) flips rows for mesh UVs; verified against native RGBA.
 const bitmap=await createBitmap(new Blob([texture.bytes],{type:'image/jpeg'}),{premultiplyAlpha:'none',colorSpaceConversion:'none',imageOrientation:'flipY'});
 if(signal?.aborted||bitmap.width!==texture.width||bitmap.height!==texture.height){bitmap.close();throw new Error('Texture decode cancelled or dimensions mismatch');}
 let disposed=false;
 return {kind:'bitmap',width:bitmap.width,height:bitmap.height,bitmap,dispose(){if(!disposed){disposed=true;bitmap.close();}}};
}

// ZzzOpenData: MODEL_HELPER+29 loads Item/EventChaosCastle but opens textures from Npc.
export function itemTextureReference(index,modelPath,name){const path=textureReference(modelPath,name);if(index===6*512+17&&name.toLowerCase()==='flarered.jpg')return 'Effect/flareRed.OZJ';return index===13*512+29?'Npc/'+path.slice(path.lastIndexOf('/')+1):itemTextureBindings[index]?.[name.toLowerCase()]??path;}
