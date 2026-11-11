import test from 'node:test';
import assert from 'node:assert/strict';
import {itemResource} from '../client/src/protocol/item-resources.js';
import {core} from './helpers/game-fixture.mjs';
test('item resource WASM keeps native paths and exposes unmapped items',()=>{
 assert.deepEqual(itemResource(core,4*512+7,0),{mapped:true,path:'Item/Arrows01.bmd'});
 assert.deepEqual(itemResource(core,7*512+10,0),{mapped:true,path:'Player/HelmElf01.bmd'});
 assert.deepEqual(itemResource(core,8191,0),{mapped:false,path:''});
 for(const [index,level] of [[-1,0],[8192,0],[0,16],[0,1.5]])assert.throws(()=>itemResource(core,index,level));
});
