// ZzzObject.cpp MoveObject/CreateObject and MapManager action overrides. Units: keys/s and UV/s.
export function objectAnimationRate(map,type,scale=1){if(map===0){if(type===0||type===1)return 10/Math.max(.001,Math.abs(scale));if([90,150,96,97].includes(type))return 7.5;if(type===59)return 0;}if(map===7&&type===40)return 1.25;if(map===51&&type===56)return 1.25;if(map===38){if(type===10)return 1;if(type===44)return .5;}if(map===57){if(type===16||type===17)return 20;if(type===68)return 1.25;}if(map===67&&type===40)return 1.25;if(map===68){if(type===44)return .5;if(type===46)return .25;if(type===70||type===90)return 1;}if([69,70,71,72,73,74].includes(map)&&type===36)return .5;if([69,70,71,72,73,74].includes(map)){if(type===64)return map===71?11:16;if(type===20)return 8;if([122,123,124].includes(type))return 12;if(type===128)return 24;}if(map===62&&type===16)return 1.5;if(map===79&&type===8)return 5;if(map===80||map===81){if(type===66)return 3.75;if(type===107)return 125;}return 4;}
export function objectUVScroll(map,type,mesh){
 if([69,70,71,72,73,74].includes(map)&&type===115){if(mesh===1)return [0,.4];if(mesh===3)return [0,-.4];}
 const rules={69:{81:[-1,0,.375]},70:{81:[-1,0,.375]},71:{81:[-1,0,.375]},72:{81:[-1,0,.375]},73:{81:[-1,0,.375]},74:{81:[-1,0,.375]},0:{105:[3,0,-1]},1:{22:[1,0,-1],23:[1,0,-1],24:[1,0,-1]},3:{18:[2,0,1],41:[0,0,.5],42:[0,-2,0],43:[0,2,0]},4:{3:[-1,-1,0],4:[-1,-1,0],19:[4,-1,0],20:[4,-1,0]},6:{21:[3,0,-1]},51:{2:[-1,0,.375],53:[-1,0,.375],55:[-1,0,.375],89:[-1,0,.125]},68:{77:[-1,0,-.2],96:[0,0,-.05]},38:{42:[-1,-.2,-.2]},8:{2:[0,-1,0],4:[0,0,-.1],11:[-1,0,-.2],12:[-1,-.05,-.05],13:[-1,0,-.2],61:[1,0,-1]}};
 const r=rules[map]?.[type];return r&&(r[0]===-1||r[0]===mesh)?r.slice(1):[0,0];
}
// ZzzObject.cpp WD_2DEVIAS: doors20/65/88 rotate;86 slides, trigger distance200 world units.
export function isMovingDoor(map,type){return map===2&&[20,65,86,88].includes(type);}
export function doorPose(origin,current,focus,dt){
 const next={...current},distance=Math.hypot(focus.x*100-origin.x,focus.y*100-origin.y),near=distance<200,steps=Math.max(0,Math.min(.2,dt))*25;
 if(near){const amount=(200-distance)*2;if(origin.type===86){if(origin.rz===90)next.y=origin.y+amount;if(origin.rz===270)next.y=origin.y-amount;if(origin.rz===0)next.x=origin.x+amount;if(origin.rz===180)next.x=origin.x-amount;}else{const angles={90:30,270:330,0:300,180:240};if(Object.hasOwn(angles,origin.rz))next.rz=angles[origin.rz]+(origin.rz===270||origin.rz===180?1:-1)*(200-distance)*.5;}}
 else{let delta=((origin.rz-next.rz+540)%360)-180;next.rz+=Math.sign(delta)*Math.min(Math.abs(delta),10*steps);const f=1-Math.pow(.8,steps);next.x+=(origin.x-next.x)*f;next.y+=(origin.y-next.y)*f;}
 return {pose:next,near,sound:origin.type===86?'SOUND_DOOR02':'SOUND_DOOR01'};
}
// ZzzObject WD_8TARKAN BlendMeshLight. A phase belongs to each placement,
// so differently rotated lava surfaces cannot share a material batch.
export function objectMaterialPulse(map,object,mesh){if([69,70,71,72,73,74].includes(map)&&object.type===41&&mesh===1)return [2,.5,.6];if(map===51&&object.type===56&&mesh===0)return [3,.3,.5];if(map===7||map===67){if(object.type===23&&mesh===0)return [2,.3,.5];if([32,34].includes(object.type)&&mesh===1)return [4,.5,.5];if(object.type===40&&mesh===0)return [4,.3,.5];}if(map!==8||mesh!==0||![4,7].includes(object.type))return undefined;return object.type===7?object.rz*.2:0;}
export function objectMaterialTint(phase,seconds){const value=Array.isArray(phase)?Math.sin(seconds*phase[0])*phase[1]+phase[2]:Math.sin(seconds*2+phase)*.35+.65;return [value,value,value];}

// GMDoppelGanger4 MoveObject: translucent meshes; type96 RenderObjectVisual overrides alpha to1.
export function objectOpacity(map,type){return map===68&&type===76?.5:1;}

// GMEmpireGuardian1::RenderObjectMesh shared by all four controllers.
export function objectMaterialColor(map,type,mesh){if(![69,70,71,72,73,74].includes(map))return undefined;if(type===47&&mesh===3)return [1.2,1.2,1.2];if(type===48&&mesh===0)return [1.1,1.1,1.1];if([72,73,74].includes(map)&&[96,97,100].includes(type))return [.170382,.170382,.170382];return undefined;}

export function objectBrightMesh(map,type,mesh){if(![69,70,71,72,73,74].includes(map))return undefined;return [37,41].includes(type)&&mesh===1||[49,70].includes(type)&&mesh===0;}
