// No cache or automatic downloads. Only bytes matching a provisioned manifest are returned.
export function createResourceLoader({manifest,fetchResource=globalThis.fetch,crypto=globalThis.crypto,
 timers=globalThis,deadlineMs=15000,maxConcurrent=2,maxReservedBytes=67108864}={}){
 if(typeof manifest?.resolve!=='function'||typeof fetchResource!=='function'||typeof crypto?.subtle?.digest!=='function'||
  !Number.isSafeInteger(deadlineMs)||deadlineMs<=0||!Number.isSafeInteger(maxConcurrent)||maxConcurrent<1||maxConcurrent>8||
  !Number.isSafeInteger(maxReservedBytes)||maxReservedBytes<1||maxReservedBytes>134217728)throw new Error('Invalid asset loader configuration');
 let active=0,reserved=0;
 return Object.freeze({
  get active(){return active;},get reservedBytes(){return reserved;},
  async load(canonical,{signal,onProgress=()=>{}}={}){
   if(signal?.aborted)throw new Error('Asset load cancelled');
   if(typeof onProgress!=='function')throw new Error('Invalid asset progress callback');
   const entry=manifest.resolve(canonical);
   if(!Number.isSafeInteger(entry.size)||entry.size<1||entry.size>67108864||!/^[a-f0-9]{64}$/.test(entry.sha256))throw new Error('Invalid asset metadata');
   if(active>=maxConcurrent||reserved+entry.size>maxReservedBytes)throw new Error('Asset load budget exceeded');
   active++;reserved+=entry.size;
   const controller=new AbortController();let reader,timer,abortListener,output,complete=false;
   const abort=()=>controller.abort();
   const aborted=new Promise((_,reject)=>{abortListener=()=>reject(new Error('Asset load cancelled or timed out'));controller.signal.addEventListener('abort',abortListener,{once:true});});
   const wait=promise=>Promise.race([promise,aborted]);
   try{
    signal?.addEventListener('abort',abort,{once:true});
    timer=timers.setTimeout(abort,deadlineMs);
    if(signal?.aborted)abort();
    const response=await wait(fetchResource(entry.url,{signal:controller.signal,redirect:'error',credentials:'omit',cache:'no-store'}));
    if(response.status!==200||response.redirected||!response.body?.getReader)throw new Error('Asset response rejected');
    if(response.url&&response.url!==entry.url)throw new Error('Asset URL changed');
    const length=response.headers.get('content-length'),encoding=response.headers.get('content-encoding');
    // Fetch streams decoded bytes; compressed Content-Length is not decoded length.
    if(length!==null&&(!encoding||encoding==='identity')&&(!/^\d+$/.test(length)||Number(length)!==entry.size))throw new Error('Asset length header mismatch');
    reader=response.body.getReader();output=new Uint8Array(entry.size);let loaded=0;
    for(;;){
     const {done,value}=await wait(reader.read());if(done)break;
     if(!(value instanceof Uint8Array)||loaded+value.byteLength>entry.size)throw new Error('Asset stream exceeds manifest size');
     output.set(value,loaded);loaded+=value.byteLength;
     onProgress(Object.freeze({loaded,total:entry.size}));
     if(controller.signal.aborted)throw new Error('Asset load cancelled');
    }
    if(loaded!==entry.size)throw new Error('Asset stream truncated');
    const digest=await wait(crypto.subtle.digest('SHA-256',output));
    if(controller.signal.aborted)throw new Error('Asset load cancelled');
    const hash=Array.from(new Uint8Array(digest),b=>b.toString(16).padStart(2,'0')).join('');
    if(hash!==entry.sha256)throw new Error('Asset checksum mismatch');
    complete=true;return output;
   }finally{
    timers.clearTimeout(timer);signal?.removeEventListener('abort',abort);
    controller.signal.removeEventListener('abort',abortListener);
    // Abort the request even if a fetch implementation does not honor reader cancellation.
    controller.abort();
    if(!complete){output?.fill(0);try{Promise.resolve(reader?.cancel()).catch(()=>{});}catch{/* cleanup only */}}
    try{reader?.releaseLock();}catch{/* an ignored pending read may still exist in a test/platform implementation */}
    active--;reserved-=entry.size;
   }
  },
 });
}
