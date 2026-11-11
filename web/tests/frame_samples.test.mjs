import test from 'node:test';
import assert from 'node:assert/strict';
import {frameSamples} from '../client/src/diagnostics/frame-samples.js';
test('frame ring retains bounded recent samples across wrap, sorts numerically and resets',()=>{
 const samples=frameSamples(3);assert.equal(samples.percentile(.95),undefined);
 for(const n of [100,2,30,4])samples.push(n);
 assert.equal(samples.length,3);assert.equal(samples.percentile(1),30);assert.equal(samples.percentile(.5),4);
 assert.equal(samples.percentile(1),30);samples.clear();samples.push(1);assert.equal(samples.length,1);assert.equal(samples.percentile(.95),1);
 assert.throws(()=>samples.push(NaN));assert.throws(()=>samples.percentile(0));assert.throws(()=>frameSamples(0));
});
