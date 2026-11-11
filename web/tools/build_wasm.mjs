import { execFileSync } from 'node:child_process';
import { mkdir, writeFile, readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { transportReference, packetFilterReference } from './transport-reference.mjs';
import { modulusReference } from './modulus-reference.mjs';
import { accountReference } from './account-reference.mjs';
import { characterReference } from './character-reference.mjs';
import { appearanceReference, companionsReference } from './appearance-reference.mjs';
const root=fileURLToPath(new URL('../../',import.meta.url));
const output=path.join(root,'web/.build/wasm');
await mkdir(output,{recursive:true});
// Use an explicit local config and existing Python/Node; Homebrew's launcher
// hardcodes Python 3.15, which this minimal install intentionally did not add.
const sdk=process.env.EMSCRIPTEN_ROOT || path.join(execFileSync('brew',['--prefix','emscripten'],{encoding:'utf8'}).trim(),'libexec');
const compiler=process.env.PYTHON || 'python3';
const compilerArgs=[path.join(sdk,'em++.py')];
process.env.EM_CACHE=path.join(output,'cache');
process.env.EM_CONFIG=path.join(output,'.emscripten');
await writeFile(process.env.EM_CONFIG,
  `LLVM_ROOT = ${JSON.stringify(path.join(sdk,'llvm/bin'))}\nBINARYEN_ROOT = ${JSON.stringify(path.join(sdk,'binaryen'))}\nNODE_JS = [${JSON.stringify(process.execPath)}]\n`);
const run=(args,options={})=>execFileSync(compiler,[...compilerArgs,...args],options);
const version=run(['--version'],{encoding:'utf8'}).split('\n')[0];
const compilerVersion=run(['-dumpversion'],{encoding:'utf8'}).trim();
if(compilerVersion!=='6.0.13-git') throw new Error(`Use pinned Emscripten 6.0.13, got ${compilerVersion}`);
const flags=['-std=c++17','-O2','-fexceptions','-sDISABLE_EXCEPTION_CATCHING=0',
  '-sALLOW_MEMORY_GROWTH=1','-sMAXIMUM_MEMORY=134217728','-sENVIRONMENT=web,node'];
run([...flags,'-lembind','-sMODULARIZE=1','-sEXPORT_ES6=1',
  '-I',path.join(root,'client-core/include'),path.join(root,'client-core/frame_reader_bindings.cpp'),
  '-o',path.join(output,'frame-reader.mjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),
  path.join(root,'web/tests/frame_reader_contract.cpp'),'-o',path.join(output,'contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'transport_reference.hpp'),transportReference(await readFile(path.join(root,'GameServer/HackCheck.cpp'),'utf8')));
await writeFile(path.join(output,'packet_filter_reference.hpp'),packetFilterReference(await readFile(path.join(root,'../Main_EX603/Main/PacketManager.cpp'),'utf8'),await readFile(path.join(root,'GameServer/PacketManager.cpp'),'utf8')));
run([...flags,'-DMU_COMPARE_SERVER_SOURCE','-I',output,'-I',path.join(root,'client-core/include'),
  path.join(root,'web/tests/connect_protocol_contract.cpp'),'-o',path.join(output,'protocol-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'modulus_reference.hpp'),modulusReference(await readFile(path.join(root,'../Main_EX603/Main/PacketManager.cpp'),'utf8')));
run([...flags,'-DMU_MODULUS_REFERENCE','-I',output,'-I',path.join(root,'client-core/include'),
  path.join(root,'web/tests/simple_modulus_contract.cpp'),'-o',path.join(output,'modulus-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'account_reference.hpp'),accountReference(await readFile(path.join(root,'GameServer/Protocol.h'),'utf8'),await readFile(path.join(root,'GameServer/Util.cpp'),'utf8')));
run([...flags,'-DMU_ACCOUNT_REFERENCE','-I',output,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/account_protocol_contract.cpp'),'-o',path.join(output,'account-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'character_reference.hpp'),characterReference(await readFile(path.join(root,'GameServer/Protocol.h'),'utf8')));
run([...flags,'-DMU_CHARACTER_REFERENCE','-I',output,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/character_protocol_contract.cpp'),'-o',path.join(output,'character-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'appearance_reference.hpp'),appearanceReference(await readFile(path.join(root,'GameServer/DSProtocol.cpp'),'utf8')));
run([...flags,'-DMU_APPEARANCE_REFERENCE','-I',output,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/appearance_contract.cpp'),'-o',path.join(output,'appearance-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'companions_reference.hpp'),companionsReference(await readFile(path.join(root,'GameServer/DSProtocol.cpp'),'utf8')));
run([...flags,'-DMU_COMPANIONS_REFERENCE','-I',output,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/companions_contract.cpp'),'-o',path.join(output,'companions-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/item_resources_contract.cpp'),'-o',path.join(output,'item-resources-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/bmd_model_contract.cpp'),'-o',path.join(output,'bmd-model-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/texture_contract.cpp'),'-o',path.join(output,'texture-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/armor_resources_contract.cpp'),'-o',path.join(output,'armor-resources-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/equipment_grip_contract.cpp'),'-o',path.join(output,'equipment-grip-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/attachment_geometry_contract.cpp'),'-o',path.join(output,'attachment-geometry-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/equipment_actions_contract.cpp'),'-o',path.join(output,'equipment-actions-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/animation_pose_contract.cpp'),'-o',path.join(output,'animation-pose-contract.cjs')],{stdio:'inherit'});
run([...flags,'-I',path.join(root,'client-core/include'),path.join(root,'web/tests/motion_animation_contract.cpp'),'-o',path.join(output,'motion-animation-contract.cjs')],{stdio:'inherit'});
await writeFile(path.join(output,'toolchain.json'),JSON.stringify({version,flags},null,2)+'\n');
console.log(`Built WASM bridge and contract: ${version}`);
