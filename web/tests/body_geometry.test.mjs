import test from 'node:test';
import assert from 'node:assert/strict';
import {assembleBody,bodyPaths,parseArmorFixture} from '../client/src/preview/body-geometry.js';
import {core} from './helpers/game-fixture.mjs';
test('body assembly binds five named parts to the same Player pose without recentering',()=>{
 const skeleton=new Uint8Array([1]),parts=bodyPaths('base',{core}).map(path=>({path,bytes:new Uint8Array([2])}));let calls=0;
 const result=assembleBody(core,parts,skeleton,{decode:(core,bytes,rig,pose)=>{assert.equal(rig,skeleton);assert.deepEqual(pose,{action:1,frame:0});calls++;return {meshes:[{positions:Float32Array.of(calls,0,0),texture:'skin.jpg'}]};}});
 assert.equal(calls,5);assert.equal(result.meshes.length,5);assert.equal(result.meshes[4].positions[0],5);assert.equal(result.meshes[0].sourcePath,'Player/HelmClass01.bmd');
 assert.throws(()=>assembleBody(core,parts.slice(1),skeleton));assert.throws(()=>assembleBody(core,[...parts].reverse(),skeleton));assert.throws(()=>bodyPaths('unknown',{core}));
 const changed=bodyPaths('helm',{core});assert.equal(changed[0],'Player/HelmMale01.bmd');changed[1]='bad';assert.equal(bodyPaths('base',{core})[1],'Player/ArmorClass01.bmd');
});
test('mixed slots preserve class head, use core substitutions, and release equipped paths on unequip',()=>{
 const types=Object.freeze([511,10,10,10,10]),options={core,baseClass:5,tier:0,armorTypes:types};
 const mixed=bodyPaths('base',options);
 assert.deepEqual(mixed,['Player/HelmClass06.bmd','Player/ArmorElfC01.bmd','Player/PantElfC01.bmd','Player/GloveElfC01.bmd','Player/BootElfC01.bmd']);
 const naked=bodyPaths('base',{core,baseClass:5});assert.equal(naked[1],'Player/ArmorClass06.bmd');assert.equal(types[0],511);
 assert.throws(()=>bodyPaths('base',{...options,armorTypes:[511]}));
 assert.throws(()=>bodyPaths('base',{...options,armorType:10}));
 assert.throws(()=>bodyPaths('base',{...options,armorTypes:[511,512,10,10,10]}));
 const parts=mixed.map(path=>({path,bytes:new Uint8Array()}));let calls=0;
 const result=assembleBody(core,parts,new Uint8Array(),{baseClass:5,armorTypes:types,decode:()=>{calls++;return {meshes:[{}]};}});
 assert.equal(calls,5);assert.deepEqual(result.meshes.map(m=>m.sourcePath),mixed);
});
test('debug armor query accepts exact slot order and rejects incomplete or ambiguous text',()=>{
 assert.equal(parseArmorFixture(null),undefined);
 assert.deepEqual(parseArmorFixture('10'),[10,10,10,10,10]);
 assert.deepEqual(parseArmorFixture('511,10,10,10,10'),[511,10,10,10,10]);
 for(const value of ['', '1,2', '1,2,3,4,5,6','-1','512','1e1','10, 10','1.5','0x10',undefined])assert.throws(()=>parseArmorFixture(value));
});
