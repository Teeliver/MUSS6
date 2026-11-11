import test from 'node:test';
import assert from 'node:assert/strict';
import {fixture} from './helpers/bmd-fixture.mjs';
import {decodeBoundGeometry} from '../client/src/platform/assets/decode-model.js';
import {core} from './helpers/game-fixture.mjs';
test('Player binding validates pose and bone identity before exporting buffers',()=>{
 const equipment=fixture(),skeleton=fixture();const geometry=decodeBoundGeometry(core,equipment,skeleton,{action:0});assert.equal(geometry.meshes[0].positions[0],2);
 const mismatch=fixture();mismatch[mismatch.length-58]=88;assert.throws(()=>decodeBoundGeometry(core,equipment,mismatch,{action:0}),/binding mismatch/);
 assert.throws(()=>decodeBoundGeometry(core,equipment,skeleton,{action:1}));assert.throws(()=>decodeBoundGeometry(core,equipment,skeleton,{action:0,frame:1}));assert.throws(()=>decodeBoundGeometry(core,equipment,skeleton,{action:-1}));
});
