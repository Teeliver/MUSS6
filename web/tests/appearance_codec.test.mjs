import test from 'node:test';
import assert from 'node:assert/strict';
import {decodeAppearance} from '../client/src/protocol/appearance-codec.js';
import {core} from './helpers/game-fixture.mjs';
test('appearance WASM preserves equipment slots, visual grade and independent raw bytes',()=>{
 const b=new Uint8Array(18);b[0]=(2<<5)|16;b[1]=1;b[12]=0x20;b[6]=0;b[7]=0;b[8]=7;b[10]=4;b[11]=4;
 const a=decodeAppearance(core,b);assert.equal(a.baseClass,2);assert.equal(a.tier,1);
 assert.deepEqual(a.parts[0],{slot:0,index:513,visualLevel:15,present:true,excellent:true,ancient:true});
 assert.equal(a.parts[2].index,7*512);b.fill(0);assert.equal(a.raw[0],80);
 assert.throws(()=>decodeAppearance(core,new Uint8Array(17)));
 const absent=new Uint8Array(18).fill(255);absent[0]=0;const empty=decodeAppearance(core,absent);assert.ok(empty.parts.every(p=>!p.present&&!p.excellent&&!p.ancient));
});
