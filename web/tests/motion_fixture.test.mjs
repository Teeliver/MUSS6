import test from 'node:test';
import assert from 'node:assert/strict';
import {fixtureMode} from '../client/src/preview/motion-fixture.js';
test('combat visual fixture keeps a .6s attack window and independent time-based scheduling',()=>{
 for(const [time,mode] of [[0,0],[3,1],[6,2],[9,3],[9.59,3],[9.6,0],[18.6,3]])assert.equal(fixtureMode('combat',time),mode);
 assert.equal(fixtureMode('attack',500),3);
 assert.throws(()=>fixtureMode('combat',NaN));assert.throws(()=>fixtureMode('combat',-1));
});

test('cast fixture matches native first147 then146 with .6 second windows',()=>{
 for(const [time,mode] of [[0,5],[.59,5],[.6,0],[1.9,0],[2,4],[2.59,4],[2.6,0],[4,5]])assert.equal(fixtureMode('cast',time),mode);
});
