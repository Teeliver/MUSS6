// CGMModelManager::OpenTexture falls back to FindTextureByName when local loading fails.
// Accept only unambiguous bytes; differing same-name images need native load-order evidence.
export async function resolveWorldSharedTexture(model,reference,{candidates,read,hash}){
 if(!/^(?:Object\d+|Monster|Skill)\/[^/]+\.bmd$/i.test(model)||!/^(?:Object\d+|Monster|Skill)\/[^/]+\.(OZT|OZJ)$/i.test(reference))return;
 const paths=[...new Set(await candidates(reference.split('/').at(-1)))].sort();
 if(!paths.length)return;
 let digest;for(const p of paths){const actual=hash(await read(p));if(digest&&actual!==digest)return;digest=actual;}
 return {path:paths[0],logical:reference,candidates:paths,sha256:digest,policy:'CGMModelManager::OpenTexture/FindTextureByName fallback; every same-name candidate has identical bytes'};
}
