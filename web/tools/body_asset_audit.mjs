// Audit only source-derived base-body paths; never enumerate or publish all Data.
import {readFile} from 'node:fs/promises';
import {resolve} from 'node:path';
import createCore from '../.build/wasm/frame-reader.mjs';
import {bodyPaths,assembleBody} from '../client/src/preview/body-geometry.js';
import {previewLoadout} from '../client/src/preview/loadouts.js';
import {decodeAttachmentGeometry} from '../client/src/platform/assets/decode-model.js';
import {textureReference} from '../client/src/platform/assets/decode-texture.js';
const root=process.argv[2];if(!root)throw Error('Data directory required');
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const skeleton=await readFile(resolve(root,'Player/Player.bmd')),paths=new Set(),entries=[];
const cases=[];for(let baseClass=0;baseClass<7;baseClass++)for(let tier=0;tier<3;tier++)cases.push({baseClass,tier});
// Bounded render fixtures; these do not validate equip requirements.
for(const [baseClass,armorType] of [[0,0],[2,10],[5,10],[4,0],[6,5]])cases.push({baseClass,tier:0,armorType});
for(const baseClass of [0,2,5,4,6]){const type=baseClass===2||baseClass===5?10:baseClass===6?5:0;cases.push({baseClass,tier:0,armorTypes:[511,type,type,type,type]});}
for(const {baseClass,tier,armorType,armorTypes} of cases){
 try{
  const names=bodyPaths('base',{baseClass,tier,core,armorType,armorTypes}),parts=[];
  for(const path of names)parts.push({path,bytes:await readFile(resolve(root,path))});
  const body=assembleBody(core,parts,skeleton,{baseClass,tier,armorType,armorTypes});
  const textures=[...new Set(body.meshes.map(m=>textureReference(m.sourcePath,m.texture)))];
  for(const path of textures)await readFile(resolve(root,path));
  for(const path of [...names,...textures])paths.add(path);
  entries.push({baseClass,tier,armorType,armorTypes,status:'verified',models:names,textures,meshes:body.meshes.length});
 }catch(error){entries.push({baseClass,tier,armorType,armorTypes,status:'unavailable',error:error.message});}
}
const itemTable=await readFile(resolve(root,'Local/Eng/Item_eng.bmd'));
for(const [baseClass,loadout] of [[2,'bow'],[0,'staff-shield']]){
 const attachments=previewLoadout(core,new URLSearchParams({loadout}),baseClass),right=attachments.find(i=>i.slot===0)?.index??-1,left=attachments.find(i=>i.slot===1)?.index??-1;
 const stance=core.equipmentActionsResult(baseClass,right,left,itemTable);if(!stance.ok)throw Error(stance.error);
 const session=new core.BodyAnimation(skeleton,stance.idle);
 try{
  for(const path of bodyPaths('base',{core,baseClass}))session.addPart(await readFile(resolve(root,path)));
  for(const item of attachments){const bytes=await readFile(resolve(root,item.path));const geometry=decodeAttachmentGeometry(core,bytes,skeleton,{action:stance.idle,slot:item.slot,index:item.index});
   paths.add(item.path);for(const mesh of geometry.meshes){const texture=textureReference(item.path,mesh.texture);await readFile(resolve(root,texture));paths.add(texture);}
   session.addWeapon(bytes,item.slot,item.index,false);
   let duplicateRejected=false;try{session.addWeapon(bytes,item.slot,item.index,false);}catch{duplicateRejected=true;}if(!duplicateRejected)throw Error('Duplicate attachment slot accepted');
  }
  session.configureSpell(skeleton,stance.walk,stance.run,baseClass===2?2:1,stance.attack);
  for(const [time,mode] of [[0,0],[.2,1],[.4,2],[.6,3],[.8,5],[1.0,4],[1.2,0]]){const sample=session.sampleMotionResult(time,mode);if(!sample.ok)throw Error(sample.error);}
  entries.push({baseClass,loadout,status:'verified',attachments,actions:{idle:stance.idle,walk:stance.walk,run:stance.run,attack:stance.attack},twoHand:stance.twoHand});
 }finally{session.delete();}
}
console.log(JSON.stringify({paths:[...paths].sort(),entries},null,2));
