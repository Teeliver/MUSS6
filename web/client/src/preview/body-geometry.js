import {armorResource} from '../protocol/armor-resources.js';
import {decodeBoundGeometry} from '../platform/assets/decode-model.js';
// Slot order matches native armor[]: Helm, Armor, Pant, Glove, Boot.
export function parseArmorFixture(value){
 if(value===null)return undefined;
 if(typeof value!=='string'||!/^\d{1,3}(,\d{1,3}){0,4}$/.test(value))throw new Error('Armor fixture requires one type or five slot types');
 const types=value.split(',').map(Number);
 if(![1,5].includes(types.length)||types.some(v=>v>511))throw new Error('Armor fixture requires types 0..511');
 return types.length===1?Array(5).fill(types[0]):types;
}
export function bodyPaths(outfit='base',{baseClass=0,tier=0,core,armorType,armorTypes}={}){
 if(!Number.isInteger(baseClass)||baseClass<0||baseClass>6||!Number.isInteger(tier)||tier<0||tier>2)throw new Error('Invalid body class/tier');
 if(!['base','helm'].includes(outfit))throw new Error('Unsupported preview outfit');
 if(armorType!==undefined&&armorTypes!==undefined)throw new Error('Ambiguous armor fixture');
 const equipped=armorTypes!==undefined||armorType!==undefined;
 if(equipped&&outfit!=='base')throw new Error('Cannot combine armor fixture with outfit');
 let types=armorTypes===undefined?Array(5).fill(armorType??511):armorTypes;
 if(!Array.isArray(types)||types.length!==5)throw new Error('Five armor slot types required');
 types=[...types];
 if(outfit==='helm'){
  if(baseClass!==0||tier!==0)throw new Error('Test helmet only verified for base DW');
  types[0]=0;
 }
 // One source of model rules for base, transformed classes and equipped armor.
 return types.map((type,part)=>armorResource(core,part,type,baseClass,tier));
}
export function assembleBody(core,parts,skeleton,{outfit='base',baseClass=0,tier=0,armorType,armorTypes,action,decode=decodeBoundGeometry}={}){
 const paths=bodyPaths(outfit,{baseClass,tier,core,armorType,armorTypes});
 if(parts.length!==5||parts.some((p,i)=>p.path!==paths[i]))throw new Error('Incomplete or reordered body parts');
 const meshes=[];
 for(const part of parts){const bound=decode(core,part.bytes,skeleton,{action:action??(baseClass===2?2:baseClass===5?3:1),frame:0});for(const mesh of bound.meshes)meshes.push({...mesh,sourcePath:part.path});}
 return {meshes}; // Keep the shared Player coordinate system; never center each body part.
}
