// Explicit source recipes; bone matrices remain native BMD units until owner transform.
export function boneEffectRecipes(map,objects){const out=[];for(const object of objects){const add=(bone,offset,texture,style)=>out.push({object,bone,offset,texture,style});
 if(map===79&&object.type===30){for(const bone of [1,2,3])add(bone,[0,0,0],'Effect/Flare.OZJ','market30');}
 if(map===79&&object.type===35)add(2,[0,0,0],'Effect/Flare.OZJ','market35');
 if(map===51&&object.type===63)add(5,[-40,-10,0],'Effect/flare01.OZJ','elbe63');
 if(map===51&&object.type===121)for(let bone=3;bone<=8;bone++)add(bone,[5,-4,-1],'Effect/flare01.OZJ','elbe121');
 if([69,70,71,72,73,74].includes(map)&&object.type===37)add(1,[0,0,0],'Effect/Flare.OZJ','empire37');
 if([69,70,71,72,73,74].includes(map)&&object.type===12){for(const [bone,offset] of [[2,[8,-3,-3]],[3,[3,-3,-3.5]]]){add(bone,offset,'Effect/shiny05.OZJ','empire12red');add(bone,offset,'Effect/shiny05.OZJ','empire12pulse');}}
 }return out;}
export function boneEffectPoint(object,matrix,offset){const [a,b,c]=offset,x=(matrix[0]*a+matrix[4]*b+matrix[8]*c+matrix[12])*object.scale,y=(matrix[1]*a+matrix[5]*b+matrix[9]*c+matrix[13])*object.scale,z=(matrix[2]*a+matrix[6]*b+matrix[10]*c+matrix[14])*object.scale,[ax,ay,az]=[object.rx,object.ry,object.rz].map(v=>v*Math.PI/180),cx=Math.cos(ax),sx=Math.sin(ax),cy=Math.cos(ay),sy=Math.sin(ay),cz=Math.cos(az),sz=Math.sin(az);return {x:(cz*cy*x+(cz*sy*sx-sz*cx)*y+(cz*sy*cx+sz*sx)*z+object.x)*.01,y:(sz*cy*x+(sz*sy*sx+cz*cx)*y+(sz*sy*cx-cz*sx)*z+object.y)*.01,z:(-sy*x+cy*sx*y+cy*cx*z+object.z)*.01};}
export function boneEffectStyle(recipe,time){const scale=recipe.object.scale;if(recipe.style==='empire12red')return {scale:.5,light:[.9,.1,.1],terrain:null};if(recipe.style==='empire12pulse'){const l=Math.abs(Math.sin(time*.0008))*.9+.1;return {scale:scale*.3*l,light:[l,l,l],terrain:null};}if(recipe.style==='market30'||recipe.style==='market35'||recipe.style==='empire37'){const l=(Math.sin(time*(recipe.style==='market30'?.002:.039))+1)*.2+.6,k=recipe.style==='market30'?.5:.7;return {scale:(recipe.style==='empire37'?4:1.5)*scale,light:[l*k,l*k,l*k],terrain:recipe.style==='empire37'?null:{range:recipe.style==='market30'?3:1,color:[l*.7,l*.5,l*.2]}};}
 const l=recipe.style==='elbe63'?(Math.sin(time*.001)+1)*.3+.4:(Math.sin(time*.002)+1)*.5+.5;return {scale:recipe.style==='elbe63'?6*scale:l,light:recipe.style==='elbe63'?[l,l,l*.6]:[l,l*.5,l*.3],terrain:null};}

// GMEmpireGuardian1/2/3/4::RenderObjectVisual type50: two particles at each of six bones.
export function boneParticleRecipes(map,objects){if(![69,70,71,72,73,74].includes(map))return [];const out=[];for(const object of objects)if(object.type===50)for(let bone=2;bone<=7;bone++)for(const [ratio,light] of [[.6,[.05,.1,.3]],[.3,[1,1,1]]])out.push({object,bone,offset:[0,0,5],ratio,light,kinds:['boneHik3'],probability:0});return out;}
