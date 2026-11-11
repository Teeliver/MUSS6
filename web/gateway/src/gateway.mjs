import http from 'node:http';
import net from 'node:net';
import {appendFileSync} from 'node:fs';
import { WebSocketServer, WebSocket } from 'ws';

export function createGateway({routes,origins,port=0,maxConnections=16,maxQueueBytes=262144,
  maxPayload=65536,maxBytesPerSecond=262144,connectMs=5000,idleMs=60000,heartbeatMs=10000}={}) {
  const targets=new Map(Object.entries(routes||{}).map(([id,target])=>{
    if(!/^[a-z0-9-]{1,32}$/.test(id)||!target||!['127.0.0.1','::1'].includes(target.host)||
      !Number.isInteger(target.port)||target.port<1||target.port>65535)throw new Error('Invalid loopback route');
    return [id,Object.freeze({host:target.host,port:target.port})];
  }));
  if(!targets.size||!Array.isArray(origins)||!origins.length)throw new Error('Routes and Origins are required');
  const allowedOrigins=new Set(origins);
  for(const origin of allowedOrigins){const u=new URL(origin);if(!['http:','https:'].includes(u.protocol)||u.origin!==origin)throw new Error('Invalid Origin');}
  if(!Number.isInteger(port)||port<0||port>65535)throw new Error('Invalid listen port');
  for(const n of [maxConnections,maxQueueBytes,maxPayload,maxBytesPerSecond,connectMs,idleMs,heartbeatMs])
    if(!Number.isSafeInteger(n)||n<=0)throw new Error('Invalid gateway limit');
  if(maxPayload>maxQueueBytes||maxQueueBytes>262144)throw new Error('Queue limit exceeds supported bounds');
  const server=http.createServer((req,res)=>{res.writeHead(404);res.end();});
  server.headersTimeout=5000;server.requestTimeout=5000;
  const wss=new WebSocketServer({noServer:true,perMessageDeflate:false,maxPayload});
  const active=new Set();let closing=false;
  server.on('upgrade',(req,socket,head)=>{
    const reject=code=>socket.end(`HTTP/1.1 ${code}\r\nConnection: close\r\nContent-Length: 0\r\n\r\n`);
    socket.on('error',()=>{});
    const match=/^\/mu\/([a-z0-9-]{1,32})$/.exec(req.url||'');
    if(closing)return reject('503 Service Unavailable');
    if(req.method!=='GET'||!allowedOrigins.has(req.headers.origin))return reject('403 Forbidden');
    if(!match||!targets.has(match[1]))return reject('404 Not Found');
    if(active.size>=maxConnections)return reject('503 Service Unavailable');
    const target=targets.get(match[1]);
    wss.handleUpgrade(req,socket,head,ws=>attach(ws,target));
  });
  function attach(ws,target){
    const upstream=net.createConnection({...target,highWaterMark:16384});upstream.setNoDelay(true);
    const session={ws,upstream,stop};active.add(session);
    let ended=false,alive=true;
    const counters={incoming:{start:Date.now(),bytes:0},outgoing:{start:Date.now(),bytes:0}};
    const budget=(direction,n)=>{
      const counter=counters[direction],now=Date.now();
      if(now-counter.start>=1000){counter.start=now;counter.bytes=0;}
      counter.bytes+=n;return counter.bytes<=maxBytesPerSecond;
    };
    const connectionTimer=setTimeout(()=>stop(1011,'upstream-timeout'),connectMs);connectionTimer.unref();
    const heartbeat=setInterval(()=>{
      if(!alive)return stop(1001,'heartbeat-timeout');
      alive=false;ws.ping();
    },heartbeatMs);heartbeat.unref();
    function stop(code=1000,reason='closed'){
      if(ended)return;ended=true;try{appendFileSync('/tmp/mu-ws-close.log',Date.now()+' '+code+' '+reason+'\n');}catch{}
      clearTimeout(connectionTimer);clearInterval(heartbeat);upstream.destroy();
      if(ws.readyState===WebSocket.OPEN){
        ws.close(code,reason);const timer=setTimeout(()=>ws.terminate(),1000);timer.unref();ws.once('close',()=>clearTimeout(timer));
      }else if(ws.readyState!==WebSocket.CLOSED)ws.terminate();
    }
    ws.on('pong',()=>{alive=true;});
    ws.on('message',(data,isBinary)=>{
      if(ended)return;
      if(!isBinary)return stop(1003,'binary-required');if(data.length>3&&(data[0]===0xc2&&data[3]===0x3f||data[0]===0xc1&&data[2]===0x3f)){try{appendFileSync('/tmp/mu-pshop-up.log',Buffer.from(data).toString('hex')+'\n');}catch{}}if(data.length>3&&[0xc1,0xc3].includes(data[0])&&((data[2]>=0x36&&data[2]<=0x3d)||(data[2]>=0x40&&data[2]<=0x44))){try{appendFileSync('/tmp/mu-trade-up.log',Buffer.from(data).toString('hex')+'\n');}catch{}}
      if(!budget('incoming',data.length)||upstream.writableLength+data.length>maxQueueBytes)return stop(1009,'upstream-queue-limit');
      if(!upstream.write(data))ws.pause();
    });
    upstream.on('connect',()=>clearTimeout(connectionTimer));
    upstream.on('drain',()=>{if(!ended)ws.resume();});
    upstream.on('data',data=>{
      if(ended)return;
      if(data.length>3&&(data[0]===0xc2&&data[3]===0x3f||data[0]===0xc1&&data[2]===0x3f)){try{appendFileSync('/tmp/mu-pshop-down.log',Buffer.from(data).toString('hex')+'\n');}catch{}}if(data.length>3&&[0xc1,0xc2,0xc3,0xc4].includes(data[0])&&((data[0]===0xc2||data[0]===0xc4?data[3]:data[2])>=0x36&&(data[0]===0xc2||data[0]===0xc4?data[3]:data[2])<=0x3d||(data[0]===0xc2||data[0]===0xc4?data[3]:data[2])>=0x40&&(data[0]===0xc2||data[0]===0xc4?data[3]:data[2])<=0x44)){try{appendFileSync('/tmp/mu-trade-down.log',Buffer.from(data).toString('hex')+'\n');}catch{}}
      if(!budget('outgoing',data.length)||ws.bufferedAmount+data.length>maxQueueBytes)return stop(1009,'browser-queue-limit');
      // Pause TCP while this chunk is pending; preserve order and bound queues.
      upstream.pause();
      ws.send(data,{binary:true,compress:false},error=>{if(error)stop(1011,'send-error');else if(!ended)upstream.resume();});
    });
    upstream.setTimeout(idleMs,()=>stop(1001,'upstream-idle'));
    upstream.on('end',()=>stop(1000,'upstream-ended'));
    upstream.on('close',()=>stop(1000,'upstream-closed'));
    upstream.on('error',()=>stop(1011,'upstream-error'));
    ws.on('error',()=>stop(1011,'websocket-error'));
    ws.on('close',()=>{stop();active.delete(session);});
  }
  return {
    async listen(){
      await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',()=>{server.off('error',reject);resolve();});});
      return server.address();
    },
    get activeConnections(){return active.size;},
    async close(){
      closing=true;
      for(const session of active){session.stop(1001,'gateway-stopping');session.ws.terminate();}
      await new Promise(resolve=>wss.close(resolve));
      await new Promise((resolve,reject)=>server.close(error=>error?reject(error):resolve()));
    },
  };
}
