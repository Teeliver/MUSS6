const checked=result=>{if(!result.ok)throw new Error(result.error);return result;};
export function createPacketCipher(core,{encryptionKeyFile,decryptionKeyFile}) {
 if(!(encryptionKeyFile instanceof Uint8Array)||!(decryptionKeyFile instanceof Uint8Array))throw new TypeError('Explicit key-file bytes are required');
 if(encryptionKeyFile.byteLength!==54||decryptionKeyFile.byteLength!==54)throw new Error('EX603 key files must be 54 bytes');
 // Copy before validating, so mutable/shared input cannot change between check and use.
 const enc=encryptionKeyFile.slice(),dec=decryptionKeyFile.slice();
 let cipher;
 try{checked(core.validateKeyFileResult(enc));checked(core.validateKeyFileResult(dec));cipher=new core.PacketCipher(enc,dec);}finally{enc.fill(0);dec.fill(0);}
 const active=()=>{if(!cipher)throw new Error('Packet cipher has been disposed');return cipher;};
 return {
  encode:bytes=>checked(active().encodeResult(bytes)).bytes,
  decode:bytes=>checked(active().decodeResult(bytes)).bytes,
  status:()=>active().status(),
  delete(){if(cipher){cipher.delete();cipher=undefined;}},
 };
}
