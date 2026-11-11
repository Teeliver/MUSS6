import {worldCamera} from './camera.js';
function inverse(m){const a=Array.from({length:4},(_,r)=>[...Array.from({length:4},(_,c)=>m[c*4+r]),...Array.from({length:4},(_,c)=>+(r===c))]);for(let c=0;c<4;c++){let p=c;for(let r=c+1;r<4;r++)if(Math.abs(a[r][c])>Math.abs(a[p][c]))p=r;if(Math.abs(a[p][c])<1e-12)return null;[a[p],a[c]]=[a[c],a[p]];const n=a[c][c];a[c]=a[c].map(v=>v/n);for(let r=0;r<4;r++)if(r!==c){const f=a[r][c];a[r]=a[r].map((v,k)=>v-f*a[c][k]);}}return a.map(r=>r.slice(4));}
export function projectWorld(point,{width,height,entry,focus,terrain}){const ground=terrain.height(entry.x,entry.y),m=worldCamera(width/height,terrain.heightAt(focus.x+.5,focus.y+.5)-ground,focus.x-entry.x,focus.y-entry.y),p=[point.x-entry.x,(point.z??terrain.heightAt(point.x,point.y))-ground,-(point.y-entry.y),1],out=Array.from({length:4},(_,r)=>p.reduce((n,v,c)=>n+m[c*4+r]*v,0));if(out[3]<=0)return null;return {x:(out[0]/out[3]+1)*width/2,y:(1-out[1]/out[3])*height/2,depth:out[2]/out[3]};}
export function worldRay(px,py,{width,height,entry,focus,terrain}){
 const ground=terrain.height(entry.x,entry.y),inv=inverse(worldCamera(width/height,terrain.heightAt(focus.x+.5,focus.y+.5)-ground,focus.x-entry.x,focus.y-entry.y));if(!inv)return null;
 const unproject=z=>{const p=[px/width*2-1,1-py/height*2,z,1],v=inv.map(row=>row.reduce((n,k,i)=>n+k*p[i],0));return {x:v[0]/v[3]+entry.x,y:-v[2]/v[3]+entry.y,z:v[1]/v[3]+ground};},a=unproject(-1),b=unproject(1),length=Math.hypot(b.x-a.x,b.y-a.y,b.z-a.z),d={x:(b.x-a.x)/length,y:(b.y-a.y)/length,z:(b.z-a.z)/length};return {origin:a,direction:d,length};
}
export function pickTerrain(px,py,context){
 const {terrain}=context,ray=worldRay(px,py,context);if(!ray)return null;const {origin:a,direction:d,length}=ray;let previous;
 const point=t=>({x:a.x+d.x*t,y:a.y+d.y*t,z:a.z+d.z*t});
 for(let t=0;t<Math.min(length,600);t+=.1){const p=point(t);if(p.x<0||p.y<0||p.x>=255||p.y>=255){previous=undefined;continue;}const diff=p.z-terrain.heightAt(p.x,p.y);if(diff<=0&&previous!==undefined){let lo=previous,hi=t;for(let i=0;i<14;i++){const mid=(lo+hi)/2,q=point(mid);if(q.z>terrain.heightAt(q.x,q.y))lo=mid;else hi=mid;}const hit=point((lo+hi)/2);return {...hit,x:Math.floor(hit.x),y:Math.floor(hit.y)};}previous=t;}
 return null;
}
