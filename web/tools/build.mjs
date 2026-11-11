import {constants as fsConstants} from 'node:fs';
import { createHash } from 'node:crypto';
import { mkdir, readFile, writeFile, rm, cp, mkdtemp, rename } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { modes, wasmFiles, outputPath } from './build-layout.mjs';
const root = fileURLToPath(new URL('../', import.meta.url));
if (process.version !== 'v26.11.1') throw new Error('Use pinned Node 26.11.1');
const mode=process.argv[2] || 'debug';
if(!Object.hasOwn(modes,mode))throw new Error('Expected debug or release');
const clientFiles=modes[mode];
const dist=path.join(root,'dist',mode);
// Validate all inputs before replacing generated output.
const sources = await Promise.all([
  ...clientFiles.map(async file => [outputPath(file), await readFile(path.join(root,'client',file))]),
  ...wasmFiles.map(async file => [`generated/${file}`, await readFile(path.join(root,'.build/wasm',file))]),
]);
for(const [file,bytes] of sources)if(file.endsWith('.css')&&/^\s*(?:import\s+.*\s+from\s*['\"]|export\s+(?:function|const|class))/m.test(bytes.toString()))throw Error('JavaScript found in stylesheet: '+file);
// Validate the literal JS graph reachable from HTML entrypoints before publication.
// Unused packaged libraries are not necessarily part of a mode's runtime graph.
const packaged=new Map(sources),visited=new Set();
function validateModule(file){if(visited.has(file))return;visited.add(file);for(const match of packaged.get(file).toString().matchAll(/(?:\bfrom\s*|\bimport\s*(?:\(\s*)?)(['"])(\.{1,2}\/[^'"]+)\1/g)){
 const reference=match[2].split(/[?#]/)[0];if(!/\.m?js$/.test(reference))continue;
 const target=path.posix.normalize(path.posix.join(path.posix.dirname(file),reference));if(!packaged.has(target))throw Error('Unpackaged module dependency: '+file+' -> '+reference);validateModule(target);
}}
for(const [file,bytes] of sources)if(file.endsWith('.html'))for(const match of bytes.toString().matchAll(/<script\b[^>]*\bsrc=['"]([^'"]+)['"]/g)){
 const reference=match[1].split(/[?#]/)[0];if(reference.startsWith('./')&&/\.m?js$/.test(reference)){const target=path.posix.normalize(path.posix.join(path.posix.dirname(file),reference));if(!packaged.has(target))throw Error('Unpackaged script entrypoint: '+target);validateModule(target);}
}
const revision=createHash('sha256');for(const [file,bytes] of sources)revision.update(file).update(bytes);const version=revision.digest('hex').slice(0,16);
// Keep locally staged resources/profile when rebuilding code. Publish only complete output.
await mkdir(path.dirname(dist),{recursive:true});
const staged=await mkdtemp(dist+'-staging-');
for(const name of ['runtime','preview-assets','audio-assets','ui-assets','map-title-assets']){
 try{await cp(path.join(dist,name),path.join(staged,name),{recursive:true,mode:fsConstants.COPYFILE_FICLONE});}catch(error){if(error.code!=='ENOENT')throw error;}
}
for (const [file,bytes] of sources) {
 const target = path.join(staged,file);
 await mkdir(path.dirname(target),{recursive:true});
 let output=bytes;
 if(/\.m?js$/.test(file))output=bytes.toString().replace(/(['"])(\.{1,2}\/[^'"?]+\.m?js)\1/g,(_,q,url)=>q+url+'?v='+version+q);
 if(file.endsWith('.html'))output=bytes.toString().replace(/((?:src|href)=["'])(\.\/[^"'?]+\.(?:css|js))(["'])/g,'$1$2?v='+version+'$3');
 await writeFile(target,output);
}
const previous=staged+'-previous';
let hadPrevious=false;
try{await rename(dist,previous);hadPrevious=true;}catch(error){if(error.code!=='ENOENT')throw error;}
try{await rename(staged,dist);}catch(error){if(hadPrevious)await rename(previous,dist);throw error;}
if(hadPrevious)await rm(previous,{recursive:true,force:true});
console.log(`Built ${sources.length} allowlisted files into web/dist/${mode}.`);
