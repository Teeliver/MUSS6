import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {armorResource} from '../client/src/protocol/armor-resources.js';
import {core} from './helpers/game-fixture.mjs';
test('portable equipped armor resolver matches native in all 53760 combinations',async()=>{
 const source=await readFile(new URL('../../native/client/character_resources.cpp',import.meta.url),'utf8');
 const start=source.indexOf('namespace mu::viewer'),end=source.indexOf('CharacterResources character_resources(');assert.ok(start>0&&end>start);
 const dir=await mkdtemp(join(tmpdir(),'mu-armor-'));
 try{
  await writeFile(join(dir,'armor_reference.hpp'),'#include <string>\n#include <stdexcept>\n#include <cstdio>\n'+source.slice(start,end)+'}\n');
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_ARMOR_REFERENCE','-I',dir,'-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),fileURLToPath(new URL('./armor_resources_contract.cpp',import.meta.url)),'-o',join(dir,'test')]);
  assert.match(execFileSync(join(dir,'test'),{encoding:'utf8'}),/armor resource contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
test('actual WASM armor bridge preserves substitutions and rejects unmapped/invalid inputs',()=>{
 assert.equal(armorResource(core,0,0,4,0),'Player/MaskHelmMale01.bmd');
 assert.equal(armorResource(core,1,10,5,0),'Player/ArmorElfC01.bmd');
 assert.equal(armorResource(core,2,18,0,0),'Player/t_PantMale19.bmd');
 assert.equal(armorResource(core,0,15,2,2),'Player/HelmClass303.bmd');
 assert.throws(()=>armorResource(core,3,60,0,0));
 for(const bad of [-1,NaN,1.5,512])assert.throws(()=>armorResource(core,0,bad,0,0));
});
test('armor fixture assembles paths from WASM instead of icon resolver',async()=>{
 const {bodyPaths,assembleBody}=await import('../client/src/preview/body-geometry.js');
 const options={baseClass:5,tier:0,armorType:10};
 const paths=bodyPaths('base',{...options,core});assert.equal(paths[1],'Player/ArmorElfC01.bmd');
 let count=0;assembleBody(core,paths.map(path=>({path,bytes:new Uint8Array()})),new Uint8Array(),{...options,decode:()=>{count++;return {meshes:[]};}});assert.equal(count,5);
 assert.throws(()=>bodyPaths('helm',{...options,core}));
});
