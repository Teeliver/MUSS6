export function fixture(){
 const bytes=[66,77,68,10];
 const name=text=>{const b=new Uint8Array(32);b.set(new TextEncoder().encode(text));bytes.push(...b);};
 const word=n=>bytes.push(n&255,(n>>8)&255);
 const float=n=>{const b=new Uint8Array(4);new DataView(b.buffer).setFloat32(0,n,true);bytes.push(...b);};
 const vec=x=>{float(x);float(0);float(0);};
 name('fixture');word(1);word(1);word(1);word(3);word(1);word(3);word(1);word(0);
 for(let i=0;i<3;i++){word(0);word(0);vec(i);}
 word(0);word(0);vec(2);word(0);word(0);
 for(let i=0;i<3;i++){float(i);float(0);}
 const start=bytes.length;bytes.push(3,0);for(const i of [0,1,2,0])word(i);for(let i=0;i<4;i++)word(0);for(const i of [0,1,2,0])word(i);while(bytes.length<start+64)bytes.push(0);
 name('fixture.tga');word(1);bytes.push(0);bytes.push(0);name('root');word(-1);vec(2);vec(0);return Uint8Array.from(bytes);
}
