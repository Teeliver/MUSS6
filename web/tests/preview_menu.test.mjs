import test from 'node:test';
import assert from 'node:assert/strict';
import {skillRows,skillsFor,selectionUrl} from '../client/src/preview/menu.js';
const rows=()=>skillRows({schema:1,skills:[{index:4,name:'Fire Ball',requireClass:[1,0,0,1,0,1,0]},{index:19,name:'Falling Slash',requireClass:[0,1,0,1,1,0,1]},{index:40,name:'Advanced',requireClass:[3,0,0,0,0,0,0]}]});
test('preview skill menu filters server class/changeup requirements',()=>{
 const data=rows();assert.deepEqual(skillsFor(data,0,0).map(s=>s.index),[4]);assert.deepEqual(skillsFor(data,0,2).map(s=>s.index),[4,40]);assert.deepEqual(skillsFor(data,1,0).map(s=>s.index),[19]);
 assert.throws(()=>skillRows({schema:1,skills:[...data,data[0]]}));assert.throws(()=>skillsFor(data,7,0));
});
test('class change drops stale gear and unsupported skill never borrows Fire effect',()=>{
 const current='http://127.0.0.1:8080/debug/?preview=models&class=0&loadout=staff-shield&effect=fire&effect-time=0.12';
 const next=selectionUrl(current,{cls:1,tier:0,skill:19},rows());assert.equal(next.searchParams.get('skill'),'19');assert.equal(next.searchParams.get('motion'),'idle');assert.equal(next.searchParams.has('effect'),false);assert.equal(next.searchParams.has('loadout'),false);
 assert.throws(()=>selectionUrl(current,{cls:1,tier:0,skill:4},rows()));
 const fire=selectionUrl(current,{cls:0,tier:0,skill:4,freeze:true},rows());assert.equal(fire.searchParams.get('effect'),'fire');assert.equal(fire.searchParams.get('effect-time'),'0.12');
 const model=selectionUrl(current,{cls:2,tier:0,skill:null},rows());assert.equal(model.searchParams.has('skill'),false);assert.equal(model.searchParams.has('effect'),false);
});

test('Energy Ball menu selects own effect and cast, not Fire01',()=>{
 const rows=[{index:17,name:'Energy Ball',requireClass:[1,0,0,1,0,1,0]}];
 const url=selectionUrl('http://localhost/debug/',{cls:0,tier:0,skill:17,freeze:true},rows);
 assert.equal(url.searchParams.get('effect'),'energy');assert.equal(url.searchParams.get('motion'),'cast');assert.equal(url.searchParams.get('effect-time'),'0.12');
 assert.throws(()=>selectionUrl(url,{cls:2,tier:0,skill:17},rows));
});
