// Native world_view.mm skill4: start height1.2, target height1, speed12.5 world units/s.
// Fixed target is a technical fixture; no monster, hit or server claim.
export function fireProjectile(geometry,seconds,output){
 if(!Number.isFinite(seconds)||seconds<0)throw new Error('Invalid projectile clock');
 const duration=Math.max(.04,3/12.5),age=seconds%2,active=age<duration,f=Math.min(1,age/duration);
 // Native actor rig: Rx(-pi/2)*.009; Fire rig: Rx(-pi/2)*.01. Undo shared axis map.
 if(output&&(output.meshes.length!==geometry.meshes.length||geometry.meshes.some((mesh,index)=>output.meshes[index].positions.length!==mesh.positions.length||output.meshes[index].normals.length!==mesh.normals.length)))throw new Error('Projectile topology changed');
 const result=output??{meshes:geometry.meshes.map((mesh,index)=>({...mesh,positions:new Float32Array(mesh.positions.length),normals:new Float32Array(mesh.normals.length),additive:index===1,sourcePath:'Skill/Fire01.bmd'}))};
 geometry.meshes.forEach((mesh,index)=>{
  const {positions,normals}=result.meshes[index];
  for(let i=0;i<positions.length;i+=3){const x=mesh.positions[i],y=mesh.positions[i+1],z=mesh.positions[i+2];
   // World projectile yaw=atan2(3,0)=pi/2 -> native Rz(pi/2).
   positions[i]=-y*.01/.009+3*f/.009;positions[i+1]=x*.01/.009;positions[i+2]=z*.01/.009+(1.2-.2*f)/.009;
   normals[i]=-mesh.normals[i+1];normals[i+1]=mesh.normals[i];normals[i+2]=mesh.normals[i+2];
   if(!active){positions[i]=positions[i+1]=positions[i+2]=0;}
  }
 });
 return result;
}
