const checked=result=>{if(!result.ok)throw new Error(result.error);return result;};
export function deriveGameKeys(core,{serial,rawMhp1,rawMhp2}) {
  if(typeof serial!=='string'||!/^[!-~]{16}$/.test(serial)||
    ![rawMhp1,rawMhp2].every(n=>Number.isInteger(n)&&n>=0&&n<=255))throw new Error('Invalid transport profile');
  return checked(core.deriveProfileResult(serial,rawMhp1,rawMhp2)).keys;
}
export function parseGameGreeting(core,bytes,expectedCode) {
  if(!Number.isInteger(expectedCode)||expectedCode<0||expectedCode>65535)throw new Error('Invalid expected server code');
  const greeting=checked(core.gameGreetingResult(bytes)).greeting;
  if(greeting.serverCode!==expectedCode)throw new Error('GameServer code does not match selection');
  // Version remains encoded bytes. No invented comparison with a human version string.
  return greeting;
}
