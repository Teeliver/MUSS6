import { createGameChannel } from './game-channel.js';
import { parseGameGreeting } from '../protocol/game-profile.js';
import { prepareIdentityPacket } from '../protocol/client-identity.js';

// Resolves after identity is sent, NOT after server verification/authentication.
export function establishGameIdentity(options,{identityProvider,signal,onPhase=()=>{},onPacket=()=>{},
 timers=globalThis,phaseDeadlineMs=5000}={}) {
 if(typeof identityProvider!=='function'||typeof onPhase!=='function'||typeof onPacket!=='function')
  return Promise.reject(new Error('Explicit identity provider and callbacks required'));
 if(!Number.isSafeInteger(phaseDeadlineMs)||phaseDeadlineMs<=0)
  return Promise.reject(new Error('Invalid handshake deadline'));
 const expectedCode=options.selected.code;
 return new Promise((resolve,reject)=>{
  let channel,phase='connecting',deadline,ended=false,completed=false;
  const providerAbort=new AbortController();
  const cleanup=()=>{timers.clearTimeout(deadline);deadline=undefined;signal?.removeEventListener('abort',abort);providerAbort.abort();};
  const stop=reason=>{
   if(ended)return;ended=true;cleanup();channel?.close();
   try{onPhase({phase:'closed',reason});}catch{/* observer cannot prevent cleanup */}
   if(!completed)reject(new Error(reason));
  };
  const abort=()=>stop('Game identity session cancelled');
  const enter=next=>{
   if(ended)return false;
   phase=next;timers.clearTimeout(deadline);
   if(next!=='identity-sent')deadline=timers.setTimeout(()=>stop('Game identity phase timeout'),phaseDeadlineMs);
   try{onPhase({phase:next});return !ended;}catch{stop('Game phase callback failed');return false;}
  };
  const receive=packet=>{
   if(ended)return;
   if(phase==='identity-sent'){onPacket(packet);return;}
   if(phase!=='awaiting-greeting'){stop('Unexpected packet during identity handshake');return;}
   let greeting;
   try{greeting=parseGameGreeting(options.core,packet,expectedCode);}catch{stop('Game greeting rejected');return;}
   if(!enter('awaiting-identity'))return;
   Promise.resolve().then(()=>{
    if(ended)return;
    return prepareIdentityPacket(options.core,()=>identityProvider({signal:providerAbort.signal}));
   }).then(identity=>{
    if(ended){identity?.fill(0);return;}
    try{
     channel.send(identity);
     if(!enter('identity-sent'))return;
     completed=true;
     resolve({greeting,get phase(){return ended?'closed':phase;},get state(){return channel.state;},
      send:packet=>channel.send(packet),close:()=>stop('local-close')});
    }catch{stop('Could not send game identity');}
    finally{identity?.fill(0);}
   }).catch(()=>{if(!ended)stop('Identity provider failed');});
  };
  if(signal?.aborted){stop('Game identity session cancelled');return;}
  try{
   channel=createGameChannel({...options,timers,onPacket:receive,onState:({state})=>{
    if(ended)return;
    if(state==='open')enter('awaiting-greeting');
    else if(state==='closed'||state==='failed')stop('Game identity transport ended');
   }});
   signal?.addEventListener('abort',abort,{once:true});
   if(enter('connecting'))channel.connect();
  }catch{stop('Could not start game identity session');}
 });
}
