import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {companionsReference} from '../tools/appearance-reference.mjs';
const root=fileURLToPath(new URL('../../',import.meta.url));
test('companions decoder matches original server packer',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-companions-'));
 try{
  await writeFile(path.join(dir,'companions_reference.hpp'),companionsReference(await readFile(path.join(root,'GameServer/DSProtocol.cpp'),'utf8')));
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_COMPANIONS_REFERENCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/companions_contract.cpp'),'-o',path.join(dir,'contract')]);
  assert.match(execFileSync(path.join(dir,'contract'),{encoding:'utf8'}),/companions contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
