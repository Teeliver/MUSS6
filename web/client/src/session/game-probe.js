import { TransportSession } from '../platform/transport/session.js';
import { createReader } from '../platform/wasm/create-reader.js';
import { createTransportCodec } from '../protocol/connect-codec.js';
import { deriveGameKeys, parseGameGreeting } from '../protocol/game-profile.js';
import { selectServerRoute } from '../protocol/server-route.js';

// Read-only greeting probe. Success is NOT authentication or a persistent game session.
export function createGameProbe({core,createSocket,gatewayUrl,routes,timers=globalThis,deadlineMs=5000}) {
  const gateway=new URL(gatewayUrl);
  if(!['ws:','wss:'].includes(gateway.protocol)||gateway.pathname!=='/'||gateway.search||gateway.hash||gateway.username||gateway.password||
    !Number.isSafeInteger(deadlineMs)||deadlineMs<=0)throw new Error('Invalid game probe configuration');
  const fixedRoutes=Object.fromEntries(Object.entries(routes).map(([code,route])=>[code,Object.freeze({...route})]));
  return async function probe(selected,profile,{signal}={}) {
    const code=selected.code;
    const path=selectServerRoute(code,selected.endpoint,fixedRoutes);
    const url=new URL(path,gateway);
    const codec=createTransportCodec(core,deriveGameKeys(core,profile));
    return new Promise((resolve,reject)=>{
      if(signal?.aborted){reject(new Error('Game probe cancelled'));return;}
      let session,deadline,settled=false;
      const finish=(error,greeting)=>{
        if(settled)return;settled=true;
        timers.clearTimeout(deadline);signal?.removeEventListener('abort',abort);session?.close();
        if(error)reject(error);else resolve(greeting);
      };
      const abort=()=>finish(new Error('Game probe cancelled'));
      try {
        session=new TransportSession({createSocket,createReader:()=>createReader(core),...codec,timers,deadlineMs,
          onState:({state})=>{
            if(settled)return;
            if(state==='open')deadline=timers.setTimeout(()=>finish(new Error('Game greeting timeout')),deadlineMs);
            if(state==='closed'||state==='failed')finish(new Error('Game transport ended before greeting'));
          },
          onFrame:frame=>{
            if(settled)return;
            try{finish(null,parseGameGreeting(core,frame,code));}
            catch{finish(new Error('Game greeting rejected'));}
          },
        });
        signal?.addEventListener('abort',abort,{once:true});session.connect(url.href);
      }catch{finish(new Error('Could not start game probe'));}
    });
  };
}
