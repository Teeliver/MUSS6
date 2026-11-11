const checked=result=>{if(!result.ok)throw new Error(result.error);return result.bytes;};
export function createIdentityCodec(core) {
 return {
  packet(identity){
   if(typeof identity!=='string'||!/^[0-9A-F]{8}(?:-[0-9A-F]{8}){4}$/.test(identity))throw new Error('Invalid identity format');
   return checked(core.identityPacketResult(identity));
  },
  encodePlain:bytes=>checked(core.packetFilterResult(bytes,true)),
  decodePlain:bytes=>checked(core.packetFilterResult(bytes,false)),
 };
}
// No browser-random ID, HWID scraping, persistence, network send, or auto-fallback.
export async function prepareIdentityPacket(core,provider) {
 if(typeof provider!=='function')throw new Error('Identity provider is not configured');
 const identity=await provider();
 return createIdentityCodec(core).packet(identity);
}
