export function accountReference(header,util) {
 const struct=header.match(/#pragma pack\(push,1\)\s*(struct PMSG_CONNECT_ACCOUNT_RECV[\s\S]*?)#pragma pack\(pop\)/)?.[1];
 const decrypt=util.match(/void PacketArgumentDecrypt\(char\* out_buff,char\* in_buff,int size\)[^\n]*\n\{[\s\S]*?\n\}/)?.[0];
 if(!struct||!decrypt)throw new Error('EX603 account source reference missing');
 return `#pragma once\n#include <cstdint>\nnamespace account_reference {\nusing BYTE=std::uint8_t;using DWORD=std::uint32_t;\nstruct PSBMSG_HEAD {BYTE type,size,head,sub;};\n#define GAMESERVER_UPDATE 603\n#pragma pack(push,1)\n${struct}\n#pragma pack(pop)\n#undef GAMESERVER_UPDATE\n${decrypt}\n}\n`;
}
