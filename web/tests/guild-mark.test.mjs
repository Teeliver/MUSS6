import test from 'node:test';import assert from 'node:assert/strict';
import {guildMarkCanvas,guildMarkPalette,guildMarkEditColor} from '../client/src/ui/guild-mark.js';
function fakeCanvas(){const calls=[],ctx={fillStyle:'',fillRect(x,y,w,h){calls.push({x,y,w,h,fillStyle:ctx.fillStyle});}},canvas={width:0,height:0,style:{},getContext:()=>ctx};return {document:{createElement:()=>canvas},calls,canvas};}
test('guild mark palette matches CreateGuildMark order and editor index 0',()=>{
 assert.deepEqual(guildMarkPalette,['rgba(0,0,0,0)','#000000','#808080','#ffffff','#ff0000','#ff8000','#ffff00','#80ff00','#00ff00','#00ff80','#00ffff','#0080ff','#0000ff','#8000ff','#ff00ff','#ff0080']);
 assert.equal(guildMarkEditColor(0),'#808080');assert.equal(guildMarkEditColor(4),'#ff0000');assert.equal(guildMarkEditColor(12),'#0000ff');
});
test('guild mark canvas expands 64 nibbles row-major with red at index 4',()=>{
 const mark=new Uint8Array(32);mark[0]=0x44;mark[5]=0x08;
 const {document,calls,canvas}=fakeCanvas(),c=guildMarkCanvas([...mark],16,{document});
 assert.equal(c,canvas);assert.equal(canvas.width,16);assert.equal(calls.length,64);
 assert.deepEqual(calls[0],{x:0,y:0,w:2,h:2,fillStyle:'#ff0000'});
 assert.deepEqual(calls[1],{x:2,y:0,w:2,h:2,fillStyle:'#ff0000'});
 assert.deepEqual(calls[11],{x:6,y:2,w:2,h:2,fillStyle:'#00ff00'});
 assert.deepEqual(calls[63],{x:14,y:14,w:2,h:2,fillStyle:'rgba(0,0,0,0)'});
});
test('guild mark canvas does not crop cells at small or odd sizes',()=>{
 const small=fakeCanvas();guildMarkCanvas([...new Uint8Array(32)],18,{document:small.document});
 assert.deepEqual(small.calls[63],{x:16,y:16,w:2,h:2,fillStyle:'rgba(0,0,0,0)'});
 const odd=fakeCanvas();guildMarkCanvas([...new Uint8Array(32)],39,{document:odd.document});
 assert.equal(odd.calls[63].x+odd.calls[63].w,39);assert.equal(odd.calls[63].y+odd.calls[63].h,39);
});
import {guildMarkTexture} from '../client/src/ui/guild-mark.js';
import {guildMarkQuad,createGuildMarkMesh,applyGuildMarkMesh,GUILD_MARK_BONE,GUILD_MARK_OFFSET,GUILD_MARK_WIDTH,GUILD_MARK_HEIGHT} from '../client/src/world/guild-mark-plane.js';
const close=(a,b)=>Math.abs(a-b)<1e-6;
test('guild mark texture keeps nibble order and source palette',()=>{
 const mark=new Uint8Array(32);mark[0]=0x44;mark[5]=0x08;
 const texture=guildMarkTexture([...mark]);
 assert.equal(texture.width,8);assert.equal(texture.height,8);
 assert.deepEqual([...texture.pixels.slice(0,8)],[255,0,0,255,255,0,0,255]);
 assert.deepEqual([...texture.pixels.slice(8,16)],[0,0,0,0,0,0,0,0]);
 assert.deepEqual([...texture.pixels.slice(44,48)],[0,255,0,255]);
});
test('guild plane uses the RenderGuild matrix, offset and 5x7 plane',()=>{
 assert.equal(GUILD_MARK_BONE,26);assert.deepEqual(GUILD_MARK_OFFSET,[20,-5,-10]);assert.equal(GUILD_MARK_WIDTH,5);assert.equal(GUILD_MARK_HEIGHT,7);
 const bone=[1,0,0,0,0,1,0,0,0,0,1,0,0,0,110,1],quad=guildMarkQuad(bone,{angle:0,x:10,y:20,ground:5});
 assert.equal(quad.length,4);
 const center=quad.reduce((sum,p)=>sum.map((v,i)=>v+p[i]),[0,0,0]).map(v=>v/4);
 assert.ok(close(center[0],10.2)&&close(center[1],19.95)&&close(center[2],6),JSON.stringify(center));
 assert.ok(close(Math.hypot(quad[1][0]-quad[0][0],quad[1][1]-quad[0][1],quad[1][2]-quad[0][2]),10*Math.SQRT2*.01),JSON.stringify(quad));
 assert.ok(close(Math.hypot(quad[2][0]-quad[1][0],quad[2][1]-quad[1][1],quad[2][2]-quad[1][2]),14*.01),JSON.stringify(quad));
 assert.notDeepEqual(quad[0],quad[1]);assert.notDeepEqual(quad[1],quad[2]);
 const u=quad[1].map((v,i)=>v-quad[0][i]),v=quad[2].map((value,i)=>value-quad[0][i]),n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]];
 assert.ok(Math.abs(Math.hypot(...n)-10*Math.SQRT2*14*.0001)<1e-9,JSON.stringify(n));
});
test('Player.bmd bone 26 is the right upper arm used by RenderGuild',async()=>{
 const {core}=await import('./helpers/game-fixture.mjs'),{readFile}=await import('node:fs/promises');
 const result=core.boneNamesResult(new Uint8Array(await readFile('/Users/angelale/Downloads/MuClientEX603/Data/Player/Player.bmd')));
 assert.equal(result.ok,true,String(result.error));assert.equal(result.names[GUILD_MARK_BONE],'Bip01 R UpperArm');assert.equal(result.names[35],'Bip01 L UpperArm');assert.equal(result.names[36],'Bip01 L Forearm');
});
test('guild mark mesh writes six transformed vertices and unit normals',()=>{
 const mesh=createGuildMarkMesh([...new Uint8Array(32).fill(0x44)]);
 assert.equal(mesh.positions.length,18);assert.equal(mesh.uv.length,12);assert.equal(mesh.castsShadow,false);
 assert.deepEqual([...mesh.uv],[0,0,1,0,1,1,0,0,1,1,0,1]);
 const bone=[1,0,0,0,0,1,0,0,0,0,1,0,0,0,110,1];
 applyGuildMarkMesh(mesh,bone,{angle:Math.PI/2,x:10,y:20,ground:5});
 assert.ok(mesh.positions.every(Number.isFinite));
 assert.ok(close(Math.hypot(mesh.normals[0],mesh.normals[1],mesh.normals[2]),1));
 const expected=guildMarkQuad(bone,{angle:Math.PI/2,x:10,y:20,ground:5})[0];assert.ok([...mesh.positions.slice(0,3)].every((value,index)=>close(value,expected[index])));
 assert.ok([...mesh.positions.slice(9,12)].every((value,index)=>close(value,mesh.positions[index])));
});
import {playerSignature} from '../client/src/world/player-layer.js';
test('remote player signature includes guild mark so the mesh rebuilds',()=>{
 const base={charset:Array.from({length:18},(_,i)=>i),equipment:[]},a=playerSignature({...base,guildMark:[...new Uint8Array(32)]}),b=playerSignature({...base,guildMark:[...new Uint8Array(32).fill(0x44)]});
 assert.notEqual(a,b);assert.equal(a,playerSignature({...base,guildMark:[...new Uint8Array(32)]}));
});
import {createWorldSession} from '../client/src/session/world-session.js';
const guildViewportPacket=entries=>{const b=new Uint8Array(5+entries.length*12);b.set([0xc2,0,b.length,0x65,entries.length]);const d=new DataView(b.buffer);entries.forEach((entry,i)=>{const p=5+i*12;d.setUint32(p,entry.key,true);b[p+4]=entry.status??0;b[p+5]=entry.type??0;b[p+6]=0;b[p+7]=(entry.index>>8)&0x7f;b[p+8]=entry.index&0xff;b[p+9]=0;});return b;};
const guildInfoPacket=(key,mark)=>{const b=new Uint8Array(60);b.set([0xc1,60,0x66,0]);new DataView(b.buffer).setUint32(4,key,true);b[8]=0;b.set(mark,25);return b;};
test('remote guild marks are cached, retried after failure and refreshed after TTL',()=>{
 let clock=1000;const sent=[],entities=[];const world=createWorldSession({now:()=>clock,timers:{setTimeout:()=>1,clearTimeout(){},clearInterval(){}},onEntities:value=>entities.push(value)});
 world.attach({greeting:{index:0},send:bytes=>sent.push(bytes),close(){}});
 world.receive(Uint8Array.of(194,0,15,19,1,0,2,0,2,101,100,101,100,16,0));
 const requests=()=>sent.filter(bytes=>bytes[2]===0x66).length;
 world.receive(guildViewportPacket([{key:77,index:2}]));
 assert.equal(requests(),1,'unknown guild mark is requested once');
 clock+=1000;world.receive(guildViewportPacket([{key:77,index:2}]));
 assert.equal(requests(),1,'in-flight request is not repeated');
 clock+=15000;world.receive(guildViewportPacket([{key:77,index:2}]));
 assert.equal(requests(),2,'failed request retries after the retry window');
 const mark=new Uint8Array(32).fill(0x44);
 world.receive(guildInfoPacket(77,mark));
 assert.deepEqual(entities.at(-1)[0].guildMark,[...mark],'mark is attached to the entity');
 clock+=1000;world.receive(guildViewportPacket([{key:77,index:2}]));
 assert.equal(requests(),2,'fresh cached mark is not requested again');
 clock+=60000;world.receive(guildViewportPacket([{key:77,index:2}]));
 assert.equal(requests(),3,'cached mark refreshes after the TTL so edits appear');
 world.close();
});
