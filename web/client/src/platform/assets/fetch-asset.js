// Retry temporary local-helper congestion; never hide missing/corrupt assets.
export async function fetchAsset(url,options,{request=fetch,pause=ms=>new Promise(resolve=>setTimeout(resolve,ms))}={}){
 for(let attempt=0;;attempt++){
  options.signal?.throwIfAborted();
  let response;
  try{response=await request(url,options);}catch(error){if(options.signal?.aborted||attempt===5)throw error;}
  if(response&&![429,503].includes(response.status))return response;
  if(attempt===5){if(response)return response;throw new Error('Asset fetch exhausted');}
  await response?.body?.cancel();
  const retry=Number(response?.headers.get('Retry-After'));
  await pause(Math.min(5000,Math.max(250,Number.isFinite(retry)&&retry>0?retry*1000:500*2**attempt)));
 }
}
