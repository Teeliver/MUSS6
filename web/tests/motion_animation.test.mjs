import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
test('motion rates, .12s blend, continuous gait phase and 60/120Hz clocks',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'mu-motion-'));try{execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),fileURLToPath(new URL('motion_animation_contract.cpp',import.meta.url)),'-o',join(dir,'test')]);assert.match(execFileSync(join(dir,'test'),{encoding:'utf8'}),/motion animation contract passed/);}finally{await rm(dir,{recursive:true,force:true});}
});
