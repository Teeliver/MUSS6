import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';
const root=process.argv[2];if(!root)throw Error('Expected client Data directory containing Local/ItemTRSData.bmd');
const file=root.replace(/\/$/,'')+'/Local/ItemTRSData.bmd';
const bytes=await readFile(file);
if(bytes.length<8)throw Error('ItemTRSData too short');
const count=bytes.readUInt32LE(0),size=32;
if(bytes.length!==4+count*size+4)throw Error('Unexpected ItemTRSData size');
const key=[0xfc,0xcf,0xab],seen=new Map();
for(let n=0;n<count;n++){
 const row=Buffer.from(bytes.subarray(4+n*size,4+(n+1)*size));
 for(let i=0;i<size;i++)row[i]^=key[i%3];
 const index=row.readInt32LE(0);
 if(index<0||index>=8192)throw Error('Invalid item index '+index);
 if(seen.has(index))continue; // source std::map::insert keeps the first record
 const value=[];
 for(let i=0;i<7;i++)value.push(row.readFloatLE(4+i*4));
 const [x,y,z,ax,ay,az,scale]=value;
 if(!value.every(Number.isFinite))throw Error('Non-finite TRS for '+index);
 seen.set(index,[+x.toFixed(6),+y.toFixed(6),+z.toFixed(6),+ax.toFixed(4),+ay.toFixed(4),+az.toFixed(4),+scale.toFixed(8)]);
}
const sorted=[...seen.entries()].sort((a,b)=>a[0]-b[0]);
const hash=createHash('sha256').update(bytes).digest('hex');
const body='// Generated from checksummed original Data/Local/ItemTRSData.bmd (ITEM_POSITION, BuxConvert).\nexport const itemTRSHash='+JSON.stringify(hash)+';\nexport const itemTRS={'+sorted.map(([i,v])=>`${i}:[${v.join(',')}]`).join(',')+'};\nexport function itemTRSAt(index){return itemTRS[index];}\n';
await writeFile(new URL('../client/src/ui/item-trs-data.js',import.meta.url),body);
console.log(`${sorted.length} item TRS entries`);