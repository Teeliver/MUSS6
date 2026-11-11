const checked=result=>{if(!result.ok)throw new Error(result.error);return result;};
export function createConnectCodec(core) {
 const invoke=(operation,input=new Uint8Array(),code=0)=>checked(core.connectCodecResult(operation,input,code));
 return {
  handshake:bytes=>{invoke('handshake',bytes);},
  listRequest:()=>invoke('list-request').bytes,
  endpointRequest:code=>{
   if(!Number.isInteger(code)||code<0||code>255)throw new RangeError('Unsupported selection code');
   return invoke('endpoint-request',new Uint8Array(),code).bytes;
  },
  list:bytes=>invoke('list',bytes).servers,
  endpoint:bytes=>invoke('endpoint',bytes).endpoint,
 };
}
export function createTransportCodec(core,keys) {
 const {key1,key2,mhp,mhp1,mhp2}=keys;
 if(typeof mhp!=='boolean'||![key1,key2,mhp1,mhp2].every(n=>Number.isInteger(n)&&n>=0&&n<=255))throw new TypeError('Explicit byte keys are required');
 const transform=(bytes,encode)=>checked(core.transformTransportResult(bytes,key1,key2,mhp,mhp1,mhp2,encode)).bytes;
 return {encodeTransport:bytes=>transform(bytes,true),decodeTransport:bytes=>transform(bytes,false)};
}

// ConnectServerProtocol.h PMSG_SERVER_NAME_LIST_SEND + PMSG_SERVER_NAME (WORD + char[32]).
export function serverNames(frame){
 if(!(frame instanceof Uint8Array)||frame.length<6||frame.length>2048||frame[0]!==0xc2||frame[1]*256+frame[2]!==frame.length||frame[3]!==0xf3||frame[4]!==0xea||frame.length!==6+frame[5]*34)throw new Error('Invalid server name list');
 const names=new Map();for(let i=0;i<frame[5];i++){const offset=6+i*34,code=frame[offset]+256*frame[offset+1],bytes=frame.subarray(offset+2,offset+34),end=bytes.indexOf(0);if(names.has(code))throw new Error('Duplicate server name');names.set(code,String.fromCharCode(...bytes.subarray(0,end<0?32:end)));}return names;
}
