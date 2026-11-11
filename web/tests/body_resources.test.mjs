import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {execFileSync} from 'node:child_process';
import {bodyPaths,assembleBody} from '../client/src/preview/body-geometry.js';
import {core} from './helpers/game-fixture.mjs';
test('all base-body class/tier paths match compiled native armor_model',async()=>{
 const source=await readFile(new URL('../../native/client/character_resources.cpp',import.meta.url),'utf8');
 const start=source.indexOf('namespace mu::viewer'),end=source.indexOf('CharacterResources character_resources(');
 assert.ok(start>0&&end>start);
 const dir=await mkdtemp(join(tmpdir(),'mu-body-reference-'));
 try{
  const cpp='#include <string>\n#include <stdexcept>\n#include <cstdio>\n#include <iostream>\n'+source.slice(start,end)+'}\nint main(){for(unsigned c=0;c<7;c++)for(unsigned t=0;t<3;t++)for(unsigned p=0;p<5;p++)std::cout<<"Player/"<<mu::viewer::armor_model(p,511,c,t)<<"\\n";}';
  await writeFile(join(dir,'ref.cpp'),cpp);
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic',join(dir,'ref.cpp'),'-o',join(dir,'ref')]);
  const expected=execFileSync(join(dir,'ref'),{encoding:'utf8'}).trim().split('\n'),actual=[];
  for(let c=0;c<7;c++)for(let t=0;t<3;t++)actual.push(...bodyPaths('base',{baseClass:c,tier:t,core}));
  assert.deepEqual(actual,expected);
 }finally{await rm(dir,{recursive:true,force:true});}
});
test('class preview uses native idle actions and rejects invalid identities',()=>{
 for(let c=0;c<7;c++){
  const parts=bodyPaths('base',{baseClass:c,core}).map(path=>({path,bytes:new Uint8Array()}));
  let calls=0;assembleBody(core,parts,new Uint8Array(),{baseClass:c,decode:(_a,_b,_c,pose)=>{assert.equal(pose.action,c===2?2:c===5?3:1);calls++;return {meshes:[]};}});assert.equal(calls,5);
 }
 for(const baseClass of [-1,7,NaN,0.5])assert.throws(()=>bodyPaths('base',{baseClass}));
 for(const tier of [-1,3,NaN,0.5])assert.throws(()=>bodyPaths('base',{tier}));
 assert.throws(()=>bodyPaths('helm',{baseClass:2}));
});
