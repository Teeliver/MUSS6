import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {core} from './helpers/game-fixture.mjs';
test('weapon actions match compiled native selection across class/item/hand/twoHand branches',async()=>{
 const source=await readFile(new URL('../../native/client/character_resources.cpp',import.meta.url),'utf8');const a=source.indexOf('std::array<unsigned,3> equipped_actions('),b=source.indexOf('CharacterResources base_character_resources(');assert.ok(a>0&&b>a);
 const renderSource=await readFile(new URL('../../native/client/character_resources.mm',import.meta.url),'utf8');const attackStart=renderSource.indexOf('if(left&&left->index/512==4'),attackEnd=renderSource.indexOf('auto skeleton=',attackStart);assert.ok(attackStart>0&&attackEnd>attackStart);
 const attackReference='unsigned attack_reference(std::optional<Item> right,std::optional<Item> left,const mu::ui::ItemCatalog& source){unsigned attack=38;auto catalog=&source;'+renderSource.slice(attackStart,attackEnd)+'return attack;}';
 const stub='#include <array>\n#include <optional>\nnamespace mu::ui {struct Attribute{bool twoHand;};struct ItemCatalog{bool two=false;Attribute at(unsigned)const{return {two};}};}\nnamespace mu::viewer {struct Item{unsigned index;};struct CharacterResources{unsigned idle,walk,run;std::array<std::optional<Item>,3> attachments;};\n'+source.slice(a,b)+attackReference+'}\n';
 const dir=await mkdtemp(join(tmpdir(),'mu-actions-'));try{await writeFile(join(dir,'actions_reference.hpp'),stub);execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_ACTIONS_REFERENCE','-I',dir,'-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),fileURLToPath(new URL('equipment_actions_contract.cpp',import.meta.url)),'-o',join(dir,'test')]);assert.match(execFileSync(join(dir,'test'),{encoding:'utf8'}),/equipment actions contract passed/);}finally{await rm(dir,{recursive:true,force:true});}
});
function table(two){const size=84*8192,b=new Uint8Array(size+4),v=new DataView(b.buffer);b[30]=two?0xFD:0xFC;let result=(0xE2F1<<9)>>>0;for(let n=0;n<size;n+=4){const value=v.getUint32(n,true);result=((n/4+0xE2F1)%2?result+value:result^value)>>>0;if(n%16===0)result=(result^(((0xE2F1+result)>>>0)>>>((n/4)%8+1)))>>>0;}v.setUint32(size,result,true);return b;}
test('actual WASM reads checksummed Item BMD twoHand and chooses weapon pose',()=>{
 const single=table(false),double=table(true);
 assert.equal(core.equipmentActionsResult(0,0,-1,single).idle,4);assert.equal(core.equipmentActionsResult(0,0,-1,double).idle,5);
 single[single.length-1]^=1;assert.equal(core.equipmentActionsResult(0,0,-1,single).ok,false);
 assert.equal(core.equipmentActionsResult(0,0,-1,new Uint8Array()).ok,false);
 assert.equal(core.equipmentActionsResult(7,-1,-1,new Uint8Array()).ok,false);
});
