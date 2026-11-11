// Model builds share one WebGL context and must remain serialized. Failed builds
// are retryable; pending builds also release their renderer if the cache closes.
export function itemIconKey(item){return [item.index,item.level,item.excellent??0,item.setOption??0].join(':');}
export function createItemIconCache({build,isUsed,limit=96}){
 const entries=new Map();let chain=Promise.resolve(),closed=false;
 const dispose=promise=>{void promise.then(data=>data.renderer.dispose(),()=>{});};
 return {get(item,meta){if(closed)return Promise.reject(Error('Item icon cache closed'));const key=itemIconKey(item);if(entries.has(key))return entries.get(key);
  if(entries.size>=limit){const unused=[...entries.keys()].find(k=>!isUsed(k));if(unused===undefined)return Promise.reject(Error('Item icon visible model budget'));dispose(entries.get(unused));entries.delete(unused);}
  const next=chain.catch(()=>{}).then(()=>{if(closed)throw Error('Item icon cache closed');return build(item,meta);});chain=next;entries.set(key,next);
  void next.catch(()=>{if(entries.get(key)===next)entries.delete(key);});return next;
 },close(){if(closed)return;closed=true;for(const promise of entries.values())dispose(promise);entries.clear();}};
}
