import {npcPriceRules} from './npc-price-rules.js';
import {maximumDurability} from './item-visual-policy.js';
// Server Item.cpp CItem::Value: supported standard goods only; custom prices remain server authoritative.
export function npcPrice(item,meta,{sell=false,tax=0,prices=[],currency=0}={}){
 const custom=prices.find(p=>['index','level','durability','excellent'].every(k=>p[k]===item[k]));if(custom&&!sell&&currency)return {amount:custom.price,currency:['Zen','WCoinC','WCoinP','GoblinPoint','PC Point'][currency]??'Coin '+currency};
 if(!meta)return;const group=item.index>>9,n=item.index&511;let price,potion=false,fixed=false,formula=false;const rule=npcPriceRules.rules.find(([index,level,grade])=>index===item.index&&(level===-1||level===item.level)&&(grade===-1||grade===item.excellent));
 if(meta.buyMoney)price=BigInt(meta.buyMoney);
 else if(rule){const stack=npcPriceRules.stacks.find(([index,level])=>index===item.index&&(level===-1||level===item.level));price=BigInt(rule[3])*BigInt(stack&&stack[2]!==0&&![4*512+7,4*512+15].includes(item.index)?item.durability:1);fixed=true;}
 else if(meta.value){price=BigInt(Math.trunc(meta.value*meta.value*10/12));if(group===14&&n<=8){price*=BigInt((n===3||n===6?2:1)*2**item.level*item.durability);potion=true;}}
 else if(group<=15){formula=true;const plain=(group===12&&((n>=7&&n<=35)||(n>=44&&n!==50&&(n<262||n>265))))||group===13||group===15;let level=meta.level+item.level*3+(group<12&&item.excellent?25:0);
  if(plain)price=BigInt(level*level*level+100);
  else{level+=({5:4,6:10,7:25,8:45,9:65,10:95,11:135,12:185,13:245,14:305,15:365}[item.level]??0);
   const wing=group===12&&(n<=6||(n>=36&&n<=43)||n===50||(n>=262&&n<=265));
   price=wing?BigInt((40+level)*level*level*11+40000000):BigInt(Math.trunc((level+40)*level*level/8)+100);}
  if(group<6&&!meta.twoHand)price=price*80n/100n;
  if(item.skill)price+=price*150n/100n;
  if(item.luck)price+=price*25n/100n;
  const add=[0,60,140,280,560][item.option];if(add===undefined)return;price+=price*BigInt(add)/100n;
  // Source adds +100% per excellent option below group 12, +25% for wings and helpers.
  for(let bit=0;bit<6;bit++)if(item.excellent&(1<<bit))price=group<12?price*2n:price+price*25n/100n;
 }
 else return;
 if(price>2000000000n)price=2000000000n;if(sell){price/=3n;if(formula&&meta.wearSlot>=0&&meta.wearSlot<=11){const max=maximumDurability(item,meta);if(max>0)price-=BigInt(Math.max(0,Math.trunc(Number(price)*.6*(1-item.durability/max))));}}
 const round=potion?10n:price>=1000n?100n:price>=100n?10n:1n;price=price/round*round;if(!sell)price+=price*BigInt(tax??0)/100n;return {amount:Number(price),currency:'Zen'};
}
// ZzzInventory::IsHighValueItem: skill Orbs are NOT all high-value items.
export function highValueItem(i){const g=i.index>>9,n=i.index&511,ancient=i.setOption&3;return !!(i.level>6&&g<12||i.excellent&63||ancient===1||ancient===2||g===12&&(n<=6||n===15||n===30||n===31||n>=36&&n<=43||n===49||n===50||n>=130&&n<=135||n>=136&&n<=143)||g===13&&[3,4,5,14,15,19,30,31,34,35,36,37,52,53].includes(n)||g===14&&[13,14,16,22,31,41,42,43,44,112,113,121,122].includes(n)||g===13&&n===20&&!i.level||[19,512*5+10,512*4+18,512*2+13].includes(i.index));}
