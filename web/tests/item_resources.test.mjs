import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {itemResourceReference} from '../tools/item-resource-reference.mjs';
const root=fileURLToPath(new URL('../../',import.meta.url));
test('portable item paths match existing native adapter',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-resource-'));
 try{
  await writeFile(path.join(dir,'item_resource_reference.hpp'),itemResourceReference(await readFile(path.join(root,'native/client/item_models.cpp'),'utf8')));
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_RESOURCE_REFERENCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/item_resources_contract.cpp'),'-o',path.join(dir,'contract')]);
  assert.match(execFileSync(path.join(dir,'contract'),{encoding:'utf8'}),/item resource contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
