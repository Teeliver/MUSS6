// RenderGuild() in Main5.2/source/ZzzCharacter.cpp:
//   Matrix = AngleMatrix(o->Angle + (80,45,135)); Matrix[0..2][3] = (20,-5,-10)
//   ParentMatrix = o->BoneTransform[26] * Matrix; RenderPlane3D(5,7,ParentMatrix)
// RenderPlane3D corners are (+-5,+-5,+-7) in the x=y diagonal plane, with
// UVs (0,1) (1,1) (1,0) (0,0). The shader receives (u,1-v), so the attribute
// values below are the source UVs flipped on V.
// WASM bone matrices are row-vector 4x4 (translation at 12..14), therefore
// Parent_row = Matrix_row * Bone_row, the transpose of the source product.
import {guildMarkTexture} from '../ui/guild-mark.js';
export const GUILD_MARK_BONE=26;
export const GUILD_MARK_WIDTH=5;
export const GUILD_MARK_HEIGHT=7;
export const GUILD_MARK_OFFSET=[20,-5,-10];
export const GUILD_MARK_ANGLES=[80,45,135];
const GUILD_MARK_CORNERS=[[-5,-5,-7],[5,5,-7],[5,5,7],[-5,-5,7]];
const GUILD_MARK_UV=Float32Array.from([0,0,1,0,1,1,0,0,1,1,0,1]);
const radians=degree=>degree*Math.PI/180;
function angleMatrix(ax,ay,az){
 // ZzzMathLib.cpp AngleMatrix, row-major [row][column].
 const sy=Math.sin(radians(az)),cy=Math.cos(radians(az)),sp=Math.sin(radians(ay)),cp=Math.cos(radians(ay)),sr=Math.sin(radians(ax)),cr=Math.cos(radians(ax));
 return [
  [cp*cy,sr*sp*cy+cr*-sy,cr*sp*cy+-sr*-sy],
  [cp*sy,sr*sp*sy+cr*cy,cr*sp*sy+-sr*cy],
  [-sp,sr*cp,cr*cp]
 ];
}
function offsetMatrix(angle){
 const [ax,ay,az]=GUILD_MARK_ANGLES,m=angleMatrix(ax,ay,az+angle*180/Math.PI),[tx,ty,tz]=GUILD_MARK_OFFSET;
 // Row-vector matrix: rows are contiguous, translation lives in row 3 (12..14).
 // The row-vector rotation is the transpose of the source AngleMatrix.
 return [m[0][0],m[1][0],m[2][0],0,m[0][1],m[1][1],m[2][1],0,m[0][2],m[1][2],m[2][2],0,tx,ty,tz,1];
}
function multiply(a,b){
 const out=new Array(16);
 for(let row=0;row<4;row++)for(let column=0;column<4;column++){let value=0;for(let k=0;k<4;k++)value+=a[row*4+k]*b[k*4+column];out[row*4+column]=value;}
 return out;
}
function transformPoint(matrix,point){
 return [0,1,2].map(row=>point[0]*matrix[row]+point[1]*matrix[4+row]+point[2]*matrix[8+row]+matrix[12+row]);
}
// World mapping matches the character mesh transform (model units -> world
// tiles): ×0.01, rotate around Z by the object angle, add map-relative x/y and
// ground. RenderGuild() itself does not apply BodyScale.
export function guildMarkQuad(bone,{angle=0,x=0,y=0,ground=0}={}){
 if(!bone||bone.length<16)return undefined;
 const factor=.01,parent=multiply(offsetMatrix(angle),bone),
  c=Math.cos(angle),s=Math.sin(angle);
 return GUILD_MARK_CORNERS.map(corner=>{
  const [px,py,pz]=transformPoint(parent,corner).map(value=>value*factor);
  return [x+c*px-s*py,y+s*px+c*py,ground+pz];
 });
}
export function createGuildMarkMesh(mark){
 const texture=guildMarkTexture(mark);
 if(!texture)return undefined;
 return {guildMark:true,image:texture,guildTexture:texture,positions:new Float32Array(18),normals:new Float32Array(18),uv:Float32Array.from(GUILD_MARK_UV),castsShadow:false};
}
export function applyGuildMarkMesh(mesh,bone,owner){
 const quad=guildMarkQuad(bone,owner);
 if(!mesh||!quad)return mesh;
 const [a,b,c]=quad,positions=mesh.positions,normals=mesh.normals,
  ux=b[0]-a[0],uy=b[1]-a[1],uz=b[2]-a[2],vx=c[0]-a[0],vy=c[1]-a[1],vz=c[2]-a[2],
  nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx,length=Math.hypot(nx,ny,nz)||1;
 for(const [slot,point] of [[0,a],[1,b],[2,c],[3,a],[4,c],[5,quad[3]]]){
  positions[slot*3]=point[0];positions[slot*3+1]=point[1];positions[slot*3+2]=point[2];
  normals[slot*3]=nx/length;normals[slot*3+1]=ny/length;normals[slot*3+2]=nz/length;
 }
 return mesh;
}
