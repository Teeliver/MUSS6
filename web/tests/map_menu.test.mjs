import test from 'node:test';import assert from 'node:assert/strict';
import {createMapMenu} from '../client/src/world/map-menu.js';
test('M menu separates offline links from server move indexes, gates unavailable maps and restores canvas focus',async()=>{
 const previous=globalThis.fetch;globalThis.fetch=async()=>({ok:true,json:async()=>({worlds:[{map:0,world:1,names:['Town'],terrain:'loadable',issues:[]},{map:42,world:43,names:[],terrain:'blocked',issues:['missing texture']}],moves:[{id:17,name:'Town',map:0,minLevel:10,money:1000,gate:2},{id:18,name:'Blocked',map:42,minLevel:100,money:2000,gate:3}]})});
 const nodes=[],canvas={focus(){this.focused=true;}},document={createElement(tag){const node={tag,children:[],handlers:{},setAttribute(k,v){this[k]=v;},addEventListener(name,fn){this.handlers[name]=fn;},append(...items){this.children.push(...items);},focus(){},remove(){this.removed=true;}};nodes.push(node);return node;},querySelector:()=>canvas,body:{append(){}}};let key,world,resolve,sent=[];const target={addEventListener(_,fn){key=fn;},removeEventListener(){key=undefined;}};
 try{const dispose=await createMapMenu({document,target,getWorld:()=>world}),panel=nodes.find(n=>n.id==='map-menu'),button=nodes.find(n=>n['aria-label']?.startsWith('Town ·')),blocked=nodes.find(n=>n['aria-label']?.startsWith('Blocked ·'));
 key({key:'m',preventDefault(){}});assert.equal(panel.hidden,false);assert.equal(button.disabled,true);key({key:'Escape',preventDefault(){}});assert.equal(panel.hidden,true);assert.equal(canvas.focused,true);
 world={position:{map:0},travel:id=>{sent.push(id);return new Promise(r=>resolve=r);}};key({key:'M',preventDefault(){}});assert.equal(button.disabled,false);assert.equal(blocked.disabled,true);
 const pending=button.handlers.click();assert.deepEqual(sent,[17]);assert.equal(button.disabled,true);assert.ok(nodes.find(n=>n.tag==='a'&&n.href==='?scene=terrain-check&map=42'&&n.target==='_blank'));resolve({map:0});await pending;assert.equal(panel.hidden,true);
 key({key:'m',target:{closest:()=>true},preventDefault(){throw Error('Typing must not toggle');}});assert.equal(panel.hidden,true);dispose();assert.equal(key,undefined);assert.equal(panel.removed,true);
 }finally{globalThis.fetch=previous;}
});
