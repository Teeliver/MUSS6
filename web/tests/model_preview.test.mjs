import test from 'node:test';
import assert from 'node:assert/strict';
import {geometryBounds,createModelPreview} from '../client/src/platform/rendering/model-preview.js';
const model=()=>({meshes:[{positions:Float32Array.of(0,0,0,1,0,0,0,1,0),normals:new Float32Array(9),uv:new Float32Array(6),image:{kind:'rgba',width:1,height:1,pixels:new Uint8Array(4)}}]});
function fake(fail=false){
 const counts={buffer:0,vao:0,texture:0,shader:0,program:0,draw:0};
 const gl={COMPILE_STATUS:1,LINK_STATUS:2,NO_ERROR:0,getShaderParameter:()=>true,getProgramParameter:()=>true,getError:()=>0,getUniformLocation:()=>0,
 createShader:()=>({}),createProgram:()=>({}),createBuffer:()=>({}),createVertexArray:()=>({}),createTexture:()=>({}),
 deleteBuffer:()=>counts.buffer++,deleteVertexArray:()=>counts.vao++,deleteTexture:()=>counts.texture++,deleteShader:()=>counts.shader++,deleteProgram:()=>counts.program++,drawArrays:()=>counts.draw++,
 texImage2D(){if(fail)throw new Error('upload');}};
 return {gl:new Proxy(gl,{get(target,key){return key in target?target[key]:()=>{};}}),counts};
}
test('preview bounds and GPU lifetime do not leak on dispose or upload failure',()=>{
 assert.deepEqual(geometryBounds(model().meshes),{center:[.5,.5,0],scale:1.45});
 assert.throws(()=>geometryBounds([]));const invalid=model();invalid.meshes[0].positions[0]=NaN;assert.throws(()=>geometryBounds(invalid.meshes));
 const {gl,counts}=fake(),renderer=createModelPreview(gl,[model()]);renderer.draw(800,600,0);assert.equal(counts.draw,1);renderer.dispose();renderer.dispose();renderer.draw(800,600,0);
 assert.deepEqual(counts,{buffer:3,vao:1,texture:1,shader:2,program:1,draw:1});
 const bad=fake(true);assert.throws(()=>createModelPreview(bad.gl,[model()]),/upload/);assert.equal(bad.counts.buffer,3);assert.equal(bad.counts.texture,1);assert.equal(bad.counts.program,1);
});
test('preview meshes sharing a decoded image upload one GPU texture',()=>{
 const m=model();m.meshes.push({...m.meshes[0]});const {gl,counts}=fake();const renderer=createModelPreview(gl,[m]);renderer.dispose();assert.equal(counts.texture,1);assert.equal(counts.buffer,6);assert.equal(counts.vao,2);
});
test('animation updates existing GPU buffers without uploading textures or allocating VAOs',()=>{
 const m=model(),{gl,counts}=fake(),renderer=createModelPreview(gl,[m]);const next=model();next.meshes[0].positions[0]=.1;renderer.update(0,next);renderer.draw(800,600,0);
 assert.equal(counts.texture,0);assert.equal(counts.buffer,0);assert.equal(counts.vao,0);
 assert.throws(()=>renderer.update(0,{meshes:[]}));next.meshes[0].positions[0]=NaN;assert.throws(()=>renderer.update(0,next));renderer.dispose();assert.equal(counts.buffer,3);assert.equal(counts.texture,1);
});
test('additive effect disables depth writes for that mesh and restores render state',()=>{
 const {gl}=fake(),depth=[],blend=[];gl.depthMask=v=>depth.push(v);gl.blendFunc=(a,b)=>blend.push([a,b]);
 const m=model();m.meshes[0].additive=true;const renderer=createModelPreview(gl,[m]);renderer.draw(800,600,0);assert.deepEqual(depth,[false,true]);assert.equal(blend.length,1);renderer.dispose();
});
test('spirit multiply material disables depth writes, updates brightness, restores state',()=>{
 const {gl}=fake(),blend=[],depth=[],values=[];gl.DST_COLOR=7;gl.ZERO=0;gl.blendFunc=(a,b)=>blend.push([a,b]);gl.depthMask=v=>depth.push(v);gl.getUniformLocation=(_,name)=>name;gl.uniform1f=(name,v)=>values.push([name,v]);
 const m=model();m.meshes[0].multiply=true;m.meshes[0].spiritLight=4.9;
 const renderer=createModelPreview(gl,[m]);const next=model();next.meshes[0].spiritLight=2;next.meshes[0].opacity=.5;renderer.update(0,next);renderer.draw(800,600,0);
 assert.deepEqual(blend,[[7,0]]);assert.deepEqual(depth,[false,true]);assert.ok(values.some(([name,v])=>name==='spiritLight'&&v===2));assert.ok(values.some(([name,v])=>name==='opacity'&&v===.5));renderer.dispose();
});
test('roof pass separates walls, fades with blending and restores depth writes',()=>{
 const {gl,counts}=fake(),depth=[],opacity=[];gl.depthMask=v=>depth.push(v);gl.getUniformLocation=(_,name)=>name;gl.uniform1f=(name,v)=>{if(name==='opacity')opacity.push(v);};
 const m=model();m.meshes.push({...m.meshes[0],roof:true});const renderer=createModelPreview(gl,[m],{world:true});renderer.setRoofOpacity(.4);
 renderer.draw(800,600,0,{pass:'opaque'});assert.equal(counts.draw,1);
 renderer.draw(800,600,0,{clear:false,pass:'roofs'});assert.equal(counts.draw,2);assert.deepEqual(depth,[true,true,false,true]);assert.deepEqual(opacity,[1,.4]);
 renderer.setRoofOpacity(0);renderer.draw(800,600,0,{clear:false,pass:'roofs'});assert.equal(counts.draw,2);renderer.dispose();
});
test('world shadow pass excludes terrain and additive effects, shares stencil coverage and follows roof fade',()=>{
 const {gl,counts}=fake(),depth=[],stencil=[],clears=[];gl.EQUAL=101;gl.KEEP=102;gl.INCR=103;gl.depthMask=v=>depth.push(v);gl.stencilFunc=(...args)=>stencil.push(args);gl.clear=mask=>clears.push(mask);
 const m=model();m.castsShadow=true;m.meshes.push({...m.meshes[0],additive:true},{...m.meshes[0],shadowReceiver:true},{...m.meshes[0],roof:true});
 const renderer=createModelPreview(gl,[m],{world:true,shadowField:{texture:{},entry:[100,100,0],sx:20}});
 renderer.draw(800,600,0,{pass:'terrain'});assert.equal(counts.draw,1);
 renderer.draw(800,600,0,{clear:false,pass:'shadows'});assert.equal(counts.draw,3);assert.deepEqual(stencil,[[101,0,255]]);assert.deepEqual(depth.slice(-3),[false,false,true]);
 renderer.setRoofOpacity(0);renderer.draw(800,600,0,{clear:false,pass:'shadows'});assert.equal(counts.draw,4);assert.equal(clears.length,1);
 renderer.draw(800,600,0,{clear:false,pass:'objects'});assert.equal(counts.draw,6);renderer.dispose();
});
test('original inventory camera preserves XY orthographic aspect and transparent icon background',()=>{const {gl}=fake(),values=[],colors=[];gl.getUniformLocation=(_,name)=>name;gl.uniform1i=(name,value)=>values.push([name,value]);gl.uniform1f=(name,value)=>values.push([name,value]);gl.clearColor=(...v)=>colors.push(v);const m=model();m.bounds={center:[0,0,0],scale:2};m.iconDepth=.02;const renderer=createModelPreview(gl,[m],{transparent:true,inventory:true});renderer.draw(100,200,0);assert.ok(values.some(([k,v])=>k==='inventory'&&v===1));assert.ok(values.some(([k,v])=>k==='scale'&&v===2));assert.ok(values.some(([k,v])=>k==='iconDepth'&&v===.02));assert.deepEqual(colors.at(-1),[0,0,0,0]);renderer.dispose();});
test('animated textures upload once, share frames, advance at40ms and free every GPU texture',()=>{
 const m=model(),first=m.meshes[0].image,second={...first,pixels:new Uint8Array(4).fill(255)};m.meshes[0].image={...first};m.meshes[0].imageFrames=[first,second];m.meshes[0].frameDuration=40;m.meshes.push({...m.meshes[0]});
 const {gl,counts}=fake(),binds=[];let id=0;gl.createTexture=()=>({id:++id});gl.bindTexture=(_,t)=>binds.push(t.id);const r=createModelPreview(gl,[m]);assert.equal(id,2);binds.length=0;
 for(const time of [0,39,40,79,80])r.draw(100,100,time);assert.deepEqual(binds,[1,1,1,1,2,2,2,2,1,1]);assert.equal(id,2);r.dispose();assert.equal(counts.texture,2);
});
test('dynamic atlas UV updates one existing buffer and validates data without texture reallocation',()=>{
 const m=model(),{gl,counts}=fake(),uploads=[];m.meshes[0].dynamicUv=true;gl.bufferSubData=(_,offset,data)=>uploads.push(data);const r=createModelPreview(gl,[m]);const next=model();next.meshes[0].uv[0]=.255;r.update(0,next);assert.equal(uploads[0],next.meshes[0].uv);assert.equal(uploads.length,3);assert.equal(counts.texture,0);next.meshes[0].uv[0]=NaN;assert.throws(()=>r.update(0,next),/animated UV/);next.meshes[0].uv=new Float32Array(2);assert.throws(()=>r.update(0,next),/animated UV/);r.dispose();assert.equal(counts.texture,1);
});
