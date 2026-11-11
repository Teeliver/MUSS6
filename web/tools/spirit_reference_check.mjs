import fs from 'node:fs';import assert from 'node:assert/strict';import {EvilSpirits} from '../client/src/preview/evil-spirit.js';
const sims=new Map();let count=0,maxError=0;
for(const line of fs.readFileSync(process.argv[2],'utf8').trim().split('\n')){const [seed,tick,i,...values]=line.split(' ').map(Number);if(!sims.has(seed))sims.set(seed,new EvilSpirits(seed));const sim=sims.get(seed);while(sim.frame<tick)sim.step();const h=sim.heads[i],actual=[h.p.x,h.p.y,h.p.z,h.yaw,h.pitch,h.dyaw,h.dpitch];for(let j=0;j<values.length;j++){const error=Math.abs(actual[j]-values[j]);maxError=Math.max(maxError,error);assert.ok(error<1e-8,`${seed}/${tick}/${i}/${j}: ${error}`);count++;}}
assert.equal(count,4200);console.log(JSON.stringify({nativeScalarComparisons:count,maxError}));
