import {roadX,roadY} from '../protocol/movement-codec.js';
export const walkable=(terrain,x,y)=>Number.isInteger(x)&&Number.isInteger(y)&&x>=0&&y>=0&&x<256&&y<256&&!(terrain.flags[y*256+x]&12);
export function canTraverse(terrain,x,y,d){const nx=x+roadX[d],ny=y+roadY[d];return walkable(terrain,nx,ny)&&!(roadX[d]&&roadY[d]&&(!walkable(terrain,nx,y)||!walkable(terrain,x,ny)));}
// Bounded A*: 8 directions, diagonal cost sqrt(2); never cut blocked corners.
export function findRoute(terrain,start,goal,{radius=255,origin=start,range=0,maxVisited=65536}={}){
 const inside=(x,y)=>Math.abs(x-origin.x)<=radius&&Math.abs(y-origin.y)<=radius;
 if(!walkable(terrain,start.x,start.y)||!inside(goal.x,goal.y)||!Number.isInteger(goal.x)||!Number.isInteger(goal.y))return null;
 const reached=(x,y)=>Math.hypot(x-goal.x,y-goal.y)<=range;
 const heuristic=(x,y)=>Math.max(0,Math.hypot(x-goal.x,y-goal.y)-range);
 const costs=new Float64Array(65536).fill(Infinity),parent=new Int32Array(65536).fill(-1),dirs=new Int8Array(65536),closed=new Uint8Array(65536),heap=[];
 const push=node=>{heap.push(node);let i=heap.length-1;while(i){const p=(i-1)>>1;if(heap[p].f<=node.f)break;heap[i]=heap[p];i=p;}heap[i]=node;};
 const pop=()=>{const top=heap[0],last=heap.pop();if(heap.length){let i=0;while(i*2+1<heap.length){let c=i*2+1;if(c+1<heap.length&&heap[c+1].f<heap[c].f)c++;if(heap[c].f>=last.f)break;heap[i]=heap[c];i=c;}heap[i]=last;}return top;};
 const first=start.y*256+start.x;costs[first]=0;push({id:first,f:heuristic(start.x,start.y)});let visited=0;
 while(heap.length&&visited<maxVisited){const {id}=pop();if(closed[id])continue;closed[id]=1;visited++;const x=id%256,y=id>>8;if(reached(x,y)){const out=[];for(let at=id;at!==first;at=parent[at])out.push(dirs[at]);return out.reverse();}
 for(let d=0;d<8;d++){const nx=x+roadX[d],ny=y+roadY[d];if(!inside(nx,ny)||!canTraverse(terrain,x,y,d))continue;const next=ny*256+nx,cost=costs[id]+(roadX[d]&&roadY[d]?Math.SQRT2:1);if(cost>=costs[next])continue;costs[next]=cost;parent[next]=id;dirs[next]=d;push({id:next,f:cost+heuristic(nx,ny)});}}
 return null;
}
export function createRouteMotion(entry,terrain,{canRun=false}={}){
 let x=entry.x,y=entry.y,direction=entry.direction,angle=(direction-1)*Math.PI/4,revision=0,runTime=0;const steps=[];
 const safe=()=>!!(terrain.flags[Math.floor(y+.5)*256+Math.floor(x+.5)]&1);
 return {setCanRun(value){canRun=!!value;if(!canRun)runTime=0;},get revision(){return revision;},get busy(){return steps.length>0;},get position(){return {x,y,direction,angle,safe:safe(),moving:steps.length>0,running:!safe()&&canRun&&runTime>=1.6};},get remaining(){return steps.reduce((n,s,i)=>n+(i?Math.hypot(s.x-steps[i-1].x,s.y-steps[i-1].y):Math.hypot(s.x-x,s.y-y)),0);},
 append(start,directions){let tx=start.x,ty=start.y;for(const d of directions){tx+=roadX[d];ty+=roadY[d];steps.push({x:tx,y:ty,direction:d});}},
 correct(p){revision++;steps.length=0;x=p.x;y=p.y;direction=p.direction??direction;angle=(direction-1)*Math.PI/4;runTime=0;},
 stop(){steps.length=0;runTime=0;},
 face(d){direction=d;},
 advance(dt){if(!Number.isFinite(dt)||dt<0)throw new Error('Invalid motion delta');dt=Math.min(dt,.1);if(safe()||!steps.length)runTime=0;else runTime+=dt;let distance=dt*(!safe()&&canRun&&runTime>=1.6?3.75:3);
 while(steps.length&&distance>0){const next=steps[0],length=Math.hypot(next.x-x,next.y-y);direction=next.direction;if(length<=distance){x=next.x;y=next.y;steps.shift();distance-=length;}else{x+=(next.x-x)*distance/length;y+=(next.y-y)*distance/length;distance=0;}}
 const target=(direction-1)*Math.PI/4,delta=Math.atan2(Math.sin(target-angle),Math.cos(target-angle));angle+=delta*(1-Math.exp(-20*dt));return this.position;}
 };
}

// Supplemental SetPlayerWalk: CLASS_DARK means Magic Gladiator (no helmet).
// Mount, wing, buff and water-map movement require additional state; foot branch only.
export function footRunEligible(appearance,map){
 if([3,4,6].includes(appearance.baseClass))return true;
 if([7,24,25,26,27,28,29,36,67].includes(map))return false;
 const boots=appearance.parts[6];return !!(boots?.present&&boots.visualLevel>=5);
}
