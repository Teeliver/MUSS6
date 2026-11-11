import test from 'node:test';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
import { drawingSize,gameUiLayout } from '../client/src/platform/rendering/viewport.js';
import { readFile, readdir, stat } from 'node:fs/promises';
import { modes, wasmFiles, outputPath } from '../tools/build-layout.mjs';
import { execFileSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
test('viewport handles Retina and bounded allocation',()=>{
 assert.deepEqual(drawingSize(800,600,2),[1600,1200]);
 assert.ok(drawingSize(10000,8000,3).every(n=>n>0&&n<=4096));
 for(const bad of [0,-1,NaN,Infinity])assert.throws(()=>drawingSize(bad,100,1));
});
for(const mode of ['debug','release'])test(`${mode}: exact build allowlist and mode separation`,{timeout:90000},async()=>{
 const root=new URL('../',import.meta.url);
 execFileSync(process.execPath,['tools/build.mjs',mode],{cwd:root});
 const base=new URL(`dist/${mode}/`,root);
 const paths=await readdir(base,{recursive:true});
 // build.mjs preserves locally staged resource directories across code rebuilds (see its copy loop).
 const preserved=['runtime','preview-assets','audio-assets','ui-assets','map-title-assets'];
 const actual=[];
 for(const p of paths)if((await stat(new URL(p,base))).isFile()&&!preserved.some(dir=>p.startsWith(dir+'/')))actual.push(p);
 const expected=[...modes[mode].map(outputPath),...wasmFiles.map(f=>`generated/${f}`)].sort();
 assert.deepEqual(actual.sort(),expected);
 for(const file of actual)if(/\.(js|mjs)$/.test(file))execFileSync(process.execPath,['--check',fileURLToPath(new URL(file,base))]);
 const digest=createHash('sha256');
 for(const p of modes[mode])digest.update(outputPath(p)).update(await readFile(new URL(`client/${p}`,root)));
 for(const p of wasmFiles)digest.update(`generated/${p}`).update(await readFile(new URL(`.build/wasm/${p}`,root)));
 const version=digest.digest('hex').slice(0,16);
 for(const p of modes[mode]){const built=await readFile(new URL(outputPath(p),base)),source=await readFile(new URL(`client/${p}`,root));if(/\.(?:m?js|html)$/.test(p)){
  const text=built.toString();assert.equal(text.replaceAll('?v='+version,''),source.toString());
  if(/\.m?js$/.test(p))assert.ok(!/(['"])(\.{1,2}\/[^'"?]+\.m?js)\1/.test(text),'all relative JS module references must carry the build revision');
 }else assert.deepEqual(built,source);}
 const versionedHtml=await readFile(new URL('index.html',base),'utf8');assert.ok(versionedHtml.includes('.js?v='+version));
 if(mode==='debug'){
  const html=await readFile(new URL('index.html',base),'utf8');
  assert.ok(!/<h[1-6]|<main|<p[ >]/i.test(html));
  assert.ok(html.includes('id="debug-overlay"'));
 }
 if(mode==='release'){
  assert.ok(!paths.some(p=>p.includes('diagnostics')||p.includes('preview')||p.includes('model-preview')));
  const text=(await Promise.all(actual.filter(p=>!p.endsWith('.wasm')).map(p=>readFile(new URL(p,base),'utf8')))).join('\n');
  for(const marker of ['id="fps"','Ping server:','WASM: đạt','Thông tin Debug'])assert.ok(!text.includes(marker));
 }
});

test('centered HUD caps at 960px and preserves native aspect on wide and narrow viewports',()=>{for(const [w,h] of [[708,899],[1920,1080],[3440,1440],[480,320]]){const l=gameUiLayout(w,h);assert.ok(l.hudWidth<=960&&l.hudWidth<=w*.9+.000001);assert.ok(Math.abs(l.hudWidth/l.hudHeight-640/51)<1e-9);assert.equal(l.left*2+l.hudWidth,w);}assert.deepEqual(gameUiLayout(1920,1080),{panelScale:2.25,hudScale:1.5,hudWidth:960,hudHeight:76.5,left:480});});
