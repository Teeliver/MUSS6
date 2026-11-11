#pragma once
#include "client_identity.hpp"
#include <array>
#include <cstdint>
namespace mu::core {
struct ModulusKey {std::array<std::uint32_t,4> modulus,key,xorValue;};
inline std::uint32_t key_u32(const WireBytes& b,unsigned offset){return std::uint32_t(b.at(offset))|(std::uint32_t(b.at(offset+1))<<8)|(std::uint32_t(b.at(offset+2))<<16)|(std::uint32_t(b.at(offset+3))<<24);}
inline void validate_modulus_key(const ModulusKey& k){for(unsigned i=0;i<4;i++)wire_require(k.modulus[i]>0&&k.modulus[i]<=262144&&k.key[i]>0,"Unsupported modulus key bounds");}
inline ModulusKey parse_modulus_key(const WireBytes& b){
 wire_require(b.size()==54&&b[0]==0x12&&b[1]==0x11&&key_u32(b,2)==54,"Invalid EX603 key-file layout");
 constexpr std::uint32_t mask[]={0x3F08A79B,0xE25CC287,0x93D27AB9,0x20DEA7BF};ModulusKey result{};
 for(unsigned i=0;i<4;i++){result.modulus[i]=key_u32(b,6+i*4)^mask[i];result.key[i]=key_u32(b,22+i*4)^mask[i];result.xorValue[i]=key_u32(b,38+i*4)^mask[i];}
 validate_modulus_key(result);return result;
}
namespace modulus_detail {
inline void bits(std::uint8_t* target,unsigned targetBit,const std::uint8_t* source,unsigned sourceBit,unsigned count){
 for(unsigned i=0;i<count;i++){unsigned value=(source[(sourceBit+i)/8]>>(7-(sourceBit+i)%8))&1;auto& byte=target[(targetBit+i)/8];byte=std::uint8_t(byte|(value<<(7-(targetBit+i)%8)));}
}
inline std::array<std::uint8_t,4> le(std::uint32_t value){return {std::uint8_t(value),std::uint8_t(value>>8),std::uint8_t(value>>16),std::uint8_t(value>>24)};}
inline std::array<std::uint8_t,11> encrypt_block(const std::uint8_t* source,unsigned length,const ModulusKey& keys){
 std::array<std::uint8_t,8> padded{};std::copy_n(source,length,padded.begin());std::array<std::uint32_t,4> words{};std::uint32_t carry=0;
 for(unsigned i=0;i<4;i++){
  auto word=std::uint32_t(padded[i*2])|(std::uint32_t(padded[i*2+1])<<8);
  // DWORD multiplication wraps at 32 bits in the supplied Windows source.
  const std::uint32_t product=((keys.xorValue[i]^word)^carry)*keys.key[i];words[i]=product%keys.modulus[i];carry=std::uint16_t(words[i]);
 }
 for(unsigned i=0;i<3;i++)words[i]=(words[i]^keys.xorValue[i])^std::uint16_t(words[i+1]);
 std::array<std::uint8_t,11> out{};unsigned position=0;
 for(auto word:words){auto bytes=le(word);bits(out.data(),position,bytes.data(),0,16);position+=16;bits(out.data(),position,bytes.data(),22,2);position+=2;}
 std::uint8_t checksum=0xF8;for(auto byte:padded)checksum^=byte;
 const std::uint8_t tail[]={std::uint8_t((checksum^length)^0x3D),checksum};bits(out.data(),72,tail,0,16);return out;
}
inline WireBytes decrypt_block(const std::uint8_t* source,const ModulusKey& keys){
 std::array<std::uint32_t,4> words{};
 for(unsigned i=0;i<4;i++){std::array<std::uint8_t,4> word{};bits(word.data(),0,source,i*18,16);bits(word.data(),22,source,i*18+16,2);words[i]=std::uint32_t(word[0])|(std::uint32_t(word[1])<<8)|(std::uint32_t(word[2])<<16);}
 for(int i=2;i>=0;i--)words[i]=(words[i]^keys.xorValue[i])^std::uint16_t(words[i+1]);
 std::array<std::uint8_t,8> plain{};std::uint32_t carry=0;
 for(unsigned i=0;i<4;i++){const std::uint32_t product=keys.key[i]*words[i];auto word=std::uint16_t(((product%keys.modulus[i])^keys.xorValue[i])^carry);plain[i*2]=word;plain[i*2+1]=word>>8;carry=std::uint16_t(words[i]);}
 std::uint8_t checksum=0xF8;for(auto b:plain)checksum^=b;
 const unsigned length=(source[9]^source[10])^0x3D;
 wire_require(checksum==source[10]&&length>0&&length<=8,"Invalid encrypted block checksum/length");
 return WireBytes(plain.begin(),plain.begin()+length);
}
}
inline WireBytes modulus_encrypt(const WireBytes& plain,const ModulusKey& keys){
 validate_modulus_key(keys);wire_require(!plain.empty()&&plain.size()<=16384,"Invalid plaintext size");WireBytes out;out.reserve(((plain.size()+7)/8)*11);
 for(std::size_t i=0;i<plain.size();i+=8){auto block=modulus_detail::encrypt_block(plain.data()+i,unsigned(std::min<std::size_t>(8,plain.size()-i)),keys);out.insert(out.end(),block.begin(),block.end());}return out;
}
inline WireBytes modulus_decrypt(const WireBytes& wire,const ModulusKey& keys){
 validate_modulus_key(keys);wire_require(!wire.empty()&&wire.size()<=16384&&wire.size()%11==0,"Invalid ciphertext size");WireBytes out;
 for(std::size_t i=0;i<wire.size();i+=11){auto block=modulus_detail::decrypt_block(wire.data()+i,keys);wire_require(i+11==wire.size()||block.size()==8,"Partial encrypted block before stream end");out.insert(out.end(),block.begin(),block.end());}return out;
}
class PacketCipher {
 ModulusKey encryption,decryption;std::uint8_t sent=0,received=0;bool failed=false;
 static unsigned header(const WireBytes& b){
  wire_require(b.size()>=3&&b.size()<=16384&&b[0]>=0xC1&&b[0]<=0xC4,"Invalid packet header");
  unsigned h=(b[0]==0xC2||b[0]==0xC4)?3:2;wire_require(b.size()>h,"Missing packet opcode");
  unsigned size=h==3?(unsigned(b[1])<<8)|b[2]:b[1];wire_require(size==b.size(),"Packet size mismatch");return h;
 }
public:
 PacketCipher(ModulusKey enc,ModulusKey dec):encryption(enc),decryption(dec){validate_modulus_key(enc);validate_modulus_key(dec);}
 WireBytes encode(WireBytes plain)try{
  wire_require(!failed,"Packet cipher is failed");unsigned h=header(plain);wire_require(plain.size()<2048,"Outbound packet exceeds source bound");
  const bool encrypted=plain[0]==0xC3||plain[0]==0xC4,wide=h==3;plain[0]=wide?0xC2:0xC1;
  plain=filter_plain_packet(std::move(plain),true);if(!encrypted)return plain;
  WireBytes input(plain.begin()+h-1,plain.end());input[0]=sent;auto body=modulus_encrypt(input,encryption);
  wire_require(wide||body.size()+h<=255,"Encrypted C3 packet exceeds byte length");WireBytes out(h,0);out[0]=wide?0xC4:0xC3;out.insert(out.end(),body.begin(),body.end());out[h-1]=std::uint8_t(out.size());if(wide)out[1]=std::uint8_t(out.size()>>8);sent++;return out;
 }catch(...){failed=true;throw;}
 WireBytes decode(WireBytes wire)try{
  wire_require(!failed,"Packet cipher is failed");unsigned h=header(wire);if(wire[0]==0xC1||wire[0]==0xC2)return wire;
  bool wide=h==3;auto plain=modulus_decrypt(WireBytes(wire.begin()+h,wire.end()),decryption);
  wire_require(plain.size()>=2&&plain[0]==received,"Inbound packet serial mismatch");WireBytes out(h,0);out[0]=wide?0xC2:0xC1;out.insert(out.end(),plain.begin()+1,plain.end());
  wire_require(wide||out.size()<=255,"Normalized C1 exceeds byte length");out[h-1]=std::uint8_t(out.size());if(wide)out[1]=std::uint8_t(out.size()>>8);received++;return out;
 }catch(...){failed=true;throw;}
 bool is_failed()const{return failed;}
 unsigned sent_serial()const{return sent;}
 unsigned received_serial()const{return received;}
};
}
