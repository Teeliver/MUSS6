import {itemResource} from '../protocol/item-resources.js';
const fixtures=Object.freeze({
 bow:Object.freeze({baseClass:2,items:Object.freeze([{slot:0,index:4*512+15},{slot:1,index:4*512}].map(Object.freeze))}),
 'staff-shield':Object.freeze({baseClass:0,items:Object.freeze([{slot:0,index:5*512},{slot:1,index:6*512}].map(Object.freeze))}),
});
export function previewLoadout(core,query,baseClass){
 const name=query.get('loadout');if(name&&query.has('weapon'))throw new Error('Choose loadout or weapon fixture');
 let items=[];
 if(name){const fixture=fixtures[name];if(!fixture||fixture.baseClass!==baseClass)throw new Error('Loadout class mismatch: bow=2, staff-shield=0');items=fixture.items;}
 else if(query.has('weapon')){const text=query.get('weapon');if(!['0','1'].includes(text))throw new Error('Preview weapon slot must be 0 or 1');items=[{slot:Number(text),index:0}];}
 return Object.freeze(items.map(item=>{const resource=itemResource(core,item.index,0);if(!resource.mapped)throw new Error('Unmapped loadout asset');return Object.freeze({...item,path:resource.path});}));
}
