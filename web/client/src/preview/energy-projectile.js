import {PREVIEW_ANGLE} from '../platform/rendering/model-preview.js';
// Native world_view.mm skill17: Thunder01 additive billboard, 0.6 world units,
// height 1, speed 15. Synthetic target 3 units away; no hit/server simulation.
export function energyProjectile(seconds,output){
 if(!Number.isFinite(seconds)||seconds<0)throw new Error('Invalid projectile clock');
 const result=output??{meshes:[{positions:new Float32Array(18),normals:new Float32Array(18),uv:Float32Array.of(0,0,1,0,1,1,0,0,1,1,0,1),additive:true,texture:'Thunder01.jpg',sourcePath:'Effect/Thunder01.OZJ',texturePath:'Effect/Thunder01.OZJ'}]};
 const mesh=result.meshes[0],age=seconds%2,active=age<.2,f=Math.min(1,age/.2),corners=[[-1,-1],[1,-1],[1,1],[-1,-1],[1,1],[-1,1]],c=Math.cos(PREVIEW_ANGLE),s=Math.sin(PREVIEW_ANGLE);
 for(let i=0;i<6;i++){
  const [u,v]=corners[i],x=u*.3/.009;
  // Inverse camera yaw yields a camera-facing plane after renderer axis map.
  mesh.positions[i*3]=3*f/.009+c*x;mesh.positions[i*3+1]=-s*x;mesh.positions[i*3+2]=(1+v*.3)/.009;
  mesh.normals[i*3]=s;mesh.normals[i*3+1]=c;mesh.normals[i*3+2]=0;
  if(!active)mesh.positions.fill(0,i*3,i*3+3);
 }
 return result;
}
