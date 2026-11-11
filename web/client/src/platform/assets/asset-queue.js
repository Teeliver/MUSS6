// Share duplicate asset reads and bound concurrent requests to the local helper.
export function createAssetQueue(loader,{concurrency=3,limit=256}={}){
 if(typeof loader!=='function'||!Number.isInteger(concurrency)||concurrency<1||concurrency>8||!Number.isInteger(limit)||limit<1)throw new Error('Invalid asset queue policy');
 const pending=[],inflight=new Map();let running=0,closed=false;
 const drain=()=>{while(!closed&&running<concurrency&&pending.length){const request=pending.shift();running++;Promise.resolve().then(()=>loader(request.path)).then(request.resolve,request.reject).finally(()=>{running--;drain();});}};
 const load=path=>{if(closed)return Promise.reject(new Error('Asset queue closed'));if(inflight.has(path))return inflight.get(path);if(pending.length>=limit)return Promise.reject(new Error('Asset queue budget exceeded'));const promise=new Promise((resolve,reject)=>{pending.push({path,resolve,reject});drain();}).finally(()=>inflight.delete(path));inflight.set(path,promise);return promise;};
 load.close=()=>{closed=true;for(const request of pending.splice(0))request.reject(new Error('Asset queue closed'));};return load;
}
