import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {core} from './helpers/game-fixture.mjs';
import {equipmentGrip} from '../client/src/protocol/equipment-grip.js';
test('portable grip transforms match original native simd matrices in 49152 cases',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'mu-grip-'));
 try{execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_GRIP_REFERENCE','-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),'-I',fileURLToPath(new URL('../../native/client',import.meta.url)),fileURLToPath(new URL('equipment_grip_contract.cpp',import.meta.url)),'-o',join(dir,'ref')]);assert.match(execFileSync(join(dir,'ref'),{encoding:'utf8'}),/49152 cases/);}finally{await rm(dir,{recursive:true,force:true});}
});
test('actual WASM grip preserves hands, arrows/back, robe, and immutable matrices',()=>{
 assert.equal(equipmentGrip(core,0,0,false).bone,33);assert.equal(equipmentGrip(core,1,0,false).bone,42);
 for(const safe of [false,true])assert.equal(equipmentGrip(core,0,4*512+7,safe).bone,47);
 assert.deepEqual(equipmentGrip(core,1,4*512,true).transform.slice(12,15),[-10,5,10]);
 assert.equal(equipmentGrip(core,7,12*512+40,false).bone,19);
 assert.throws(()=>{equipmentGrip(core,0,0,false).transform[0]=9;});
 for(const input of [[2,0,false],[0,-1,false],[0,8192,false],[0,0,1]])assert.throws(()=>equipmentGrip(core,...input));
});
