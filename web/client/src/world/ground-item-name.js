// ZzzObject.cpp CreateDropItemName: level, Skill, Option, Luck.
// Option is a presence suffix, not an invented +4/+8 defense/damage value.
export function groundItemName(item,catalog,texts=new Map()){
 if(item.money!==undefined)return item.money.toLocaleString('en-US')+' '+(texts.get(224)??'Zen');
 let name=(catalog?.[item.index]?.name??'Item '+item.index)+(item.level?' +'+item.level:'');
 const g=item.index>>9,t=item.index&511,suffix=g<=12||(g===13&&((t>=8&&t<=9)||(t>=12&&t<=13)||(t>=21&&t<=28)));
 if(suffix){if(item.skill)name+=texts.get(176)??'+Skill';if(item.option)name+=texts.get(177)??'+Option';if(item.luck)name+=texts.get(178)??'+Luck';}
 return name;
}
// TEXT_COLOR ids used by CreateDropItemName. The ground layer uses HookColorFont,
// which differs from the tooltip palette (#19C81C green here vs #19FF80 in tooltips).
const colors={blue:'#7fb2ff',yellow:'#ffcc19',green:'#19c81c',violet:'#b266ff',purple:'#ff19ff',orange:'#e56b0a',gray:'#bcbcbc',white:'#ffffff'};
// Yellow list (also the bold list subset) from ZzzObject.cpp:5921-5982.
function yellowDrop(g,t,level){
 if(g===14&&([13,14,15,16,22,31,41,42,43,44].includes(t)||[17,18,19,28,29,91,100,111,112,113].includes(t)||t===21&&level===3||t>=101&&t<=109||t>=114&&t<=119||t>=120&&t<=124||t>=126&&t<=132||t>=134&&t<=139||t>=157&&t<=159))return true;
 if(g===12&&[15,25,30,31].includes(t))return true;
 if(g===12&&t>=136&&t<=143)return true;
 if(g===13&&[14,97,98,109,110,111,112,113,114,115,121].includes(t))return true;
 return false;
}
function boldDrop(g,t){
 return g===14&&[13,14,16,17,18,19,22,31,41,42,43,44].includes(t)||g===12&&([15,30,31].includes(t)||t>=136&&t<=143)||g===13&&t===14;
}
export function groundItemVisual(item,catalog,texts=new Map()){
 const name=groundItemName(item,catalog,texts),g=item.index>>9,t=item.index&511;
 if(item.money!==undefined)return {name,color:colors.yellow,bold:false};
 if(yellowDrop(g,t,item.level))return {name,color:colors.yellow,bold:boldDrop(g,t)};
 // SocketItem/Seed ranges and helper 66 render violet.
 if(g===12&&(t>=60&&t<=65||t>=70&&t<=74||t>=100&&t<=129)||g===13&&t===66)return {name,color:colors.violet,bold:false};
 if([2*512+13,5*512+10,0*512+19,4*512+18,5*512+36].includes(item.index))return {name,color:colors.purple,bold:false};
 const plain=!item.excellent&&!item.setOption||g===12&&((t>=3&&t<=6)||(t>=36&&t<=40)||(t>=42&&t<=43)||(t>=49&&t<=50))||g===13&&t===30;
 if(!plain)return {name,color:colors.green,bold:false};
 if(item.level>=7)return {name,color:colors.yellow,bold:false};
 if(item.skill||item.luck||(item.option&3))return {name,color:colors.blue,bold:false};
 if(item.level>=5)return {name,color:colors.blue,bold:false};
 if(item.level>=3)return {name,color:colors.orange,bold:false};
 if(item.level)return {name,color:colors.gray,bold:false};
 return {name,color:colors.white,bold:false};
}
