import { TransportSession } from '../platform/transport/session.js';
import { createReader } from '../platform/wasm/create-reader.js';
import { serverNames, createConnectCodec } from '../protocol/connect-codec.js';
import { selectServerRoute } from '../protocol/server-route.js';

// Every operation owns one CS session. No credential/login or automatic game connection.
export function createDiscovery({core,createSocket,connectUrl,routes={},timers=globalThis,deadlineMs=5000}) {
  const url=new URL(connectUrl);
  if(!/^\/mu\/[a-z0-9-]{1,32}$/.test(url.pathname)||url.search||url.hash||url.username||url.password)
    throw new Error('A configured gateway route URL is required');
  if(!['ws:','wss:'].includes(url.protocol)||!Number.isSafeInteger(deadlineMs)||deadlineMs<=0)
    throw new Error('Invalid discovery configuration');
  const fixedRoutes=Object.fromEntries(Object.entries(routes).map(([code,route])=>[code,Object.freeze({...route})]));
  const codec=createConnectCodec(core);
  function exchange(request,parse,{signal}={}) {
    return new Promise((resolve,reject)=>{
      if(signal?.aborted){reject(new Error('Discovery cancelled'));return;}
      let settled=false,session,deadline,phase='connecting',names=new Map(),namesSeen=false;
      const finish=(error,value)=>{
        if(settled)return;settled=true;
        timers.clearTimeout(deadline);signal?.removeEventListener('abort',abort);
        session?.close();
        if(error)reject(error);else resolve(value);
      };
      const abort=()=>finish(new Error('Discovery cancelled'));
      const arm=()=>{timers.clearTimeout(deadline);deadline=timers.setTimeout(()=>finish(new Error('ConnectServer response timeout')),deadlineMs);};
      try {
        session=new TransportSession({createSocket,createReader:()=>createReader(core),
          // ConnectServer uses plain bytes in the supplied source; game cipher is separate.
          decodeTransport:bytes=>bytes,encodeTransport:bytes=>bytes,timers,deadlineMs,
          onState:({state})=>{
            if(settled)return;
            if(state==='open'){phase='handshake';arm();}
            if(state==='closed'||state==='failed')finish(new Error('ConnectServer session ended before response'));
          },
          onFrame:frame=>{
            if(settled)return;
            try {
              if(phase==='handshake'){
                codec.handshake(frame);phase='response';arm();session.send(request);
              }else if(phase==='response'){
                if(frame[0]===0xc2&&frame[3]===0xf3&&frame[4]===0xea){if(namesSeen)throw new Error('Repeated name list');names=serverNames(frame);namesSeen=true;}
                else {const value=parse(frame);finish(null,Array.isArray(value)?value.map(server=>names.has(server.code)?{...server,name:names.get(server.code)}:server):value);}
              }
              else finish(new Error('Unexpected ConnectServer frame'));
            }catch{finish(new Error('ConnectServer response rejected'));}
          },
        });
        signal?.addEventListener('abort',abort,{once:true});
        session.connect(url.href);
      }catch{finish(new Error('Could not start ConnectServer session'));}
    });
  }
  return {
    async discover(options){return exchange(codec.listRequest(),frame=>codec.list(frame),options);},
    async select(code,options){
      // Reject unlisted codes before any socket is created.
      const route=Object.hasOwn(fixedRoutes,code)?fixedRoutes[code]:undefined;
      if(!route)throw new Error('Server code is not configured');
      const request=codec.endpointRequest(code);
      return exchange(request,frame=>{
        const endpoint=codec.endpoint(frame),routePath=selectServerRoute(code,endpoint,fixedRoutes);
        const gameUrl=new URL(routePath,url);return {code,endpoint,routePath,gameUrl:gameUrl.href};
      },options);
    },
  };
}
