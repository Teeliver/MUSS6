import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
const root=fileURLToPath(new URL('../../',import.meta.url));
test('OZT native pixel comparison for 24/32bit and all origins',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-texture-'));
 try{
  const exe=path.join(dir,'contract');
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_TEXTURE_REFERENCE','-I',path.join(root,'client-core/include'),'-I',path.join(root,'native/client'),path.join(root,'web/tests/texture_contract.cpp'),path.join(root,'native/client/assets.cpp'),'-o',exe]);
  assert.match(execFileSync(exe,{encoding:'utf8'}),/texture contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
