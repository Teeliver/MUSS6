import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {appearanceReference} from '../tools/appearance-reference.mjs';
const root=fileURLToPath(new URL('../../',import.meta.url));
test('appearance decoder matches original server equipment packer',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-appearance-'));
 try{
  await writeFile(path.join(dir,'appearance_reference.hpp'),appearanceReference(await readFile(path.join(root,'GameServer/DSProtocol.cpp'),'utf8')));
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_APPEARANCE_REFERENCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/appearance_contract.cpp'),'-o',path.join(dir,'contract')]);
  assert.match(execFileSync(path.join(dir,'contract'),{encoding:'utf8'}),/appearance contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
