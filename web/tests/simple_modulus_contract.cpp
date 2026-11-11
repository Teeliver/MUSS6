#include "mu/simple_modulus.hpp"
#include <iostream>
#ifdef MU_MODULUS_REFERENCE
#include "modulus_reference.hpp"
#endif
using namespace mu::core;
void check(bool b){if(!b)throw std::runtime_error("SimpleModulus contract failed");}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}check(failed);}
ModulusKey key(bool decrypt){return {{65537,65537,65537,65537},{decrypt?21846u:3u,decrypt?21846u:3u,decrypt?21846u:3u,decrypt?21846u:3u},{0x1234,0xABCD,0x5678,0x1357}};}
WireBytes key_file(const ModulusKey& k){
 WireBytes b(54,0);b[0]=0x12;b[1]=0x11;b[2]=54;const std::uint32_t mask[]={0x3F08A79B,0xE25CC287,0x93D27AB9,0x20DEA7BF};
 for(unsigned table=0;table<3;table++)for(unsigned i=0;i<4;i++){auto value=(table==0?k.modulus[i]:table==1?k.key[i]:k.xorValue[i])^mask[i];for(unsigned n=0;n<4;n++)b[6+table*16+i*4+n]=std::uint8_t(value>>(8*n));}return b;
}
int main(){
 auto enc=key(false),dec=key(true);check(parse_modulus_key(key_file(enc)).key==enc.key);
 auto file=key_file(enc);for(unsigned n=0;n<54;n++)rejects([&]{parse_modulus_key(WireBytes(file.begin(),file.begin()+n));});
 auto bad=file;bad.push_back(0);rejects([&]{parse_modulus_key(bad);});bad=file;bad[0]=0;rejects([&]{parse_modulus_key(bad);});
 auto invalid=enc;invalid.modulus[0]=0;rejects([&]{parse_modulus_key(key_file(invalid));});
#ifdef MU_MODULUS_REFERENCE
 modulus_reference::Cipher reference;
 for(unsigned i=0;i<4;i++){reference.m_Encryption.Modulus[i]=enc.modulus[i];reference.m_Encryption.Key[i]=enc.key[i];reference.m_Encryption.Xor[i]=enc.xorValue[i];reference.m_Decryption.Modulus[i]=dec.modulus[i];reference.m_Decryption.Key[i]=dec.key[i];reference.m_Decryption.Xor[i]=dec.xorValue[i];}
#endif
 for(unsigned size=1;size<=80;size++){
  WireBytes plain;for(unsigned i=0;i<size;i++)plain.push_back(std::uint8_t(i*73+size));auto wire=modulus_encrypt(plain,enc);check(modulus_decrypt(wire,dec)==plain);
#ifdef MU_MODULUS_REFERENCE
  for(unsigned offset=0;offset<size;offset+=8){
   alignas(std::uint32_t)std::uint8_t padded[8]{},original[11]{},decoded[8]{};unsigned n=std::min(8u,size-offset);std::copy_n(plain.begin()+offset,n,padded);
   check(reference.EncryptBlock(original,padded,int(n))==88);check(std::equal(original,original+11,wire.begin()+(offset/8)*11));check(reference.DecryptBlock(decoded,original)==int(n));check(std::equal(decoded,decoded+n,plain.begin()+offset));
  }
#endif
 }
#ifdef MU_MODULUS_REFERENCE
 auto wrapping=enc;for(unsigned i=0;i<4;i++){wrapping.key[i]=0xFFFFFFFFu;reference.m_Encryption.Key[i]=0xFFFFFFFFu;}
 alignas(std::uint32_t)std::uint8_t extreme[8]={0xFF,0xFF,0xFE,0xFF,0xFD,0xFF,0xFC,0xFF},originalWrap[11]{};
 reference.EncryptBlock(originalWrap,extreme,8);check(modulus_encrypt(WireBytes(extreme,extreme+8),wrapping)==WireBytes(originalWrap,originalWrap+11));
#endif
 auto zeroLength=modulus_encrypt({1},enc);zeroLength[9]=std::uint8_t(zeroLength[10]^0x3D);rejects([&]{modulus_decrypt(zeroLength,dec);});
 PacketCipher limit(enc,dec);WireBytes c3(185,0);c3[0]=0xC3;c3[1]=185;check(limit.encode(c3).size()==255);
 PacketCipher oversized(enc,dec);c3.resize(186);c3[1]=186;rejects([&]{oversized.encode(c3);});check(oversized.is_failed()&&oversized.sent_serial()==0);
 auto corrupt=modulus_encrypt({1,2,3},enc);corrupt[10]^=1;rejects([&]{modulus_decrypt(corrupt,dec);});
 rejects([&]{modulus_decrypt({1},dec);});auto partial=modulus_encrypt({1},enc);auto another=partial;partial.insert(partial.end(),another.begin(),another.end());rejects([&]{modulus_decrypt(partial,dec);});
 PacketCipher sender(enc,dec);
 for(unsigned n=0;n<260;n++){
  auto wire=sender.encode({0xC3,5,0xF1,1,1});auto body=modulus_decrypt(WireBytes(wire.begin()+2,wire.end()),dec);check(body[0]==std::uint8_t(n));
  WireBytes restored={0xC1,5};restored.insert(restored.end(),body.begin()+1,body.end());check(filter_plain_packet(restored,false)==WireBytes({0xC1,5,0xF1,1,1}));
 }check(sender.sent_serial()==4);
 PacketCipher receiver(enc,dec);
 for(unsigned n=0;n<260;n++){
  auto body=modulus_encrypt({std::uint8_t(n),0xF1,1,1},enc);WireBytes wire={0xC4,0,std::uint8_t(body.size()+3)};wire.insert(wire.end(),body.begin(),body.end());check(receiver.decode(wire)==WireBytes({0xC2,0,6,0xF1,1,1}));
 }check(receiver.received_serial()==4);
 auto wrongBody=modulus_encrypt({99,0xF1,1,1},enc);WireBytes wrong={0xC3,std::uint8_t(wrongBody.size()+2)};wrong.insert(wrong.end(),wrongBody.begin(),wrongBody.end());rejects([&]{receiver.decode(wrong);});check(receiver.is_failed());rejects([&]{receiver.decode({0xC1,3,0});});
 PacketCipher failure(enc,dec);rejects([&]{failure.encode({0xC3,2});});check(failure.is_failed());
 std::cout<<"simple-modulus contract passed\n";
}
