import { TransportSession } from '../platform/transport/session.js';
import { createReader } from '../platform/wasm/create-reader.js';
import { createPacketCipher } from '../platform/wasm/create-packet-cipher.js';
import { createTransportCodec } from '../protocol/connect-codec.js';
import { deriveGameKeys } from '../protocol/game-profile.js';
import { selectServerRoute } from '../protocol/server-route.js';

// Packet channel only: open means WebSocket open, never MU authentication.
export function createGameChannel({core,createSocket,gatewayUrl,routes,selected,profile,
 encryptionKeyFile,decryptionKeyFile,onPacket=()=>{},onState=()=>{},timers=globalThis,deadlineMs=5000}) {
 const gateway=new URL(gatewayUrl);
 if(!['ws:','wss:'].includes(gateway.protocol)||gateway.pathname!=='/'||gateway.search||gateway.hash||gateway.username||gateway.password)
  throw new Error('Invalid game gateway URL');
 if(typeof onPacket!=='function'||typeof onState!=='function')throw new TypeError('Missing channel callbacks');
 const url=new URL(selectServerRoute(selected.code,selected.endpoint,routes),gateway).href;
 const transport=createTransportCodec(core,deriveGameKeys(core,profile));
 // Own snapshots until connect/close; never retain the caller's mutable key buffers.
 if(!(encryptionKeyFile instanceof Uint8Array)||!(decryptionKeyFile instanceof Uint8Array)||encryptionKeyFile.length!==54||decryptionKeyFile.length!==54)
  throw new Error('Explicit EX603 key files required');
 const enc=encryptionKeyFile.slice(),dec=decryptionKeyFile.slice();
 let packet,reader;
 const erase=()=>{enc.fill(0);dec.fill(0);};
 let session;
 try { session=new TransportSession({createSocket,timers,deadlineMs,onState,
  createReader:()=>{
   try {
    packet=createPacketCipher(core,{encryptionKeyFile:enc,decryptionKeyFile:dec});
    reader=createReader(core);
    return {feed:b=>reader.feed(b),pendingSize:()=>reader.pendingSize(),finish:()=>reader.finish(),
     delete(){try{reader?.delete();}finally{reader=undefined;packet?.delete();packet=undefined;}}};
   }catch(error){packet?.delete();packet=undefined;throw error;}finally{erase();}
  },
  encodeTransport:bytes=>transport.encodeTransport(packet.encode(bytes)),
  decodeTransport:bytes=>transport.decodeTransport(bytes),
  onFrame:frame=>onPacket(packet.decode(frame)),
 }); }catch(error){erase();throw error;}
 return {
  get state(){return session.state;},
  connect(){try{session.connect(url);}finally{erase();}},
  send:bytes=>session.send(bytes),
  close(){erase();session.close();},
 };
}
