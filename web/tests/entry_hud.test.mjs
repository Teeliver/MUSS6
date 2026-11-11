import test from 'node:test';
import assert from 'node:assert/strict';
import {createEntryHud} from '../client/src/world/entry-hud.js';
test('entry HUD preserves 32-bit server values and clears on disconnect',()=>{
 const elements=[];const document={createElement(){const element={textContent:'',setAttribute(){},append(){},remove(){this.removed=true;}};elements.push(element);return element;},body:{append(){}}};
 const hud=createEntryHud(document),[panel,output]=elements;
 assert.equal(panel.hidden,true);
 hud.show({name:'ElfTest',level:400},{map:3,x:181,y:96,hp:70000,maxHp:80000,money:4294967295});
 assert.equal(panel.hidden,false);
 assert.equal(output.textContent,'ElfTest · Lv.400 | Map 3 (181, 96) | HP 70000/80000 | Zen 4294967295');
 hud.clear();assert.equal(panel.hidden,true);assert.equal(output.textContent,'');
 hud.show({name:'Next',level:1},{map:0,x:1,y:2,hp:10,maxHp:10,money:0});assert.ok(!output.textContent.includes('ElfTest'));
 hud.dispose();assert.equal(panel.removed,true);
});
