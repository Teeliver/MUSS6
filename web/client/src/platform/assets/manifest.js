const pathPattern=/^(?:Local\/Eng\/Item_eng\.bmd|(?:Item|Player|Skill|Effect)\/[A-Za-z0-9_-]+\.(?:[bB][mM][dD]|[oO][zZ][jJtT]))$/;
const hashPattern=/^[a-f0-9]{64}$/;
export function createResourceManifest(document,baseUrl){
 const base=new URL(baseUrl);
 if(!['http:','https:'].includes(base.protocol)||base.username||base.password||base.search||base.hash||!base.pathname.endsWith('/'))
  throw new Error('Explicit asset base directory required');
 if(document?.schema!==1||typeof document.version!=='string'||!hashPattern.test(document.version)||
  !Array.isArray(document.entries)||document.entries.length<1||document.entries.length>8192)throw new Error('Invalid resource manifest');
 const entries=new Map(),actual=new Set();
 for(const item of document.entries){
  if(!pathPattern.test(item?.canonical)||!pathPattern.test(item?.path)||
   !Number.isSafeInteger(item.size)||item.size<1||item.size>67108864||typeof item.sha256!=='string'||!hashPattern.test(item.sha256)||
   entries.has(item.canonical)||actual.has(item.path.toLowerCase()))throw new Error('Invalid or duplicate resource entry');
  // Only case corrections are permitted. A manifest cannot silently remap a different model.
  if(item.canonical.toLowerCase()!==item.path.toLowerCase())throw new Error('Resource identity mismatch');
  entries.set(item.canonical,Object.freeze({path:item.path,size:item.size,sha256:item.sha256}));actual.add(item.path.toLowerCase());
 }
 return Object.freeze({version:document.version,count:entries.size,
  resolve(canonical){
   const entry=entries.get(canonical);if(!entry)throw new Error('Resource is outside manifest');
   return Object.freeze({...entry,url:new URL(entry.path,base).href});
  },
 });
}
