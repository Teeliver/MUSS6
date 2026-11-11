export function characterReference(source){
 const names=['PMSG_CHARACTER_LIST_SEND','PMSG_CHARACTER_LIST','PMSG_CHARACTER_CREATION_ENABLE_SEND'];
 const structs=names.map(name=>{
  const value=source.match(new RegExp(`struct ${name}\\s*\\{[\\s\\S]*?\\n\\};`))?.[0];
  if(!value)throw new Error('Character source schema missing');return value;
 }).join('\n');
 return `#pragma once\n#include <cstdint>\nnamespace character_reference {\nusing BYTE=std::uint8_t;using WORD=std::uint16_t;\nstruct PSBMSG_HEAD{BYTE type,size,head,sub;};struct PBMSG_HEAD{BYTE type,size,head;};\n#define GAMESERVER_UPDATE 603\n#define GAMESERVER_EXTRA 1\n${structs}\n#undef GAMESERVER_UPDATE\n#undef GAMESERVER_EXTRA\n}\n`;
}
