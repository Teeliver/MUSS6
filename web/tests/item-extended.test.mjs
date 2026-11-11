import test from 'node:test';import assert from 'node:assert/strict';import {readFile} from 'node:fs/promises';import {createHash} from 'node:crypto';
import {socketItemHash,jewelryHarmonyHash,itemAddOptionHash,socketTable,harmonyTable,itemAddOptions} from '../client/src/ui/item-extended-data.js';
import {socketValueText,itemSocketInfo,itemHarmonyInfo,itemPeriodState,itemAddOptionLines,expireTimeText,socketCategoryText} from '../client/src/ui/item-extended-info.js';
import {decodeWireItem,decodePeriodicItem,createGameState} from '../client/src/protocol/game-state.js';
import {createNpcShop} from '../client/src/protocol/npc-shop.js';
import {mapDisplayName,mapNameTextId} from '../client/src/world/hero-position.js';
import {mapTitleAssets} from '../client/src/world/map-title-assets.js';
import {decodeTextTable} from '../client/src/ui/tables.js';
const data='/Users/angelale/Downloads/MuClientEX603/Data/';
const texts=decodeTextTable(new Uint8Array(await readFile(data+'Local/Eng/Text_eng.bmd')));

test('generated socket and harmony tables match the checksummed original Data files',async()=>{
 const socket=await readFile(data+'Local/Eng/SocketItem_eng.bmd'),harmony=await readFile(data+'Local/Eng/JewelOfHarmonyOption_eng.bmd'),addOption=await readFile(data+'Local/ItemAddOption.bmd');
 assert.equal(createHash('sha256').update(socket).digest('hex'),socketItemHash);
 assert.equal(createHash('sha256').update(harmony).digest('hex'),jewelryHarmonyHash);
 assert.equal(createHash('sha256').update(addOption).digest('hex'),itemAddOptionHash);
 assert.equal(socketTable.length,3);assert.equal(socketTable[0].length,50);assert.equal(harmonyTable.length,3);assert.equal(harmonyTable[0].length,10);
 assert.equal(Object.keys(itemAddOptions).length,101);
 assert.deepEqual(itemAddOptions[22],[2,200,1,10]);assert.deepEqual(itemAddOptions[7*512+29],[8,20,3,10]);
 assert.deepEqual(socketTable[0][0],{id:0,category:1,name:'(Level type)Attack/Wizardry increase',type:3,values:[20,19,18,17,14]});
 assert.equal(harmonyTable[2][6].name,'Damage decrement increase');
});

test('socket tooltip follows NewUIItemMng seed/sphere decoding and SocketSystem text',()=>{
 const wire=Uint8Array.from([0,0,20,0,0,0,1,0,1,50,254,255]);
 const item=decodeWireItem(wire);
 assert.equal(item.socketCount,4);assert.equal(item.harmony,1);
 assert.deepEqual(item.socketOptions,[0,1,50,254,255]);
 const info=itemSocketInfo(item,{level:400,maxHp:1000,maxMp:500});
 assert.equal(info.sockets.length,4);
 assert.deepEqual(info.sockets[0],{number:1,seed:0,sphere:1,category:1,name:'(Level type)Attack/Wizardry increase',valueText:'+20'});
 assert.deepEqual(info.sockets[1],{number:2,seed:1,sphere:1,category:1,name:'Attack speed increase',valueText:'+7'});
 assert.deepEqual(info.sockets[2],{number:3,seed:0,sphere:2,category:1,name:'(Level type)Attack/Wizardry increase',valueText:'+21'});
 assert.equal(info.sockets[3].empty,true);
 assert.deepEqual(info.set,{name:'Skill attack increase',valueText:'+11'});
 assert.equal(itemSocketInfo(decodeWireItem(Uint8Array.from([0,0,20,0,0,0,0,255,255,255,255,255]))),undefined);
 assert.equal(socketValueText(2,4),'+4%');assert.equal(socketValueText(4,10,{maxHp:1000}),'+100');assert.equal(socketValueText(5,10,{maxMp:500}),'+50');
 assert.equal(socketCategoryText(id=>id===2640?'Fire':'',1),'Fire');assert.equal(socketCategoryText(()=>'',0),'');
});

test('harmony tooltip reads option nibbles and ignores socket items like ItemByteConvert',()=>{
 const item=decodeWireItem(Uint8Array.from([0,0,20,0,0,96,(7<<4)|10,255,255,255,255,255]));
 assert.equal(item.harmony,0x7a);assert.equal(item.socketCount,0);
 assert.deepEqual(itemHarmonyInfo(item),{name:'Damage decrement increase',value:4,percent:true,level:10});
 // Weapon options 1..10 are valid (MAXHARMONYJEWELOPTIONINDEX+1); staff/shield only 1..8.
 assert.equal(itemHarmonyInfo(decodeWireItem(Uint8Array.from([0,0,20,0,0,0,(11<<4)|1,255,255,255,255,255]))),undefined);
 const socket=decodeWireItem(Uint8Array.from([45,0,20,0,0,112,(7<<4)|10,0,255,255,255,255]));
 assert.equal(socket.index,7*512+45);assert.equal(socket.socketCount,1);
 assert.equal(itemHarmonyInfo(socket),undefined);
 // Socket-capable item type suppresses harmony even with no socket applied (IsSocketItem(type)).
 const bare=decodeWireItem(Uint8Array.from([45,0,20,0,0,112,(7<<4)|10,255,255,255,255,255]));
 assert.equal(bare.socketCount,0);assert.equal(itemHarmonyInfo(bare),undefined);
});

test('period and 380 flags decode from the 12-byte item wire',()=>{
 const active=decodeWireItem(Uint8Array.from([0,0,20,0,0,2,0,255,255,255,255,255]));
 assert.equal(active.periodic,true);assert.equal(active.expired,false);assert.equal(itemPeriodState(active),'active');
 const expired=decodeWireItem(Uint8Array.from([0,0,20,0,0,6,0,255,255,255,255,255]));
 assert.equal(expired.expired,true);assert.equal(itemPeriodState(expired),'expired');
 const option380=decodeWireItem(Uint8Array.from([0,0,20,0,0,8,0,255,255,255,255,255]));
 assert.equal(option380.option380,true);assert.equal(itemPeriodState(option380),undefined);
});

test('380 add-option tooltip reads ItemAddOption.bmd rows and expiration time formats locally',()=>{
 const item=decodeWireItem(Uint8Array.from([22,0,20,0,0,8,255,255,255,255,255,255]));
 assert.equal(item.index,22);assert.equal(item.option380,true);
 assert.deepEqual(itemAddOptionLines(item),[{id:2185,value:200},{id:2184,value:10}]);
 assert.deepEqual(itemAddOptionLines({...item,option380:false}),[]);
 const local=new Date(2026,0,2,3,4,0).getTime()/1000;
 assert.equal(expireTimeText(local),'2026-01-02  03:04');assert.equal(expireTimeText(0),undefined);
 const state=createGameState();
 state.patchInventory({slot:12,item:decodeWireItem(Uint8Array.from([22,0,20,0,0,2,255,255,255,255,255,255]))});
 const time=Math.floor(Date.now()/1000)+86400;
 const packet=Uint8Array.of(193,12,210,18,1,0,12,0,time&255,time>>8&255,time>>16&255,time>>>24);
 const periodic=decodePeriodicItem(packet);
 assert.deepEqual(periodic,{slot:12,time});
 state.itemExpire(periodic.slot,periodic.time);
 assert.equal(state.snapshot().items[0].expireTime,time);
});

test('PC Point price list C1:F3:E6 uses a six-byte header and 20-byte records',()=>{
 const shop=createNpcShop();
 const packet=Uint8Array.of(194,0,26,243,230,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,123,0,0,0);
 // malformed trailing byte is rejected
 assert.throws(()=>shop.apply(packet.slice(0,-1)));
 assert.equal(shop.apply(packet),false);
 shop.begin({index:258,type:251});
 shop.apply(Uint8Array.of(195,11,48,0,0,0,0,0,0,0,0));
 shop.apply(Uint8Array.of(194,0,19,49,0,1,0,0,0,20,0,0,0,0,0,0,0,0,0));
 assert.equal(shop.apply(packet),true);
 const snap=shop.snapshot();
 assert.equal(snap.currency,4);
 assert.deepEqual(snap.prices,[{index:0,level:0,durability:0,excellent:0,price:123}]);
});

test('map 32 uses the CustomMap/UIMapName Devil Square title and display name',()=>{
 assert.equal(mapNameTextId(32),39);assert.equal(mapDisplayName(32,texts),'Devil Square');
 assert.equal(mapDisplayName(40,texts),'GM summon zone');
 assert.equal(mapTitleAssets[32].path,'Interface/ImgsMapName/DevilSquare.ozt');
 assert.equal(mapTitleAssets[40],undefined);assert.equal(mapTitleAssets[82].path,'Interface/ImgsMapName/Karutan.ozt');
});

test('inventory equipment BodyHeight is applied to source vertex z before rotation',async()=>{
 const source=await readFile(new URL('../client/src/ui/item-icons.js',import.meta.url),'utf8');
 assert.match(source,/const number=item\.index&511,lucky=group>=7&&group<=12&&number>=62&&number<=72;/);
 assert.match(source,/const bodyHeight=\(\{7:-160,8:-100,9:-50,10:-70,11:0\}\[group\]\?\?0\)\+\(lucky\?\(group===7\?-10:group===8\?-13:0\):0\);/);
 assert.match(source,/positions\[i\+2\]\+=bodyHeight/);
 assert.match(source,/group===7&&\[65,70\]\.includes\(number\)/);
});
