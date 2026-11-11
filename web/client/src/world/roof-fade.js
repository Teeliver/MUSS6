// Main5.2 ZzzObject.cpp 4335–4365; ZzzInterface.cpp HeroTile; ZzzAI.cpp Alpha.
export function isRoofObject(map,type){return map===0?[125,126].includes(type):map===2&&[81,82,96,98,99].includes(type);}
export function indoorRoofTile(map,tile){return map===0?tile===4:map===2&&(tile===3||tile>=10);}
export function createRoofFade(map,terrain){
 let alpha;
 return {advance(position,dt){
  if(!Number.isFinite(dt)||dt<0)throw new Error('Invalid roof delta');
  const x=Math.max(0,Math.min(255,Math.floor(position.x+.5))),y=Math.max(0,Math.min(255,Math.floor(position.y+.5)));
  const target=indoorRoofTile(map,terrain.layer1[y*256+x])?0:1;
  alpha=alpha===undefined?target:target+(alpha-target)*Math.pow(.9,25*dt);
  return alpha;
 }};
}
