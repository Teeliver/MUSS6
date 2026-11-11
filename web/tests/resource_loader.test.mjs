import test from 'node:test';
import assert from 'node:assert/strict';
import {webcrypto} from 'node:crypto';
import {createResourceManifest} from '../client/src/platform/assets/manifest.js';
import {createResourceLoader} from '../client/src/platform/assets/load-resource.js';
const bytes=Uint8Array.of(1,2,3,4),sha256=Buffer.from(await webcrypto.subtle.digest('SHA-256',bytes)).toString('hex');
const manifest=createResourceManifest({schema:1,version:'a'.repeat(64),entries:[{canonical:'Item/Test.bmd',path:'Item/test.bmd',size:4,sha256}]},'http://127.0.0.1:8080/assets/');
function response(chunks,extra={}){return new Response(new ReadableStream({start(controller){for(const chunk of chunks)controller.enqueue(Uint8Array.from(chunk));controller.close();}}),{status:200,...extra});}
const loader=fetchResource=>createResourceLoader({manifest,fetchResource,crypto:webcrypto});
test('asset loader returns verified owned bytes with real byte progress and exact-case URL',async()=>{
 const progress=[];const l=loader(async(url,options)=>{assert.equal(url,'http://127.0.0.1:8080/assets/Item/test.bmd');assert.equal(options.redirect,'error');assert.equal(options.credentials,'omit');return response([[1],[2,3,4]],{headers:{'content-length':'4'}});});
 assert.deepEqual(await l.load('Item/Test.bmd',{onProgress:p=>progress.push(p)}),bytes);
 assert.deepEqual(progress,[{loaded:1,total:4},{loaded:4,total:4}]);assert.equal(l.active,0);assert.equal(l.reservedBytes,0);
 await assert.rejects(l.load('Item/Other.bmd'),/outside manifest/);
});
test('asset loader rejects bad status, length, truncated, oversized and checksum-invalid content',async()=>{
 for(const make of [()=>new Response('bad',{status:404}),()=>response([[1,2,3,4]],{headers:{'content-length':'9'}}),()=>response([[1,2]]),()=>response([[1,2,3,4,5]]),()=>response([[1,2,3,5]])]){
  const l=loader(async()=>make());await assert.rejects(l.load('Item/Test.bmd'));assert.equal(l.active,0);assert.equal(l.reservedBytes,0);
 }
 const compressed=loader(async()=>response([[1,2,3,4]],{headers:{'content-length':'99','content-encoding':'gzip'}}));assert.deepEqual(await compressed.load('Item/Test.bmd'),bytes);
});
test('asset loader deadline and abort release reservations even when fetch or read ignores signal',async()=>{
 for(const stalled of ['fetch','read']){
  const jobs=new Map();let id=0,cancelled=0;
  const fetchResource=stalled==='fetch'?()=>new Promise(()=>{}):async()=>({status:200,headers:new Headers(),body:{getReader:()=>({read:()=>new Promise(()=>{}),cancel(){cancelled++;},releaseLock(){}})}});
  const l=createResourceLoader({manifest,fetchResource,crypto:webcrypto,timers:{setTimeout(fn){jobs.set(++id,fn);return id;},clearTimeout(i){jobs.delete(i);}}});
  const promise=l.load('Item/Test.bmd'),rejected=assert.rejects(promise,/cancelled|timed out/);
  for(let n=0;n<4;n++)await Promise.resolve();[...jobs.values()][0]();await rejected;assert.equal(l.active,0);assert.equal(jobs.size,0);if(stalled==='read')assert.equal(cancelled,1);
 }
 const controller=new AbortController(),l=loader(()=>new Promise(()=>{})),p=l.load('Item/Test.bmd',{signal:controller.signal});const rejected=assert.rejects(p,/cancelled/);controller.abort();await rejected;
 await assert.rejects(l.load('Item/Test.bmd',{signal:controller.signal}),/cancelled/);assert.equal(l.active,0);
});
test('asset loader budget and observer failure clean up without caching rejected bytes',async()=>{
 let release;const l=createResourceLoader({manifest,crypto:webcrypto,maxConcurrent:1,maxReservedBytes:4,fetchResource:()=>new Promise(r=>release=r)});
 const first=l.load('Item/Test.bmd');await assert.rejects(l.load('Item/Test.bmd'),/budget/);assert.equal(l.reservedBytes,4);release(response([[1,2,3,4]]));await first;assert.equal(l.reservedBytes,0);
 let calls=0;const c=loader(async()=>{calls++;return response([[1,2,3,4]]);});await assert.rejects(c.load('Item/Test.bmd',{onProgress(){throw new Error('observer');}}),/observer/);
 assert.deepEqual(await c.load('Item/Test.bmd'),bytes);assert.equal(calls,2);assert.equal(c.active,0);
});
