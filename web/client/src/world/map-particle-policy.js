// GMSwampOfQuiet::{MoveObject,RenderObjectVisual}; GMCrywolf1st::Render...Mesh.
// ZzzEffectParticle create/move: TRUE_FIRE5, SMOKE21/49, CLOUD20.
export const mapParticleTextures={boneHik3:'Effect/firehik_mono03.OZJ',trueFire0:'Effect/fantaF.OZJ',trueBlue0:'Effect/fantaB.OZJ',flame12:'Effect/Flame01.OZJ',cloud22:'Effect/clouds.OZJ',cloud21:'Effect/clouds.OZJ',smoke22:'Effect/smoke01.OZJ',market1:'Effect/firehik01.OZJ',market2:'Effect/firehik02.OZJ',market3:'Effect/firehik03.OZJ',bubble:'Object8/drop01.OZJ',smoke60:'Effect/smoke01.OZJ',fire:'Effect/fantaF.OZJ',smoke:'Effect/smoke01.OZJ',mist:'Effect/clouds.OZJ',cloud:'Effect/clouds.OZJ',cloudDark:'Effect/clouds.OZJ',cloudTint:'Effect/clouds.OZJ',hik1:'Effect/firehik_mono01.OZJ',hik2:'Effect/firehik_mono02.OZJ',hik3:'Effect/firehik_mono03.OZJ',water7:'Effect/WATERFALL3.OZJ'};
export function mapParticleEmitters(map,objects){if(![7,34,38,51,56,57,58,65,67,68,69,70,71,72,73,74,79].includes(map))return [];const out=[];for(const o of objects){let kinds=[],probability=1/3,scale=o.scale,z=o.z;
 if(map===51){if(o.type===0)kinds=['trueFire0'];else if(o.type===61)kinds=['trueBlue0'];}
 else if(map===79){if(o.type===57){kinds=['market1','market2','market3'];probability=1;}}
 else if([7,67].includes(map)&&o.type===22||map===68&&o.type===97){kinds=['bubble'];probability=1;}
 else if(map===34||map===56){if(o.type===57)kinds=['fire','smoke'];else if(o.type===71)kinds=['fire'];
  else if(map===56){if(o.type===72){kinds=['mist'];probability=1/6;z+=50;}else if(o.type===74){kinds=['smoke'];scale*=2;}else if(o.type===77||o.type===78){kinds=['cloud'];probability=1/6;}}}
 else if(map===38){if(o.type===48)kinds=['cloudDark'];if(o.type===50||o.type===51)kinds=['cloudTint'];if(o.type===10||o.type===52){kinds=['water7'];probability=1/6;if(o.type===10)z=410;}}
 else if([69,70,71,72,73,74].includes(map)){if(o.type===85){kinds=['flame12'];probability=0;}else if(o.type===64){kinds=['cloud22'];probability=0;}else if(o.type===79){kinds=['market1','market2','market3'];probability=1;}else if(o.type===131)kinds=['smoke22','smoke'];else if(o.type===132)kinds=['smoke60','smoke'];else if([86,129,130].includes(o.type)){kinds=['cloud21'];probability=1/6;}}
 else if((map===57||map===58||map===65)&&(o.type===70||o.type===80)||map===68&&o.type===47){kinds=['hik1','hik2','hik3'];probability=1;}
 if(kinds.length)out.push({...o,map,z,scale,kinds,probability});}return out;}
// Raklion emits one random monochrome family plus HIK1 each visual tick.
export function mapEmissionKinds(e,random=Math.random){return e.kinds.includes('market1')?[e.kinds[Math.floor(random()*3)]]:e.kinds.includes('hik1')?['hik'+(Math.floor(random()*3)+1),'hik1']:e.kinds.includes('smoke60')?['smoke60','smoke60','smoke']:e.kinds;}
export function mapParticleMultiply(kind){return kind==='smoke'||kind==='cloudDark';}
export function spawnMapParticle(e,kind,random=Math.random){const n=max=>Math.floor(random()*max),p={kind,x:e.x*.01,y:e.y*.01,z:e.z*.01,rx:e.rx,ry:e.ry,rz:e.rz,scale:e.scale,rotation:0,gravity:0,vx:0,vy:0,vz:0,alpha:1,light:[1,1,1]};
 if(kind==='trueFire0'||kind==='trueBlue0'){p.life=24;p.vx=(n(10)-5)*.004;p.vz=(n(10)+5)*.002;}
 else if(kind==='boneHik3'){p.life=n(5)+34;p.scale*=(n(72)+72)*.01;p.rotation=n(360)*Math.PI/180;p.gravity=(n(10)+40)*.001;p.alpha=0;p.light=[...e.light];}
 else if(kind==='flame12'){p.life=10;p.vz=(n(128)+128)*.0015;p.scale*=(n(64)+64)*.01;p.light=[.5,.5,.5];}
 else if(kind==='cloud22'){p.life=80;n(1000);p.alpha=.1;p.light=[.04,.03,.02];p.x+=(n(20)-10)*.01;p.y+=(n(20)-10)*.01;p.z+=(n(20)-20)*.01;p.scale*=(n(20)+50)*.003;const angle=(p.rz+n(360))*Math.PI/180,speed=(n(10)*.1+.2)*.01;p.vx=-Math.sin(angle)*speed;p.vy=Math.cos(angle)*speed;}
 else if(kind.startsWith('market')){p.life=n(5)+(kind==='market1'?27:kind==='market2'?12:17);p.scale*=(n(72)+72)*.01;p.rotation=n(360)*Math.PI/180;p.gravity=(n(24)+64)*.001;p.alpha=0;}
 else if(kind==='bubble'){p.life=30+n(10);p.scale=(n(6)+4)*(e.map===68?.04:.03);p.frame=0;p.bubbleSubtype=e.map===68?5:0;}
 else if(kind.startsWith('hik')){p.life=n(5)+(kind==='hik1'?27:kind==='hik2'?24:17);p.scale*=(n(72)+72)*.01;p.rotation=n(360)*Math.PI/180;p.gravity=(n(24)+100)*.001;const drift=n(50)*.0003;p.vx=.025+drift;p.vy=-(kind==='hik1'?.07:.05)-drift;p.alpha=0;p.light=(e.type===70||e.map===68&&e.type===47)?[.1,.4,1]:[.7,.2,.1];}
 else if(kind==='cloudDark'||kind==='cloudTint'){p.light=kind==='cloudDark'?[0,0,0]:e.type===50?[0,.01,n(3)*.01+.02]:[n(3)*.01+.01,0,0];p.life=500;p.x+=(n(400)-200)*.01;p.y+=(n(400)-200)*.01;p.z+=(n(400)-200)*.01;p.start=[p.x,p.y,p.z];p.scale+=(n(15)+(kind==='cloudDark'?10:15))/(kind==='cloudDark'?15:30);p.vx=p.vy=(n(10)+5)*.0012;}
 else if(kind==='water7'){p.life=60;n(360);p.scale=(n(10)+10)*.02;p.vz=(n(4)*.1+.8)*.01;p.scale*=(n(10)+15)*.02;p.rotation=n(360)*Math.PI/180;p.x+=(n(18)-9)*.01;p.z+=(n(18)-9)*.01;}
 else if(kind==='fire'){p.life=20;p.vx=(n(10)-5)*.004;p.vz=(n(10)+5)*.002;}
 else if(kind==='smoke60'||kind==='smoke22'||kind==='smoke'){p.life=kind==='smoke'?80:60;if(kind==='smoke60')p.light.fill(.4);else if(kind==='smoke'&&[69,70,71,72,73,74].includes(e.map))p.scale*=2;p.scale*= (n(64)+64)*.005;p.rotation=n(360)*Math.PI/180;p.gravity=(n(32)+60)*.001;}
 else if(kind==='mist'){p.light=[.03,.03,.03];p.life=n(50)+100;p.rotation=n(360)*Math.PI/180;p.scale*=.4;p.spin=n(2)===0?1:-1;}
 else if(kind==='cloud21'){p.light=e.type===86?[.05,.02,.01]:e.type===129?[.01,.02,.05]:[.01,.05,.02];p.life=100;n(1000);p.alpha=.6;p.x+=(n(200)-100)*.01;p.y+=(n(200)-100)*.01;p.z+=(n(20)-20)*.01;n(100);p.scale*=(n(20)+180)*.01;n(10);}
 else if(kind==='cloud'){p.light=e.type===77?[.04,.06,.03]:[.03,.03,.05];p.life=300;n(1000);p.alpha=.1;p.x+=(n(500)-250)*.01;p.y+=(n(500)-250)*.01;p.z+=(n(20)-20)*.01;n(100);p.scale*=(n(20)+180)*.01;n(10);}
 else throw Error('Unknown native map particle');return p;}
export function stepMapParticle(p,random=Math.random,time=0){const n=max=>Math.floor(random()*max);p.life--;if(p.life<=0)return false;
 // MovePosition rotates velocity by AngleMatrix before subtype updates.
 const [ax,ay,az]=[p.rx,p.ry,p.rz].map(a=>a*Math.PI/180),cx=Math.cos(ax),sx=Math.sin(ax),cy=Math.cos(ay),sy=Math.sin(ay),cz=Math.cos(az),sz=Math.sin(az);
 p.x+=cz*cy*p.vx+(cz*sy*sx-sz*cx)*p.vy+(cz*sy*cx+sz*sx)*p.vz;p.y+=sz*cy*p.vx+(sz*sy*sx+cz*cx)*p.vy+(sz*sy*cx-cz*sx)*p.vz;p.z+=-sy*p.vx+cy*sx*p.vy+cy*cx*p.vz;
 if(p.kind==='trueFire0'||p.kind==='trueBlue0'){p.scale=Math.max(0,p.scale-.02);p.vx*=.95;p.vy*=.95;p.z+=.01;p.light.fill(p.life/25);return true;}
 if(p.kind==='boneHik3'){if(p.life<15)p.alpha-=.2;else if(p.alpha<1)p.alpha+=(n(2)+2)*.1;else p.alpha=1;if(p.alpha<.1||p.scale<=0)return false;p.scale-=(n(4)+5)*.01;p.z+=p.gravity;p.rotation+=3*Math.PI/180;return true;}
 if(p.kind==='flame12')return true;
 if(p.kind==='cloud22'){if(p.life>40){if(p.alpha<1)p.alpha+=.1;p.z+=.005;}else{if(p.alpha>0)p.alpha-=.025;else return false;p.z-=.005;}p.scale+=.001;return true;}
 if(p.kind.startsWith('market')){if(p.life<(p.kind==='market1'?15:10))p.alpha-=.2;else if(p.alpha<1)p.alpha+=(n(2)+2)*.1;else p.alpha=1;if(p.alpha<.1||p.scale<=0)return false;p.scale-=(n(3)+(p.kind==='market1'?5:p.kind==='market2'?6:7))*.01;p.z+=p.gravity;p.rotation+=3*Math.PI/180;return true;}
 else if(p.kind==='bubble'){p.frame++;p.x+=(n(20)-10)*.025*p.scale;p.y+=(n(20)-10)*.025*p.scale;p.z+=(n(20)+10)*.025*p.scale;return p.bubbleSubtype!==5||p.z<=5.5;}
 else if(p.kind.startsWith('hik')){if(p.life<10)p.alpha-=.2;else if(p.alpha<1)p.alpha+=(n(2)+2)*.1;else p.alpha=1;if(p.alpha<.1)return false;p.scale-=(n(3)+(p.kind==='hik2'?6:7))*.01;p.x+=p.vx;p.y+=p.vy;p.z+=p.gravity;p.rotation+=3*Math.PI/180;return p.scale>0;}
 else if(p.kind==='cloudDark'||p.kind==='cloudTint'){p.x+=p.vx;p.y+=p.vy;const distance=Math.hypot(p.x-p.start[0],p.y-p.start[1],p.z-p.start[2]);if(distance>5)return false;if(p.kind==='cloudDark'){if(distance<=3)p.light=p.light.map(v=>Math.min(.15,v+.01));else p.light=p.light.map(v=>v/1.05);}else p.rotation+=(.5+n(2))*Math.PI/180;return p.scale>0;}
 else if(p.kind==='water7'){p.vz+=.0001;p.light=p.light.map(v=>v*.97);return true;}
 else if(p.kind==='fire'){p.scale=Math.max(0,p.scale-.02);p.vx*=.95;p.vy*=.95;p.z+=.01;p.light.fill(p.life/25);}
 else if(p.kind==='smoke60'||p.kind==='smoke22'){p.gravity-=.001;p.x+=p.gravity*Math.sin(time)*.05;p.z+=p.gravity;p.scale+=.04;p.light=p.kind==='smoke60'?[.4,.4,.4].map(v=>v*p.life/50):[.9,.5,.5].map(v=>v*p.life/50);}
 else if(p.kind==='smoke'){p.gravity-=.001;p.x-=p.gravity*.2;p.z+=p.gravity;p.scale-=.01;p.light.fill(p.life/50);}
 else if(p.kind==='mist'){p.light=p.light.map(v=>v*(p.life<100?.97:1.03));if(p.light[0]<=.01)return false;p.scale+=(n(15)+4)*.001;p.z+=p.scale*.06;p.rotation+=(n(10)+10)*.1*p.spin*Math.PI/180;}
 else if(p.kind==='cloud21'){if(p.life>50){if(p.alpha<1)p.alpha+=.04;p.z+=.02;}else{if(p.alpha>.1)p.alpha-=.005;p.z-=.01;}}
 else if(p.life>150){if(p.alpha<1)p.alpha+=.04;p.z+=.01;}else{if(p.alpha>.1)p.alpha-=.025;p.z-=.005;}
 return p.scale>0;}
export function nativeMapLightNodes(map,objects){if(map===8)return objects.filter(o=>[4,7,61].includes(o.type)).map(o=>({...o,map,range:o.type===61?2:3}));if(![34,56].includes(map))return [];return objects.filter(o=>o.type===57||map===34&&[41,71].includes(o.type)).map(o=>({...o,map,range:o.type===41?2:3}));}
export function nativeMapLightColor(node,time,random=Math.random){if(node.map===8){const l=Math.sin(time*.002+(node.type===7?node.rz*.2:0))*.35+.65;return node.type===4?[l,l,l]:[l,l*.6,l*.2];}const l=(Math.floor(random()*4)+3)*.1;return node.type===41?[.2,.7,.5]:[l,l*.6,l*.2];}

// OBJECT::Timer starts at0; preserve float32 accumulation from MoveObject.
export function stepMapEmitter(e){if(!e.kinds.includes('bubble'))return true;e.timer=Math.fround(Math.fround(e.timer??0)+Math.fround(.1));if(e.timer>10)e.timer=0;return e.timer>5;}
export function mapParticleUV(p){if(p.kind!=='bubble')return [0,0,1,1];const frame=p.frame%9;return [frame%3*.25+.005,Math.floor(frame/3)*.25+.005,.24,.24];}

// _define.h MAX_PARTICLES: map-layer total is bounded separately from skill layers.
export const mapParticleLimit=3000;
export function mapParticleCapacity(kind,emitters){const life={boneHik3:38,trueFire0:24,trueBlue0:24,flame12:2800,cloud22:240,cloud21:100,smoke22:60,smoke60:120};if(emitters&&(life[kind]||kind==='smoke'&&emitters.some(e=>[69,70,71,72,73,74].includes(e.map))))return Math.min(mapParticleLimit,emitters.filter(e=>e.kinds.includes(kind)).length*(life[kind]??80));if(kind.startsWith('market'))return emitters?Math.min(mapParticleLimit,emitters.filter(e=>e.kinds.includes(kind)).length*(kind==='market1'?31:kind==='market2'?16:21)):mapParticleLimit;return kind==='bubble'?mapParticleLimit:256;}
