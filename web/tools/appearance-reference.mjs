export function appearanceReference(source){
 const start=source.indexOf('info.CharSet[1] = TempInventory[0]%256;');
 const end=source.indexOf('#if(GAMESERVER_UPDATE>=601)',start);
 if(start<0||end<0)throw new Error('Appearance packer source missing');
 return `#pragma once\n#include <cstdint>\nnamespace appearance_reference {using BYTE=std::uint8_t;\nstruct Info{BYTE CharSet[18]{};};struct Input{BYTE Inventory[45]{};};\ninline Info pack(const int* TempInventory,const Input* lpInfo){Info info;\n${source.slice(start,end)}\nreturn info;}\n}\n`;
}

export function companionsReference(source){
 const start=source.indexOf('#if(GAMESERVER_UPDATE>=601)',source.indexOf('info.CharSet[1] = TempInventory[0]%256;'));
 const end=source.indexOf('#pragma endregion',start);
 if(start<0||end<0)throw new Error('Companion packer source missing');
 return `#pragma once\n#include <cstdint>\nnamespace companions_reference {using BYTE=std::uint8_t;struct Info{BYTE CharSet[18]{};};struct Input{BYTE Inventory[45]{};};\nstruct Custom {int CheckCustomWingByItem(int item){return item==12*512+500;}int GetCustomWingIndex(int){return 3;}};\ninline Info pack(const int* TempInventory,const Input* lpInfo){Info info;Custom gCustomWing;\n#define GET_ITEM(a,b) ((a)*512+(b))\n#define GAMESERVER_UPDATE 603\n${source.slice(start,end)}\n#undef GAMESERVER_UPDATE\n#undef GET_ITEM\nreturn info;}\n}\n`;
}
