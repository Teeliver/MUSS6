import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {core} from './helpers/game-fixture.mjs';
import {decodeAttachmentGeometry} from '../client/src/platform/assets/decode-model.js';
test('attachment geometry contract handles hierarchy, pose errors and normal rotation',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'mu-attachment-'));
 try{execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-I',fileURLToPath(new URL('../../client-core/include',import.meta.url)),fileURLToPath(new URL('attachment_geometry_contract.cpp',import.meta.url)),'-o',join(dir,'test')]);assert.match(execFileSync(join(dir,'test'),{encoding:'utf8'}),/attachment geometry contract passed/);}finally{await rm(dir,{recursive:true,force:true});}
});
test('attachment WASM adapter rejects invalid inputs and missing model data',()=>{
 assert.throws(()=>decodeAttachmentGeometry(core,new Uint8Array(),new Uint8Array()));
 assert.throws(()=>decodeAttachmentGeometry(core,[],new Uint8Array()));
 for(const options of [{slot:2},{index:8192},{action:512},{frame:256},{safe:1}])assert.throws(()=>decodeAttachmentGeometry(core,new Uint8Array(),new Uint8Array(),options));
});

test('Satan shoulder attachment reaches the WASM geometry adapter',()=>{
 let received;
 const bridge={attachmentGeometryResult(...args){received=args;return {ok:true,meshes:[]};}};
 assert.deepEqual(decodeAttachmentGeometry(bridge,new Uint8Array([1]),new Uint8Array([2]),{slot:8,index:6657}),{meshes:[]});
 assert.equal(received[4],8);assert.equal(received[5],6657);
});
