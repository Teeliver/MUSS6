import test from 'node:test';
import assert from 'node:assert/strict';
import {createResourceManifest} from '../client/src/platform/assets/manifest.js';
const doc=()=>({schema:1,version:'a'.repeat(64),entries:[{canonical:'Item/Wing08.bmd',path:'Item/wing08.bmd',size:123,sha256:'b'.repeat(64)}]});
test('resource manifest resolves exact case with immutable ownership',()=>{
 const input=doc(),manifest=createResourceManifest(input,'http://127.0.0.1:8080/assets/v1/');input.entries[0].path='Item/other.bmd';
 assert.equal(manifest.resolve('Item/Wing08.bmd').url,'http://127.0.0.1:8080/assets/v1/Item/wing08.bmd');
 assert.equal(manifest.resolve('Item/Wing08.bmd').sha256,'b'.repeat(64));assert.equal(manifest.count,1);
 assert.throws(()=>manifest.resolve('Item/Unknown.bmd'));
});
test('resource manifest rejects traversal, remapping, invalid hashes and collisions',()=>{
 for(const patch of [{path:'../escape.bmd'},{path:'Item/other.bmd'},{canonical:'Item/%2e%2e.bmd'},{size:0},{size:67108865},{sha256:'invalid'}]){
  const d=doc();Object.assign(d.entries[0],patch);assert.throws(()=>createResourceManifest(d,'https://example.com/Data/'));
 }
 const duplicate=doc();duplicate.entries.push({...duplicate.entries[0]});assert.throws(()=>createResourceManifest(duplicate,'https://example.com/Data/'));
 for(const base of ['file:///Data/','https://user:pass@example.com/Data/','https://example.com/Data/?x=1','https://example.com/Data'])assert.throws(()=>createResourceManifest(doc(),base));
});
test('original lowercase texture extensions resolve without renaming Data',()=>{
 const input=doc();input.entries[0]={canonical:'Player/boots_02.OZJ',path:'Player/boots_02.ozj',size:123,sha256:'b'.repeat(64)};
 const manifest=createResourceManifest(input,'http://127.0.0.1:8080/Data/');
 assert.equal(manifest.resolve('Player/boots_02.OZJ').path,'Player/boots_02.ozj');
 for(const path of ['Player/boots_02.png','Player/../boots_02.ozj']){input.entries[0].path=path;assert.throws(()=>createResourceManifest(input,'http://127.0.0.1:8080/Data/'));}
});
test('manifest allows only the explicit original Item table outside Item/Player folders',()=>{
 const d=doc();d.entries[0]={...d.entries[0],canonical:'Local/Eng/Item_eng.bmd',path:'Local/Eng/Item_eng.bmd'};
 assert.equal(createResourceManifest(d,'http://127.0.0.1:8080/Data/').resolve('Local/Eng/Item_eng.bmd').path,'Local/Eng/Item_eng.bmd');
 for(const path of ['Local/Eng/Other.bmd','Local/Config.ini','Local/../Eng/Item_eng.bmd']){d.entries[0]={...d.entries[0],canonical:path,path};assert.throws(()=>createResourceManifest(d,'http://127.0.0.1:8080/Data/'));}
});

test('Skill effect assets stay confined to one filename under the original folder',()=>{
 const input=doc();input.entries[0]={...input.entries[0],canonical:'Skill/Fire01.bmd',path:'Skill/Fire01.bmd'};
 assert.equal(createResourceManifest(input,'http://127.0.0.1:8080/Data/').resolve('Skill/Fire01.bmd').path,'Skill/Fire01.bmd');
 input.entries[0].path='Skill/../Fire01.bmd';assert.throws(()=>createResourceManifest(input,'http://127.0.0.1:8080/Data/'));
});
