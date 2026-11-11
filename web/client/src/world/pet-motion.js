// Native pet controllers run at 25 Hz. Rendering interpolates their poses, not their rules.
const turn=(from,to,limit)=>from+Math.max(-limit,Math.min(limit,Math.atan2(Math.sin(to-from),Math.cos(to-from))));
export function petTarget(owner,controller,time){const circle=[4,5,6,7].includes(controller),phase=time*Math.PI/2000;return {x:owner.x+(circle?Math.sin(phase)*.5:0),y:owner.y+(circle?Math.cos(phase)*.5:0)};}
export function createPetMotion(owner,policy,time,{random=Math.random}={}){const born=time,unicorn=policy.controller===6,spawnAngle=random()*Math.PI*2;let pose={...(unicorn?{x:owner.x+Math.cos(spawnAngle),y:owner.y+Math.sin(spawnAngle)}:petTarget(owner,policy.controller,0)),angle:(owner.direction-1)*Math.PI/4},before={...pose},last=time,clock=time,remaining=0,speed=0,oldOwner={...owner},frame=0,beforeFrame=0,velocity=(policy.keys??20)/25;
 const collector=[4,5,6,7].includes(policy.controller),range=collector?.1:policy.range;
 function step(owner,time){before={...pose};beforeFrame=frame;if(unicorn)frame+=velocity;const target=petTarget(owner,policy.controller,time-born),dx=target.x-pose.x,dy=target.y-pose.y,d=Math.hypot(dx,dy);
  if(d>9){pose={...target,angle:(owner.direction-1)*Math.PI/4};before={...pose};speed=0;return;}
  // Unicorn/collector turn unconditionally in Stand (80 >= FlyRange), even within 10 units.
  if(collector||d>range)pose.angle=turn(pose.angle,Math.atan2(dx,-dy),(collector?8:10)*Math.PI/180);
  // Direction is the previous native tick's velocity, rotated by the new yaw.
  pose.x+=Math.sin(pose.angle)*speed*.01;pose.y-=Math.cos(pose.angle)*speed*.01;
  speed=d<=range?0:collector?Math.log(Math.max(1,d*100))*2.3:Math.log(Math.max(1,d*d*10000))+5;if(unicorn)velocity=(policy.keys??20)/25*(speed===0?.35:1.2);
 }
 return {advance(owner,time){const elapsed=Math.max(0,time-last);if(elapsed>500||Math.hypot(owner.x-oldOwner.x,owner.y-oldOwner.y)>9){pose={...petTarget(owner,policy.controller,time-born),angle:(owner.direction-1)*Math.PI/4};before={...pose};clock=time;remaining=0;speed=0;}
  else{remaining+=elapsed;while(remaining>=40){clock+=40;const f=elapsed?Math.max(0,Math.min(1,(clock-last)/elapsed)):1;step({...owner,x:oldOwner.x+(owner.x-oldOwner.x)*f,y:oldOwner.y+(owner.y-oldOwner.y)*f},clock);remaining-=40;}}
  last=time;oldOwner={...owner};const f=remaining/40,angle=before.angle+Math.atan2(Math.sin(pose.angle-before.angle),Math.cos(pose.angle-before.angle))*f;
  return {x:before.x+(pose.x-before.x)*f,y:before.y+(pose.y-before.y)*f,angle,direction:angle*4/Math.PI+1,animationTime:unicorn?(beforeFrame+(frame-beforeFrame)*f)/6.25:undefined,animationRate:policy.controller===6?(speed===0?.35:1.2):1};},get position(){return {...pose,direction:pose.angle*4/Math.PI+1};}};
}
