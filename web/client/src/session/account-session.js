import { establishGameIdentity } from './game-handshake.js';
import { createAccountCodec } from '../protocol/account-codec.js';

// One login attempt per connection. No credential caching or automatic retry.
export async function establishAccountSession(options,{onPhase=()=>{},onPacket=()=>{},
 timers=globalThis,loginDeadlineMs=5000,...identityPolicy}={}) {
 if(typeof onPhase!=='function'||typeof onPacket!=='function'||!Number.isSafeInteger(loginDeadlineMs)||loginDeadlineMs<=0)
  throw new Error('Invalid account session policy');
 const codec=createAccountCodec(options.core);
 let connection,phase='connecting',pending,deadline,attempted=false,closed=false;
 const clear=()=>{timers.clearTimeout(deadline);deadline=undefined;};
 const terminal=()=>{
  if(closed)return;closed=true;phase='closed';clear();
  const request=pending;pending=undefined;request?.reject(new Error('Account session closed'));
 };
 const close=()=>{terminal();connection?.close();};
 const transition=next=>{phase=next;onPhase({phase:next});};
 connection=await establishGameIdentity(options,{...identityPolicy,timers,
  onPhase:event=>{if(event.phase==='closed')terminal();onPhase(event);},
  onPacket:packet=>{
   if(closed)return;
   if(phase==='authenticated'){onPacket(packet);return;}
   if(phase!=='awaiting-login'){close();throw new Error('Unexpected account packet');}
   try{
    const result=codec.response(packet),request=pending;pending=undefined;clear();
    try{transition(result.accepted?'authenticated':'login-rejected');}
    catch{request.reject(new Error('Account phase callback failed'));close();return;}
    if(closed){request.reject(new Error('Account session closed'));return;}
    request.resolve(result);
    if(!result.accepted)close();
   }catch{close();throw new Error('Invalid login response');}
  },
 });
 if(closed){connection.close();throw new Error('Account session closed');}
 try{transition('ready-for-login');}catch{close();throw new Error('Account phase callback failed');}
 if(closed)throw new Error('Account session closed');
 return {
  get phase(){return phase;},get state(){return connection.state;},greeting:connection.greeting,
  login(input){
   if(closed||phase!=='ready-for-login'||attempted)return Promise.reject(new Error('Login is not available on this connection'));
   let packet;
   try{packet=codec.login(input);}catch{return Promise.reject(new Error('Invalid login input'));}
   attempted=true;
   return new Promise((resolve,reject)=>{
    pending={resolve,reject};
    try{
     deadline=timers.setTimeout(()=>close(),loginDeadlineMs);
     transition('awaiting-login');
     if(!closed)connection.send(packet);
    }catch{close();}
    finally{packet.fill(0);}
   });
  },
  send(packet){if(closed||phase!=='authenticated')throw new Error('Account is not authenticated');connection.send(packet);},
  close,
 };
}
