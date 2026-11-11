// Conservative AABB clip test in native world coordinates [x,y,z]. The camera
// projects [x,z,-y,1]. A support point gives the same six-plane rejection as
// transforming all eight corners, without per-mesh corner/clip arrays.
export function worldBounds(positions){const min=[Infinity,Infinity,Infinity],max=[-Infinity,-Infinity,-Infinity];for(let i=0;i<positions.length;i++){const a=i%3;min[a]=Math.min(min[a],positions[i]);max[a]=Math.max(max[a],positions[i]);}return {min,max};}
export function createFrustumTest(matrix,margin=2){
 const planes=new Float64Array(24);let at=0;
 for(let axis=0;axis<3;axis++)for(const side of [1,-1]){
  planes[at++]=matrix[3]+side*matrix[axis];
  planes[at++]=-(matrix[11]+side*matrix[8+axis]);
  planes[at++]=matrix[7]+side*matrix[4+axis];
  planes[at++]=matrix[15]+side*matrix[12+axis];
 }
 return bounds=>{const {min,max}=bounds;
  for(let i=0;i<24;i+=4){const x=planes[i],y=planes[i+1],z=planes[i+2],distance=x*(x>=0?max[0]+margin:min[0]-margin)+y*(y>=0?max[1]+margin:min[1]-margin)+z*(z>=0?max[2]+margin:min[2]-margin)+planes[i+3];
   // Keep touching/rounding-edge bounds visible instead of risking holes.
   if(distance< -1e-7)return false;
  }return true;
 };
}
export function boundsVisible(bounds,matrix,margin=2){return createFrustumTest(matrix,margin)(bounds);}
