import test from 'node:test';import assert from 'node:assert/strict';
import {skillCatalog} from '../client/src/ui/skill-catalog.js';
import {castRange} from '../client/src/ui/skill-range.js';
test('skill catalog keeps PC client names/range alongside server rules',()=>{
 const byId=id=>skillCatalog.skills.find(s=>s.id===id);
 assert.equal(byId(62).name,'Earthquake');assert.equal(byId(62).clientName,'Earthshake');
 assert.equal(byId(10).name,'Hell Fire');assert.equal(byId(10).clientName,'Hellfire');assert.equal(byId(48).name,'Sweel Life');assert.equal(byId(48).clientName,'Swell Life');
 assert.equal(byId(6).clientRange,6);assert.equal(byId(65).clientRange,10);
 assert.equal(new Set(skillCatalog.skills.map(s=>s.id)).size,skillCatalog.skills.length);
});
test('cast range prefers server range/radius, then PC client range',()=>{
 assert.equal(castRange({range:6,radius:0,clientRange:2}),6);
 assert.equal(castRange({range:0,radius:5,clientRange:2}),5);
 assert.equal(castRange({range:0,radius:0,clientRange:6}),6);
 assert.equal(castRange({range:0,radius:0,clientRange:undefined}),3);
});
