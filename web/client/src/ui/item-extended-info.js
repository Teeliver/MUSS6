import {socketTable,harmonyTable,itemAddOptions} from './item-extended-data.js';
// Tooltip rules from Main5.2/source:
// - NewUIItemMng.cpp:58-90 splits harmony option/level and socket seed ids
//   (0xFF ends the socket list, 0xFE is an empty socket, else seed = raw % 50,
//   sphere level = floor(raw / 50) + 1).
// - SocketSystem.cpp:227-300 renders category, option name, value and set bonus.
// - NewUIItemTooltip.cpp:652-700 renders Jewel of Harmony text.
// - NewUIItemTooltip.cpp:761-775 renders period/expired state.
const SOCKET_EMPTY=0xfe,SOCKET_NONE=0xff,SOCKET_MAX=50;
const SOCKET_CATEGORY=7;
const harmonyPercent=(type,option)=>type===2&&option===7;

export function socketValueText(type,value,{level=0,maxHp=0,maxMp=0}={}){
 if(type===2)return '+'+Math.trunc(value)+'%';
 if(type===3)return value>0?'+'+Math.trunc(level/value):'';
 if(type===4)return value>0?'+'+Math.trunc(maxHp/value):'';
 if(type===5)return value>0?'+'+Math.trunc(maxMp/value):'';
 return '+'+Math.trunc(value);
}

export function itemSocketInfo(item,context={}){
 const count=Number.isInteger(item?.socketCount)?item.socketCount:0;
 if(count<=0)return undefined;
 const sockets=[];
 for(let i=0;i<count;i++){
  const raw=item.wire?.[7+i];
  if(raw===undefined||raw===SOCKET_NONE)break;
  if(raw===SOCKET_EMPTY){sockets.push({number:i+1,empty:true});continue;}
  const seed=raw%SOCKET_MAX,sphere=Math.floor(raw/SOCKET_MAX)+1,row=socketTable[0][seed];
  if(!row?.name)continue;
  sockets.push({number:i+1,seed,sphere,category:row.category,name:row.name,valueText:socketValueText(row.type,row.values[sphere-1]??0,context)});
 }
 const setRaw=item.wire?.[6],set=setRaw<SOCKET_MAX?socketTable[1][setRaw]:undefined;
 return {
  sockets,
  set:set?.name?{name:set.name,valueText:socketValueText(set.type,set.values[0]??0,context)}:undefined,
 };
}

export function itemHarmonyInfo(item){
 if(!item)return undefined;
 const number=item.index&511,group=item.index>>9;
 const socketType=(group===0&&number>=26&&number<=28)||(group===2&&(number===16||number===17))||(group===4&&number===23)||(group===5&&number>=30&&number<=32)||(group===6&&number>=17&&number<=20)||(group>=7&&group<=11&&number>=45&&number<=53);
 if(socketType)return undefined;
 const option=(item.wire?.[6]??0)>>4,level=(item.wire?.[6]??0)&15;
 if(option<1)return undefined;
 const type=item.index<5*512?0:item.index<6*512?1:item.index<12*512?2:undefined;
 if(type===undefined)return undefined;
 if(type!==0&&option>8)return undefined;
 const row=harmonyTable[type][option-1];
 if(!row?.name)return undefined;
 return {name:row.name,value:row.levels[level]??0,percent:harmonyPercent(type,option),level};
}

export function itemPeriodState(item){return item?.periodic?(item.expired?'expired':'active'):undefined;}
export function expireTimeText(value){if(!Number.isFinite(value)||value<=0)return undefined;const date=new Date(value*1000),pad=n=>String(n).padStart(2,'0');return `${date.getFullYear()}-${pad(date.getMonth()+1)}-${pad(date.getDate())}  ${pad(date.getHours())}:${pad(date.getMinutes())}`;}
export function itemAddOptionLines(item){
 if(!item?.option380)return [];
 const row=itemAddOptions[item.index];
 if(!row)return [];
 const lines=[];
 for(let i=0;i<4;i+=2){
  const option=row[i],value=row[i+1];
  if(option>=1&&option<=8)lines.push({id:2183+option,value});
 }
 return lines;
}
export function socketCategoryText(text,code){return code>=1&&code<=SOCKET_CATEGORY?text(2640+code-1,''):'';}
