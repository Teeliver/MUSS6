// Test-only extraction of supplied Main_EX603 block operations, without key files or I/O.
export function modulusReference(source) {
 const methods=['EncryptBlock','DecryptBlock','AddBits','GetByteOfBit','Shift'];
 const bodies=methods.map(name=>{
  const signature=new RegExp(`(?:int|void) CPacketManager::${name}\\(`).exec(source);
  if(!signature)throw new Error(`Missing modulus reference ${name}`);
  const start=signature.index,brace=source.indexOf('{',start);let depth=1,end=brace+1;
  for(;end<source.length&&depth;end++){if(source[end]==='{')depth++;else if(source[end]==='}')depth--;}
  if(depth)throw new Error('Incomplete modulus reference');
  return source.slice(start,end).replace('CPacketManager::','Cipher::');
 });
 return `#pragma once\n#include <cstdint>\n#include <cstring>\nnamespace modulus_reference {\nusing BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;\nstruct Key {DWORD Modulus[4],Key[4],Xor[4];};\nstruct Cipher {Key m_Encryption{},m_Decryption{};int EncryptBlock(BYTE*,BYTE*,int);int DecryptBlock(BYTE*,BYTE*);int AddBits(BYTE*,int,BYTE*,int,int);int GetByteOfBit(int);void Shift(BYTE*,int,int);};\n${bodies.join('\n')}\n}\n`;
}
