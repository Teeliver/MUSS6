import test from 'node:test';
import assert from 'node:assert/strict';
import {inspectBmdModel} from '../client/src/platform/assets/decode-model.js';
import {core} from './helpers/game-fixture.mjs';
test('BMD WASM accepts direct bytes with checked errors and bounded input',()=>{
 const empty=new Uint8Array(42);empty.set([66,77,68,10]);
 assert.deepEqual(inspectBmdModel(core,empty),{meshes:[],boneParents:[],actions:0,retainedClips:[]});
 assert.throws(()=>inspectBmdModel(core,empty.subarray(0,41)));
 empty[3]=0;assert.throws(()=>inspectBmdModel(core,empty));
 assert.throws(()=>inspectBmdModel(core,new Uint8Array(16*1024*1024+1)));
 assert.throws(()=>inspectBmdModel(core,'not bytes'));
});
