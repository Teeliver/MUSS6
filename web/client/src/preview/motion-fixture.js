// Combat is a timed visual fixture, not a server-authorized attack.
export function fixtureMode(name,seconds){
 if(!Number.isFinite(seconds)||seconds<0)throw new Error('Invalid fixture clock');
 if(name==='cast')return seconds%2<.6?(Math.floor(seconds/2)%2===0?5:4):0;
 if(name==='combat'){const phase=seconds%9.6;return phase<3?0:phase<6?1:phase<9?2:3;}
 return name==='attack'?3:name==='cycle'?Math.floor(seconds/3)%3:name==='walk'?1:name==='run'?2:0;
}
