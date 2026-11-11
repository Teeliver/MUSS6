import assert from 'node:assert/strict';
import {upgradeStyle} from '../../client/src/ui/item-visual-policy.js';
import {createHelperController} from '../../client/src/world/helper-controller.js';
const normal=upgradeStyle({index:3585,level:9,excellent:0}),exc=upgradeStyle({index:3585,level:9,excellent:1});console.log('Excellent and normal material identical:',JSON.stringify(normal)===JSON.stringify(exc));
const rejected=[];process.on('unhandledRejection',e=>rejected.push(e.message));
const w={position:{map:0,x:100,y:100,hp:1},gameState:{helper:{running:true,config:{potion:true,potionPercent:50}},vitals:{hp:1,maxHp:100},items:[{slot:12,index:7168,durability:1}]},useItem:()=>Promise.reject(Error('audit busy potion rejection'))};
const controller=createHelperController({getWorld:()=>w,command:()=>false,catalog:[],now:()=>10000});controller.tick();await new Promise(r=>setTimeout(r,20));console.log('Uncaught helper promise rejection:',JSON.stringify(rejected));
