import test from 'node:test';
import assert from 'node:assert/strict';
import {energyProjectile} from '../client/src/preview/energy-projectile.js';
import {PREVIEW_ANGLE} from '../client/src/platform/rendering/model-preview.js';
test('Energy follows native size, speed, height and faces preview camera; buffers reused',()=>{
 const g=energyProjectile(.1),m=g.meshes[0],c=Math.cos(PREVIEW_ANGLE),s=Math.sin(PREVIEW_ANGLE);
 const cameraZ=[];for(let i=0;i<6;i++){const x=m.positions[i*3]-1.5/.009,y=m.positions[i*3+1];cameraZ.push(-s*x-c*y);}
 assert.ok(cameraZ.every(z=>Math.abs(z)<.00002));assert.equal(m.additive,true);assert.equal(m.sourcePath,'Effect/Thunder01.OZJ');
 assert.ok(Math.abs(m.positions[8]-1.3/.009)<.00002);const p=m.positions;assert.equal(energyProjectile(.2,g),g);assert.equal(g.meshes[0].positions,p);assert.ok(p.every(n=>n===0));assert.throws(()=>energyProjectile(-1));
});
