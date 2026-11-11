import test from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { mkdtemp, rm, readFile, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { transportReference, packetFilterReference } from '../tools/transport-reference.mjs';
test('portable ConnectServer codec and transport transform contract',async()=>{
 const dir=await mkdtemp(path.join(tmpdir(),'mu-connect-'));
 const root=fileURLToPath(new URL('../../',import.meta.url)),binary=path.join(dir,'contract');
 try {
  await writeFile(path.join(dir,'transport_reference.hpp'),transportReference(await readFile(path.join(root,'GameServer/HackCheck.cpp'),'utf8')));
  await writeFile(path.join(dir,'packet_filter_reference.hpp'),packetFilterReference(await readFile(path.join(root,'../Main_EX603/Main/PacketManager.cpp'),'utf8'),await readFile(path.join(root,'GameServer/PacketManager.cpp'),'utf8')));
  execFileSync('clang++',['-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-DMU_COMPARE_SERVER_SOURCE','-I',dir,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/connect_protocol_contract.cpp'),'-o',binary]);
  assert.match(execFileSync(binary,{encoding:'utf8'}),/connect-protocol contract passed/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
