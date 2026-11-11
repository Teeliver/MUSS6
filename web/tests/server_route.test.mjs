import test from 'node:test';
import assert from 'node:assert/strict';
import { selectServerRoute } from '../client/src/protocol/server-route.js';
test('server selection only resolves the configured route and matching endpoint',()=>{
 const routes={1:{id:'game-one',address:'127.0.0.1',port:55901}};
 assert.equal(selectServerRoute(1,{address:'127.0.0.1',port:55901},routes),'/mu/game-one');
 for(const [code,endpoint] of [[256,{address:'127.0.0.1',port:55901}],[2,{address:'127.0.0.1',port:55901}],[1,{address:'example.com',port:55901}],[1,{address:'127.0.0.1',port:80}]])assert.throws(()=>selectServerRoute(code,endpoint,routes));
});
