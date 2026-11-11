import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';
// Decodes the two original client tables used by the item tooltip:
// - SocketItem_eng.bmd: 3 option types x 50 records, sizeof(SOCKET_OPTION_INFO)=104.
//   Main5.2/source/SocketSystem.cpp::OpenSocketItemScript applies BuxConvert to each record.
// - JewelOfHarmonyOption_eng.bmd: 3 item types x 10 options, sizeof(HARMONYJEWELOPTION)=180.
//   Main5.2/source/UIJewelHarmony.cpp::OpenJewelHarmonyInfoFile applies BuxConvert once.
// - ItemAddOption.bmd: MAX_ITEM=8192 records, sizeof(ITEM_ADD_OPTION)=16 (padding included).
//   Main5.2/source/ItemAddOptioninfo.cpp::OpenItemAddOptionInfoFile applies BuxConvert once.
const root=process.argv[2];if(!root)throw Error('Expected client Data directory containing Local/Eng');
const dir=root.replace(/\/$/,'')+'/Local/Eng/';
const dataDir=root.replace(/\/$/,'')+'/Local/';
const key=[0xfc,0xcf,0xab];
const bux=(buffer,offset=0)=>{const out=Buffer.from(buffer);for(let i=0;i<out.length;i++)out[i]^=key[(i+offset)%3];return out;};
const readCString=(buffer,start,end)=>buffer.subarray(start,end).toString('latin1').replace(/\0.*$/,'');
const socketBytes=await readFile(dir+'SocketItem_eng.bmd');
const socketSize=104,socketTypes=3,socketCount=50;
if(socketBytes.length!==socketSize*socketTypes*socketCount)throw Error('Unexpected SocketItem size '+socketBytes.length);
const socketTable=[];
for(let type=0;type<socketTypes;type++){
 const rows=[];
 for(let index=0;index<socketCount;index++){
  const record=bux(socketBytes.subarray((type*socketCount+index)*socketSize,(type*socketCount+index+1)*socketSize));
  rows.push({
   id:record.readInt32LE(0),
   category:record.readInt32LE(4),
   name:readCString(record,8,72),
   type:record.readUInt8(72),
   values:Array.from({length:5},(_,n)=>record.readInt32LE(76+n*4)),
  });
 }
 socketTable.push(rows);
}
const harmonyBytes=await readFile(dir+'JewelOfHarmonyOption_eng.bmd');
const harmonySize=180,harmonyTypes=3,harmonyCount=10;
if(harmonyBytes.length!==harmonySize*harmonyTypes*harmonyCount)throw Error('Unexpected JewelOfHarmonyOption size '+harmonyBytes.length);
const harmony=bux(harmonyBytes),harmonyTable=[];
for(let type=0;type<harmonyTypes;type++){
 const rows=[];
 for(let index=0;index<harmonyCount;index++){
  const record=harmony.subarray((type*harmonyCount+index)*harmonySize,(type*harmonyCount+index+1)*harmonySize);
  rows.push({
   option:record.readInt32LE(0),
   name:readCString(record,4,64),
   minLevel:record.readInt32LE(64),
   levels:Array.from({length:14},(_,n)=>record.readInt32LE(68+n*4)),
  });
 }
 harmonyTable.push(rows);
}
const socketHash=createHash('sha256').update(socketBytes).digest('hex');
const harmonyHash=createHash('sha256').update(harmonyBytes).digest('hex');
const addOptionBytes=await readFile(dataDir+'ItemAddOption.bmd');
const addOptionSize=16,addOptionCount=8192;
if(addOptionBytes.length!==addOptionSize*addOptionCount)throw Error('Unexpected ItemAddOption size '+addOptionBytes.length);
const addOption=bux(addOptionBytes),addOptions={};
for(let index=0;index<addOptionCount;index++){
 const record=addOption.subarray(index*addOptionSize,(index+1)*addOptionSize);
 const row=[record.readUInt8(0),record.readUInt16LE(2),record.readUInt8(4),record.readUInt16LE(6)];
 if(row.some(Boolean))addOptions[index]=row;
}
const addOptionHash=createHash('sha256').update(addOptionBytes).digest('hex');
const body='// Generated from checksummed original Data/Local/Eng SocketItem + JewelOfHarmonyOption and Local/ItemAddOption (BuxConvert).\n'
 +'export const socketItemHash='+JSON.stringify(socketHash)+';\n'
 +'export const jewelryHarmonyHash='+JSON.stringify(harmonyHash)+';\n'
 +'export const itemAddOptionHash='+JSON.stringify(addOptionHash)+';\n'
 +'export const socketTable='+JSON.stringify(socketTable)+';\n'
 +'export const harmonyTable='+JSON.stringify(harmonyTable)+';\n'
 +'export const itemAddOptions='+JSON.stringify(addOptions)+';\n';
await writeFile(new URL('../client/src/ui/item-extended-data.js',import.meta.url),body);
console.log(`${socketTypes*socketCount} socket records, ${harmonyTypes*harmonyCount} harmony records, ${Object.keys(addOptions).length} add-option records`);
