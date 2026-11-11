// Build-time fixture only: compare against actual provided server function bodies.
// Never imports a server config or executes server startup.
export function transportReference(source) {
  const names=['DecryptData','EncryptData','MHPDecryptData','MHPEncryptData','InitHackCheck'];
  const functions=names.map(name=>{
    const start=source.indexOf(`void ${name}(`);
    if(start<0)throw new Error(`Missing reference function ${name}`);
    const brace=source.indexOf('{',start);let depth=1,end=brace+1;
    for(;end<source.length&&depth;end++){if(source[end]==='{')depth++;else if(source[end]==='}')depth--;}
    if(depth)throw new Error('Incomplete transport reference');
    return source.slice(start,end);
  });
  return `#pragma once\n#include <cstring>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
#define VM_START
#define VM_END
namespace server_reference {\nusing BYTE=unsigned char;using WORD=unsigned short;
inline BYTE LOBYTE(WORD n){return n&255;} inline BYTE HIBYTE(WORD n){return n>>8;}
template<std::size_t N>void strcpy_s(char(&dest)[N],const char* src){std::strncpy(dest,src,N);}
inline char CustomerName[32]{};\ninline BYTE EncDecKey1=0,EncDecKey2=0;\ninline struct {BYTE m_ServerEncDecKey1=0,m_ServerEncDecKey2=0;char m_ServerSerial[17]{};} gServerInfo;\nvoid MHPDecryptData(BYTE*,int);void MHPEncryptData(BYTE*,int);\n${functions.join('\n')}\n}\n#undef VM_START\n#undef VM_END\n#ifdef __clang__\n#pragma clang diagnostic pop\n#endif\n`;
}

export function packetFilterReference(mainSource,serverSource) {
  const table=source=>Array.from(source.matchAll(/m_XorFilter\[\d+\]\s*=\s*(0x[0-9A-Fa-f]+);/g),m=>m[1]).slice(0,32);
  const main=table(mainSource),server=table(serverSource);
  if(main.length!==32||JSON.stringify(main)!==JSON.stringify(server))throw new Error('EX603 filter tables do not match');
  const method=(source,name)=>{
    const start=source.indexOf('void CPacketManager::XorData('),brace=source.indexOf('{',start);let depth=1,end=brace+1;
    if(start<0||brace<0)throw new Error('Missing XorData reference');
    for(;end<source.length&&depth;end++){if(source[end]==='{')depth++;else if(source[end]==='}')depth--;}
    if(depth)throw new Error('Incomplete XorData reference');
    return source.slice(start,end).replace('CPacketManager::',`${name}::`);
  };
  const decl=name=>`struct ${name} {unsigned char m_buff[2048]{};unsigned char m_XorFilter[32]{${main.join(',')}};void XorData(int,int);};`;
  return `#pragma once\nnamespace packet_reference {\n${decl('MainFilter')}\n${decl('ServerFilter')}\n${method(mainSource,'MainFilter')}\n${method(serverSource,'ServerFilter')}\n}\n`;
}
