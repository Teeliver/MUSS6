import test from 'node:test';import assert from 'node:assert/strict';import {petPolicy} from '../client/src/world/pet-layer.js';
test('pet visual policy uses helper identity and suppresses expired pet without inventing mounts',()=>{assert.equal(petPolicy({index:6656,durability:5}).flying,true);assert.equal(petPolicy({index:13*512+80}).flying,false);assert.equal(petPolicy({index:6657,durability:5}),undefined);assert.equal(petPolicy({index:6656,durability:0}),undefined);assert.equal(petPolicy({index:13*512+4}),undefined);assert.equal(petPolicy({index:14*512}),undefined);});

test('helper pets use original Pet.bmd scales, blend and hover height',()=>{
 for(const [type,scale]of [[64,.2],[65,.4],[67,.8],[80,.8],[106,.6],[123,.4]])assert.ok(Math.abs(petPolicy({index:6656+type}).scale-scale)<1e-6);
 assert.equal(petPolicy({index:6721}).blend,0);
 assert.equal(petPolicy({index:6762}).flying,true);
 assert.ok(Math.abs(petPolicy({index:6762}).keys-20)<1e-5);
 assert.ok(Math.abs(petPolicy({index:6720}).height-2.07)<1e-6);
});

test('original Pet.bmd checksum validates before publishing pet configuration',async()=>{
 const {readFile}=await import('node:fs/promises'),{decodePetData}=await import('../client/src/protocol/pet-data.js');
 const bytes=new Uint8Array(await readFile('/Users/angelale/Downloads/MuClientEX603/Data/Local/Pet.bmd'));
 const rows=decodePetData(bytes);assert.deepEqual(rows.map(r=>r.type),[64,65,67,80,106,123]);
 const broken=bytes.slice();broken[30]^=1;assert.throws(()=>decodePetData(broken),/checksum/);
 assert.throws(()=>decodePetData(bytes.slice(0,-1)),/layout/);
});

test('Satan light follows native shoulder pose and clears when helper attachment is removed',async()=>{
 const {core}=await import('./helpers/game-fixture.mjs'),{readFile}=await import('node:fs/promises'),{bodyPaths}=await import('../client/src/preview/body-geometry.js');
 const read=async p=>new Uint8Array(await readFile('/Users/angelale/Downloads/MuClientEX603/Data/'+p)),player=await read('Player/Player.bmd'),rig=new core.BodyAnimation(player,1);
 try{for(const p of bodyPaths('base',{core,baseClass:0,tier:0}))rig.addPart(await read(p));rig.addWeapon(await read('Player/Helper02.bmd'),8,6657,true);const normal=rig.sampleResult(0);assert.equal(normal.ok,true,normal.error);assert.equal(normal.helperLight.length,3);assert.ok(normal.helperLight.every(Number.isFinite));rig.configureHelper(6,59);const rf=rig.sampleResult(0);assert.ok(rf.helperLight[2]-normal.helperLight[2]>15);rig.setWeapon(await read('Item/Sword01.bmd'),0,0,true);assert.equal(rig.sampleResult(0).helperLight,undefined);}finally{rig.delete();}
});
