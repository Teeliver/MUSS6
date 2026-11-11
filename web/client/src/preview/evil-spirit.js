// Port of native/client/evil_spirit.hpp; deterministic 25 Hz visual simulation.
const rad=Math.PI/180;
export class EvilSpirits {
 constructor(seed=1){this.random=seed>>>0||1;this.frame=0;this.accumulator=0;this.heads=Array.from({length:4},(_,i)=>({p:{x:0,y:0,z:1},yaw:i*90,pitch:0,dyaw:0,dpitch:0,tail:[{x:0,y:0,z:1}]}));}
 next(){let n=this.random;n^=n<<13;n^=n>>>17;n^=n<<5;return this.random=n>>>0;}
 turn(a,b){const d=b-a,quotient=d/360,base=Math.trunc(quotient),fraction=Math.abs(quotient-base),nearest=fraction<.5?base:fraction>.5?base+Math.sign(quotient):base%2===0?base:base+Math.sign(quotient);let remainder=d-nearest*360;return a+Math.max(-10,Math.min(10,remainder));}
 step(){if(this.frame>=50)return;for(const h of this.heads){const sy=Math.sin(h.yaw*rad),cy=Math.cos(h.yaw*rad),sp=Math.sin(h.pitch*rad),cp=Math.cos(h.pitch*rad);h.p={x:h.p.x+.7*sy*cp,y:h.p.y-.7*cy*cp,z:h.p.z-.7*sp};const dx=0-h.p.x,dy=0-h.p.y,d=Math.hypot(dx,dy);h.yaw=this.turn(h.yaw,Math.atan2(dx,-dy)/rad);h.pitch=this.turn(h.pitch,-Math.atan2(.8-h.p.z,d)/rad);h.dpitch+=(this.next()%32-16)*.2;h.dyaw+=(this.next()%32-16)*.8;h.pitch+=h.dpitch;h.yaw+=h.dyaw;h.dpitch*=.6;h.dyaw*=.8;if(h.p.z<1){h.dpitch=0;h.pitch=-5;}if(h.p.z>4){h.dpitch=0;h.pitch=5;}h.tail.unshift({...h.p});if(h.tail.length>6)h.tail.pop();}this.frame++;}
 advance(dt){if(!Number.isFinite(dt)||dt<0||dt>60)throw new Error('Invalid spirit delta');this.accumulator+=dt;while(this.accumulator>=.04&&this.frame<50){this.accumulator-=.04;this.step();}}
}
export function spiritGeometry(headGeometry,simulation,seconds,output){
 const heads=simulation.heads,luminosity=Math.max(0,(49-simulation.frame)*.1),opacity=Math.min(1,Math.max(0,(2-seconds)*2.5));
 const result=output??{meshes:[...heads.flatMap(()=>headGeometry.meshes.map(mesh=>({...mesh,positions:new Float32Array(mesh.positions.length),normals:new Float32Array(mesh.normals.length),multiply:true}))),...Array.from({length:8},()=>({positions:new Float32Array(90),normals:new Float32Array(90).fill(1),uv:new Float32Array(60),multiply:true,sourcePath:'Effect/JointSpirit01.OZJ',texturePath:'Effect/JointSpirit01.OZJ'}))]};
 let index=0;
 for(const h of heads){const cy=Math.cos(h.yaw*rad),sy=Math.sin(h.yaw*rad),cp=Math.cos(h.pitch*rad),sp=Math.sin(h.pitch*rad);
  for(const source of headGeometry.meshes){const mesh=result.meshes[index++];for(let i=0;i<source.positions.length;i+=3){const x=source.positions[i]*.013,y=source.positions[i+1]*.013,z=source.positions[i+2]*.013,ry=cp*y-sp*z,rz=sp*y+cp*z;mesh.positions[i]=(cy*x-sy*ry+h.p.x)/.009;mesh.positions[i+1]=(sy*x+cy*ry+h.p.y)/.009;mesh.positions[i+2]=(rz+h.p.z)/.009;mesh.normals[i]=source.normals[i];mesh.normals[i+1]=source.normals[i+1];mesh.normals[i+2]=source.normals[i+2];}}
 }
 for(let ribbon=0;ribbon<8;ribbon++){const mesh=result.meshes[index++],tail=heads[Math.floor(ribbon/2)].tail,width=ribbon%2?.1:.4;let vertex=0;for(let segment=1;segment<=5;segment++)for(const [j,side] of [[segment-1,-1],[segment-1,1],[segment,-1],[segment-1,1],[segment,1],[segment,-1]]){const p=tail[Math.min(j,tail.length-1)],active=segment<tail.length;mesh.positions.set(active?[(p.x+side*width*(1-j/6))/.009,p.y/.009,p.z/.009]:[0,0,0],vertex*3);mesh.uv.set([(side+1)/2,j/5],vertex*2);vertex++;}}
 for(const mesh of result.meshes){mesh.spiritLight=luminosity;mesh.opacity=opacity;}return result;
}
