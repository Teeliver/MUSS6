import test from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { mkdtemp,readFile,writeFile,rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { modulusReference } from '../tools/modulus-reference.mjs';
test('SimpleModulus/key-file/serial native contract matches supplied Main block functions',async()=>{
 const root=fileURLToPath(new URL('../../',import.meta.url));const dir=await mkdtemp(path.join(tmpdir(),'mu-modulus-'));
 try{
  await writeFile(path.join(dir,'modulus_reference.hpp'),modulusReference(await readFile(path.join(root,'../Main_EX603/Main/PacketManager.cpp'),'utf8')));
  const binary=path.join(dir,'contract');
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_MODULUS_REFERENCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/simple_modulus_contract.cpp'),'-o',binary]);
  assert.match(execFileSync(binary,{encoding:'utf8'}),/simple-modulus contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
