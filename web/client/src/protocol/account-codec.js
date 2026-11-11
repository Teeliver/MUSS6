const checked=result=>{if(!result.ok)throw new Error(result.error);return result;};
export function createAccountCodec(core) {
 return {
  login({account,password,tick,version,serial}) {
   // Validate before Embind uint32 coercion or unbounded string copies.
   if(typeof account!=='string'||!/^[a-zA-Z0-9]{1,10}$/.test(account)||
    typeof password!=='string'||!/^[!-~]{1,10}$/.test(password)||
    !Number.isInteger(tick)||tick<0||tick>0xFFFFFFFF||
    !(version instanceof Uint8Array)||version.length!==5||
    typeof serial!=='string'||!/^[!-~]{16}$/.test(serial))throw new Error('Invalid EX603 login input');
   return checked(core.loginPacketResult(account,password,tick,version,serial)).bytes;
  },
  response:bytes=>{const result=checked(core.loginResponseResult(bytes));return {code:result.code,accepted:result.accepted};},
 };
}
