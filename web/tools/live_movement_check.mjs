// Explicit local integration check: only the existing isolated muverify/MacGuiDW.
import {readFile,stat} from 'node:fs/promises';
import {setTimeout as delay} from 'node:timers/promises';
import WebSocket from '../gateway/node_modules/ws/wrapper.mjs';
import createCore from '../.build/wasm/frame-reader.mjs';
import {establishLobbySession} from '../client/src/session/lobby-session.js';
import {createWorldSession} from '../client/src/session/world-session.js';
import {decodeAppearance} from '../client/src/protocol/appearance-codec.js';
import {terrainData} from '../client/src/world/terrain.js';
import {createStepMotion} from '../client/src/world/step-motion.js';
import path from 'node:path';
const [credentials,data]=process.argv.slice(2);
if(!credentials||!data)throw new Error('Usage: node web/tools/live_movement_check.mjs PRIVATE_CREDENTIALS DATA_DIRECTORY');
const info=await stat(credentials);if(info.uid!==process.getuid()||(info.mode&0o077))throw new Error('Credential file must be owner-only');
const [account,password]=String(await readFile(credentials)).trimEnd().split(/\r?\n/);if(account!=='muverify')throw new Error('Only isolated test account allowed');
const core=await createCore({wasmBinary:await readFile(new URL('../.build/wasm/frame-reader.wasm',import.meta.url))});
const profile=JSON.parse(await readFile(new URL('../dist/debug/runtime/live-profile.json',import.meta.url)));
const enc=new Uint8Array(await readFile(path.join(data,'Enc1.dat'))),dec=new Uint8Array(await readFile(path.join(data,'Dec2.dat')));
let lobby,world,failed=false,stage='connect';
try{
 world=createWorldSession({onError:()=>{failed=true;}});
 lobby=await establishLobbySession({core,profile,encryptionKeyFile:enc,decryptionKeyFile:dec,createSocket:url=>new WebSocket(url,{origin:'http://127.0.0.1:8080'}),gatewayUrl:'ws://127.0.0.1:9000/',routes:{0:{id:'game-0',address:'127.0.0.1',port:55900}},selected:{code:0,endpoint:{address:'127.0.0.1',port:55900}}},{signal:AbortSignal.timeout(15000),identityProvider:async()=>{const response=await fetch('http://127.0.0.1:9001/identity',{headers:{Origin:'http://127.0.0.1:8080'}});if(!response.ok)throw new Error('Identity helper unavailable');return (await response.json()).identity;},onPacket:packet=>world.receive(packet),onPhase:({phase})=>{if(phase!=='closed')stage=phase;if(phase==='closed')world?.close();}});
 stage='login';world.attach(lobby);
 const login=await lobby.login({account,password,tick:Math.floor(performance.now())>>>0,version:Uint8Array.from(profile.version),serial:profile.serial});
 if(!login.accepted){console.error('Login result code',login.code);throw new Error('Isolated account login rejected; no retry');}
 world.authenticated();const roster=await lobby.characters();stage='find-isolated-character';const row=roster.characters.find(row=>row.name==='MacGuiDW');if(!row)throw new Error('Isolated test character unavailable');
 stage='world-entry';const entry=await world.select(row,decodeAppearance(core,row.charset).baseClass);
 if(entry.map!==0)throw new Error('Test requires Lorencia');
 stage='load-terrain';const dir=path.join(data,'World1'),terrain=terrainData(0,new Uint8Array(await readFile(path.join(dir,'TerrainHeight.OZB'))),new Uint8Array(await readFile(path.join(dir,'EncTerrain1.map'))),new Uint8Array(await readFile(path.join(dir,'EncTerrain1.att'))));
 stage='safezone-check';const motion=createStepMotion(entry,terrain);if(!(terrain.flags[entry.y*256+entry.x]&1))throw new Error('Test requires safe zone');
 const direction=[1,3,5,7].find(d=>motion.canStep(d)&&!!(terrain.flags[(entry.y+[0,-1,0,0,0,1,0,0][d])*256+entry.x+[0,0,0,1,0,0,0,-1][d]]&1));
 if(direction===undefined)throw new Error('No adjacent safe walkable tile');
 stage='movement-ack';await delay(800);const first=await world.move([direction]);await delay(800);const back=await world.move([(direction+4)%8]);await delay(800);
 if(failed||back.x!==entry.x||back.y!==entry.y||first.x===entry.x&&first.y===entry.y)throw new Error('Live movement not confirmed');
 console.log(`PASS JS→gateway→GameServer: ${entry.x},${entry.y} → ${first.x},${first.y} → ${back.x},${back.y}; own-index ACK; starting tile restored. Browser visual/save-after-relogin not tested.`);
}catch(error){const location=String(error.stack).match(/live_movement_check\.mjs:(\d+):/);if(location)console.error('Check line',location[1]);console.error(`Live movement check failed at ${stage}; no credentials or packet data logged.`);process.exitCode=1;}
finally{enc.fill(0);dec.fill(0);world?.close();lobby?.close();}
