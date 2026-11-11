// Read-only audit of effect references explicitly found in native world_view.mm.
import {readFile,readdir,realpath} from 'node:fs/promises';
import {resolve,relative,isAbsolute} from 'node:path';
import {createHash} from 'node:crypto';
import createCore from '../.build/wasm/frame-reader.mjs';
import {decodeBmdGeometry} from '../client/src/platform/assets/decode-model.js';
const input=process.argv[2];if(!input)throw Error('Explicit Data root required');const root=await realpath(input);
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
async function asset(folder,name){
 if(!['Skill','Effect'].includes(folder)||!/^[A-Za-z0-9_-]+\.(bmd|OZJ|OZT)$/i.test(name))throw Error('Unsupported effect reference');
 const matches=(await readdir(resolve(root,folder))).filter(n=>n.toLowerCase()===name.toLowerCase());if(matches.length!==1)throw Error('Missing or ambiguous effect reference');
 const path=await realpath(resolve(root,folder,matches[0])),rel=relative(root,path);if(isAbsolute(rel)||rel.startsWith('..'))throw Error('Effect escapes Data');
 const bytes=await readFile(path);return {bytes,entry:{canonical:`${folder}/${name}`,path:`${folder}/${matches[0]}`,size:bytes.length,sha256:createHash('sha256').update(bytes).digest('hex')}};
}
const entries=[],failures=[];
for(const name of ['Fire01.bmd','Laser01.bmd']){
 try{const model=await asset('Skill',name),geometry=decodeBmdGeometry(core,model.bytes);entries.push({...model.entry,meshes:geometry.meshes.length});
 for(const mesh of geometry.meshes){if(!/^[A-Za-z0-9_-]+\.(jpg|tga)$/i.test(mesh.texture))throw Error('Unsupported mesh texture name');const target=mesh.texture.replace(/\.(jpg|tga)$/i,ext=>ext.toLowerCase()==='.jpg'?'.OZJ':'.OZT');entries.push((await asset('Skill',target)).entry);}
 }catch(error){failures.push({model:name,error:error.message});}
}
for(const name of ['JointSpirit01.OZJ','Thunder01.OZJ','Spark03.OZJ']){try{entries.push((await asset('Effect',name)).entry);}catch(error){failures.push({texture:name,error:error.message});}}
console.log(JSON.stringify({source:'native/client/world_view.mm lines 38–40',scope:'asset bytes/mesh references only; no material/effect parity',entries:[...new Map(entries.map(e=>[e.path,e])).values()],failures},null,2));
