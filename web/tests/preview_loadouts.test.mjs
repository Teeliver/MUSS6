import test from 'node:test';
import assert from 'node:assert/strict';
import {previewLoadout} from '../client/src/preview/loadouts.js';
import {core} from './helpers/game-fixture.mjs';
test('loadouts resolve original item paths and preserve MU hand assignments',()=>{
 const bow=previewLoadout(core,new URLSearchParams('loadout=bow'),2);
 assert.deepEqual(bow.map(i=>[i.slot,i.index,i.path]),[[0,2063,'Item/Arrows02.bmd'],[1,2048,'Item/Bow01.bmd']]);
 const staff=previewLoadout(core,new URLSearchParams('loadout=staff-shield'),0);
 assert.deepEqual(staff.map(i=>[i.slot,i.index,i.path]),[[0,2560,'Item/Staff01.bmd'],[1,3072,'Item/Shield01.bmd']]);
 assert.throws(()=>{staff[0].index=0;});assert.equal(previewLoadout(core,new URLSearchParams(),0).length,0);
 for(const [query,cls] of [['loadout=bow',0],['loadout=unknown',0],['weapon=',0],['loadout=bow&weapon=0',2]])assert.throws(()=>previewLoadout(core,new URLSearchParams(query),cls));
});
