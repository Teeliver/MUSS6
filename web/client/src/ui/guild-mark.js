// CreateGuildMark palette (Main5.2/source/ZzzInventory.cpp, MarkColor[16]).
// Byte order in source: (A<<24)|(B<<16)|(G<<8)|R, so case 4 is pure red.
export const guildMarkPalette=['rgba(0,0,0,0)','#000000','#808080','#ffffff','#ff0000','#ff8000','#ffff00','#80ff00','#00ff00','#00ff80','#00ffff','#0080ff','#0000ff','#8000ff','#ff00ff','#ff0080'];
// RenderGuildColor() draws index 0 as an empty slot outline (gray) inside the editor.
export const guildMarkEditColor=index=>index===0?'#808080':guildMarkPalette[index]??'#808080';
export function guildMarkCanvas(mark,size,{document=globalThis.document}={}){
 if(!Array.isArray(mark)||mark.length<32)return undefined;
 const c=document.createElement('canvas');c.width=size;c.height=size;
 const ctx=c.getContext('2d');if(!ctx)return undefined;
 const edge=k=>Math.round(k*size/8);
 for(let i=0;i<64;i++){const value=i%2===0?mark[i>>1]>>4:mark[i>>1]&15,x0=edge(i%8),x1=edge(i%8+1),y0=edge(Math.floor(i/8)),y1=edge(Math.floor(i/8)+1);ctx.fillStyle=guildMarkPalette[value]??guildMarkPalette[0];ctx.fillRect(x0,y0,x1-x0,y1-y0);}
 c.style.cssText=`position:absolute;width:${size}px;height:${size}px;pointer-events:none`;
 return c;
}
// MarkColor[16] as RGBA bytes (CreateGuildMark packs (A<<24)|(B<<16)|(G<<8)|R).
export const guildMarkRGBA=[[0,0,0,0],[0,0,0,255],[128,128,128,255],[255,255,255,255],[255,0,0,255],[255,128,0,255],[255,255,0,255],[128,255,0,255],[0,255,0,255],[0,255,128,255],[0,255,255,255],[0,128,255,255],[0,0,255,255],[128,0,255,255],[255,0,255,255],[255,0,128,255]];
// 8x8 RGBA texture consumed by model-preview ({width,height,pixels}).
export function guildMarkTexture(mark){
 if(!Array.isArray(mark)||mark.length<32)return undefined;
 const pixels=new Uint8Array(8*8*4);
 for(let i=0;i<64;i++){const value=i%2===0?mark[i>>1]>>4:mark[i>>1]&15,offset=i*4;pixels.set(guildMarkRGBA[value]??guildMarkRGBA[0],offset);}
 return {width:8,height:8,pixels};
}
