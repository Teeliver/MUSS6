import { establishAccountSession } from './account-session.js';
import { createCharacterCodec } from '../protocol/character-codec.js';

// List and policy are an ordered pair; never publish a partial roster as ready.
export async function establishLobbySession(options,{onPhase=()=>{},onPacket=()=>{},
 timers=globalThis,rosterDeadlineMs=5000,...accountPolicy}={}) {
 if(typeof onPhase!=='function'||typeof onPacket!=='function'||!Number.isSafeInteger(rosterDeadlineMs)||rosterDeadlineMs<=0)
  throw new Error('Invalid lobby policy');
 const codec=createCharacterCodec(options.core);
 let account,pending,phase='connecting',closed=false;
 const detach=request=>{timers.clearTimeout(request.timer);request.signal?.removeEventListener('abort',request.abort);};
 const terminal=()=>{
  if(closed)return;closed=true;phase='closed';
  const request=pending;pending=undefined;
  if(request){detach(request);request.reject(new Error('Lobby session closed'));}
 };
 const close=()=>{terminal();account?.close();};
 const transition=next=>{phase=next;onPhase({phase:next});};
 account=await establishAccountSession(options,{...accountPolicy,timers,
  onPhase:event=>{if(event.phase==='closed')terminal();else phase=event.phase;onPhase(event);},
  onPacket:packet=>{
   if(closed)return;
   if(!pending){onPacket(packet);return;}
   try{
    if(phase==='awaiting-roster'){
     pending.roster=codec.roster(packet);transition('awaiting-roster-policy');return;
    }
    if(phase!=='awaiting-roster-policy')throw new Error('Invalid lobby phase');
    const policy=codec.policy(packet),request=pending;
    // Policy classCode is authoritative for creation, raw roster metadata remains available.
    const result={...request.roster,policy};
    transition('roster-ready');
    if(closed)return;
    pending=undefined;detach(request);request.resolve(result);
   }catch{close();throw new Error('Invalid lobby response');}
  },
 });
 if(closed){account.close();throw new Error('Lobby session closed');}
 return {
  get phase(){return phase;},get state(){return account.state;},greeting:account.greeting,
  login:input=>account.login(input),
  characters({signal}={}){
   if(closed||account.phase!=='authenticated'||pending)return Promise.reject(new Error('Character list is not available'));
   if(signal?.aborted)return Promise.reject(new Error('Character list cancelled'));
   return new Promise((resolve,reject)=>{
    const request={resolve,reject,signal,abort:()=>close()};pending=request;
    try{
     request.timer=timers.setTimeout(()=>close(),rosterDeadlineMs);
     signal?.addEventListener('abort',request.abort,{once:true});
     transition('awaiting-roster');
     if(!closed)account.send(codec.listRequest());
    }catch{close();}
   });
  },
  send(packet){if(closed||pending)throw new Error('Lobby request is pending or closed');account.send(packet);},
  close,
 };
}
