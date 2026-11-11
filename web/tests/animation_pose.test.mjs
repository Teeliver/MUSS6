import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {core} from './helpers/game-fixture.mjs';
test('animation sampler shortest rotation, clock, loop and locked terminal frame',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'mu-animation-'));try{execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),fileURLToPath(new URL('animation_pose_contract.cpp',import.meta.url)),'-o',join(dir,'test')]);assert.match(execFileSync(join(dir,'test'),{encoding:'utf8'}),/animation pose contract passed/);}finally{await rm(dir,{recursive:true,force:true});}
});
test('WASM animated body rejects incomplete source models',()=>{assert.equal(typeof core.BodyAnimation,'function');assert.throws(()=>new core.BodyAnimation(new Uint8Array(),1));});
