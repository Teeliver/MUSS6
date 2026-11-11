import test from 'node:test';
import assert from 'node:assert/strict';
import {createAccountCodec} from '../client/src/protocol/account-codec.js';
import {createPacketCipher} from '../client/src/platform/wasm/create-packet-cipher.js';
import {createIdentityCodec} from '../client/src/protocol/client-identity.js';
import {core,key} from './helpers/game-fixture.mjs';
const input={account:'TestUser',password:'Test!123',tick:0xFEDCBA98,version:Uint8Array.of(1,2,3,4,5),serial:'1234567890123456'};
test('login WASM codec validates before coercion and passes C3 crypto pipeline',()=>{
 const codec=createAccountCodec(core),packet=codec.login(input);
 assert.equal(packet.length,59);assert.deepEqual([...packet.slice(34,43)],[0x98,0xBA,0xDC,0xFE,1,2,3,4,5]);
 const cipher=createPacketCipher(core,{encryptionKeyFile:key(3),decryptionKeyFile:key(21846)});
 try{
  const encrypted=cipher.encode(packet);assert.equal(encrypted[0],0xC3);assert.equal(encrypted.length,90);
  const normalized=cipher.decode(encrypted),restored=createIdentityCodec(core).decodePlain(normalized);
  const expected=packet.slice();expected[0]=0xC1;assert.deepEqual(restored,expected);
 }finally{cipher.delete();packet.fill(0);}
 for(const [field,value] of [['tick',-1],['tick',0x100000000],['tick',1.1],['tick',NaN],['account','bad_name'],['password','elevenchars'],['password','has space'],['version',new Uint8Array(4)],['serial','short']])
  assert.throws(()=>codec.login({...input,[field]:value}));
 for(let code=0;code<256;code++)assert.deepEqual(codec.response(Uint8Array.of(0xC1,5,0xF1,1,code)),{code,accepted:code===1});
 assert.throws(()=>codec.response(Uint8Array.of(0xC1,5,0xF1,1)));
 assert.throws(()=>codec.response(Uint8Array.of(0xC3,5,0xF1,1,1)));
});
