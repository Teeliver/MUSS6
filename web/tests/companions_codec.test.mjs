import test from 'node:test';
import assert from 'node:assert/strict';
import {decodeCompanions} from '../client/src/protocol/appearance-codec.js';
import {core} from './helpers/game-fixture.mjs';
test('companion WASM exposes builtin, custom and unknown without inventing a model',()=>{
 const b=new Uint8Array(18);b[5]=15;b[17]=8;
 const custom=decodeCompanions(core,b);assert.equal(custom.wing.status,2);assert.equal(custom.wing.customIndex,3);assert.equal(custom.pet.status,0);
 b[17]=0;b[5]=11;b[16]=4|32;const builtin=decodeCompanions(core,b);assert.equal(builtin.wing.index,12*512+262);assert.equal(builtin.pet.index,13*512+64);
 b.fill(0);assert.equal(custom.raw[17],8);
 b[5]=12;assert.equal(decodeCompanions(core,b).wing.status,3);assert.throws(()=>decodeCompanions(core,new Uint8Array(17)));
});
