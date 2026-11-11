export function createCharacterCodec(core) {
 const invoke=(operation,input=new Uint8Array())=>{
  const result=core.characterCodecResult(operation,input);
  if(!result.ok)throw new Error(result.error);
  const {ok,...value}=result;return value;
 };
 return {listRequest:()=>invoke('request').bytes,roster:bytes=>invoke('roster',bytes),policy:bytes=>invoke('policy',bytes)};
}
