// Explicit shared-library bindings for Raul's BMD texture names. Original files only.
// This adapter is recorded separately from source-proven per-model OpenTexture folders.
export const raulTextures={
 'flarered.ozj':{paths:['Effect/flareRed.OZJ','Item/xmas/flareRed.OZJ'],sha256:'fc7b772d1b8685b5b89989838431616d19224e321a1492cc95f9b5369b794a48'},
 'youmuk01aeff.ozj':{paths:['Item/youmuk01aeff.OZJ','Skill/youmuk01aeff.OZJ'],sha256:'ae560230ea4f268cbf3d87287825ce37629847feb2c7fcf4f58732abbbd2832c'},
 'sulnalbagiteam.ozj':{paths:['Monster/sulnalbagiteam.OZJ'],sha256:'2e449e55b5fa15135eac893c4c3c23f78f659ecc33bf0bac9ee7f6750b123dd0'},
 'xmaseffitemx.ozj':{paths:['Skill/xmaseffitemx.OZJ','Item/xmas/xmaseffitemx.OZJ'],sha256:'8956c8b8edfddaa908e2ea1fe04cd6e31760adb87bc1cd46ab3a6622e6c978be'},
 'puckitemz.ozj':{paths:['Item/puckitemz.OZJ'],sha256:'3a393144589deb3fc36166f455d697a419df31efe995fad2c58f508ab0eebdf5'},
 'sulnalbagiteampeg.ozj':{paths:['Monster/sulnalbagiteampeg.OZJ'],sha256:'cffed1898c04abdb5336c260e6e361748d431193a9030d38e1d601a3c9abc382'},
 'motion_blur_r3.ozj':{paths:['Effect/motion_blur_r3.OZJ'],sha256:'d26fa9ea3ed6f3faafc8c2b17fb2b54000e2c0b1e582a8b130fbf9cbe3991631'}
};
export async function resolveNpcSharedTexture(model,reference,{canonical,read,hash}){
 if(model.toLowerCase()!=='npc/unitedmarkedplace_raul.bmd'||!reference.toLowerCase().startsWith('npc/'))return;
 const spec=raulTextures[reference.slice(4).toLowerCase()];if(!spec)return;
 const paths=[];for(const p of spec.paths){const actual=await canonical(p);if(hash(await read(actual))!==spec.sha256)throw new Error('Shared NPC texture baseline changed: '+p);paths.push(actual);}
 return {path:paths[0],logical:reference,candidates:paths,sha256:spec.sha256,policy:'Original same-name shared asset; explicit web lookup adapter, original engine routing unverified'};
}
