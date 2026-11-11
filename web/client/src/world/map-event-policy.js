import {objectAnimationRate} from './object-motion-policy.js';
export const mapEffectLimit=1000;
// RenderObjectVisual samples WorldTime, not an invented periodic catch-up burst.
export function flameBurstCount(time,random=Math.random){const remainder=Math.trunc(time)%200;return remainder>=0&&remainder<=1?4+Math.floor(random()*4):0;}
export function createMapEventClock(emitters){const effects=[];let dropped=0,created=0;return {
 visual(time,focus,random=Math.random){for(const e of emitters)if(e.type===85&&Math.abs(e.x*.01-focus.x)<24&&Math.abs(e.y*.01-focus.y)<24){const count=flameBurstCount(time,random);for(let i=0;i<count;i++){if(effects.length<mapEffectLimit)(effects.push({emitter:e,life:40}),created++);else dropped++;}}},
 tick(random=Math.random){const out=[];for(let i=effects.length-1;i>=0;i--){const effect=effects[i],e=effect.emitter;out.push({...e,x:e.x+Math.floor(random()*32)-16,y:e.y+Math.floor(random()*32)-16});if(--effect.life<=0)effects.splice(i,1);}return out;},
 get created(){return created;},get size(){return effects.length;},get dropped(){return dropped;},dispose(){effects.length=0;}
};}
// Match the same action0 period and clock used to draw the environment rig.
export function machineCloudEmission(emitter,time,period){if(!Number.isInteger(period)||period<=0)return [];const frame=(time/1000*objectAnimationRate(emitter.map,64,emitter.scale))%period;if(!(frame>9.5&&frame<11.5||frame>23.5&&frame<25.5))return [];const angle=(emitter.rz+90)*Math.PI/180;return Array.from({length:3},()=>({...emitter,x:emitter.x-Math.sin(angle)*30,y:emitter.y+Math.cos(angle)*30}));}
