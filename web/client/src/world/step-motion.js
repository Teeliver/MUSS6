import {roadX,roadY} from '../protocol/movement-codec.js';
// First movement slice: one server-confirmed walking step, no prediction/run.
export function createStepMotion(entry,terrain,{radius=12}={}){
 let x=entry.x,y=entry.y,fromX=x,fromY=y,targetX=x,targetY=y,progress=1,direction=entry.direction,revision=0;
 const origin={x,y};
 const blocked=(x,y)=>x<0||y<0||x>255||y>255||(terrain.flags[y*256+x]&12)!==0;
 return {
  get revision(){return revision;},
  get busy(){return progress<1;},
  get position(){return {x,y,direction,moving:progress<1};},
  canStep(d){if(!Number.isInteger(d)||d<0||d>7||progress<1)return false;const nx=targetX+roadX[d],ny=targetY+roadY[d];return Math.abs(nx-origin.x)<=radius&&Math.abs(ny-origin.y)<=radius&&!blocked(nx,ny)&&!(roadX[d]&&roadY[d]&&(blocked(nx,targetY)||blocked(targetX,ny)));},
  confirm(reply){fromX=x;fromY=y;targetX=reply.x;targetY=reply.y;direction=reply.direction??direction;progress=0;},
  correct(reply){revision++;x=fromX=targetX=reply.x;y=fromY=targetY=reply.y;direction=reply.direction??direction;progress=1;},
  advance(dt){if(!Number.isFinite(dt)||dt<0)throw new Error('Invalid motion delta');const distance=Math.hypot(targetX-fromX,targetY-fromY);progress=Math.min(1,progress+Math.min(dt,.05)*3/(distance||1));x=fromX+(targetX-fromX)*progress;y=fromY+(targetY-fromY)*progress;return this.position;}
 };
}
export function installStepKeys(target,{motion,move,onPosition=()=>{},onError=()=>{},hidden=()=>false}){
 let pending=false,disposed=false;const keys={ArrowUp:1,ArrowRight:3,ArrowDown:5,ArrowLeft:7};
 const listener=async event=>{
  if(disposed||hidden()||event.defaultPrevented||event.altKey||event.ctrlKey||event.metaKey||event.shiftKey||event.target?.closest?.('input,textarea,select,button,[contenteditable]'))return;
  const d=keys[event.key];if(d===undefined)return;event.preventDefault();if(pending||!motion.canStep(d))return;pending=true;const revision=motion.revision;
  try{const position=await move([d]);if(disposed||motion.revision!==revision)return;motion.confirm(position);onPosition(position);}catch(e){if(!disposed)onError(e);}finally{pending=false;}
 };
 target.addEventListener('keydown',listener);
 return ()=>{disposed=true;target.removeEventListener('keydown',listener);};
}
