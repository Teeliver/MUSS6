import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {characterReference} from '../tools/character-reference.mjs';
const root=fileURLToPath(new URL('../../',import.meta.url));
test('EX603 character list and creation policy match server ABI',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-character-'));
 try{
  await writeFile(path.join(dir,'character_reference.hpp'),characterReference(await readFile(path.join(root,'GameServer/Protocol.h'),'utf8')));
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_CHARACTER_REFERENCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/character_protocol_contract.cpp'),'-o',path.join(dir,'contract')]);
  assert.match(execFileSync(path.join(dir,'contract'),{encoding:'utf8'}),/character protocol contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
