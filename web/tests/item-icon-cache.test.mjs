import test from 'node:test';import assert from 'node:assert/strict';
import {createItemIconCache,itemIconKey} from '../client/src/ui/item-icon-cache.js';
const item={index:3585,level:9};
test('failed icon build can recover on next attachment and retains serialized GPU work',async()=>{
 let calls=0;const cache=createItemIconCache({isUsed:()=>false,build:async()=>{if(++calls===1)throw Error('temporary resource failure');return {renderer:{dispose(){}}};}});
 const first=cache.get(item,{});assert.equal(cache.get(item,{}),first);await assert.rejects(first,/temporary/);const retry=cache.get(item,{});assert.notEqual(retry,first);await retry;assert.equal(calls,2);assert.equal(cache.get(item,{}),retry);cache.close();
});
test('EXC/ancient icons have separate cache entries, visible icons are not evicted',async()=>{
 let disposed=0;const visible=new Set([itemIconKey(item)]),cache=createItemIconCache({limit:1,isUsed:key=>visible.has(key),build:async()=>({renderer:{dispose(){disposed++;}}})});
 await cache.get(item,{});await assert.rejects(cache.get({...item,excellent:1},{}),/budget/);visible.clear();await cache.get({...item,excellent:1},{});await Promise.resolve();assert.equal(disposed,1);assert.notEqual(itemIconKey({...item,excellent:1}),itemIconKey({...item,setOption:1}));cache.close();await Promise.resolve();assert.equal(disposed,2);
});
test('closing while a build is pending disposes its completed renderer exactly once',async()=>{
 let finish,disposed=0;const cache=createItemIconCache({isUsed:()=>false,build:()=>new Promise(resolve=>finish=()=>resolve({renderer:{dispose(){disposed++;}}}))});const pending=cache.get(item,{});await new Promise(r=>setImmediate(r));cache.close();cache.close();finish();await pending;await Promise.resolve();assert.equal(disposed,1);await assert.rejects(cache.get(item,{}),/closed/);
});
