#pragma once
#include "connect_protocol.hpp"
#include <array>
namespace mu::core {
inline WireBytes hardware_identity_packet(const std::string& identity) {
 wire_require(identity.size()==44,"Invalid identity format");
 for(unsigned i=0;i<44;i++){
  const bool hyphen=i==8||i==17||i==26||i==35;
  wire_require(hyphen?identity[i]=='-':((identity[i]>='0'&&identity[i]<='9')||(identity[i]>='A'&&identity[i]<='F')),"Invalid identity format");
 }
 WireBytes result(49,0);result[0]=0xC1;result[1]=49;result[2]=0xF3;result[3]=9;
 std::copy(identity.begin(),identity.end(),result.begin()+4);return result;
}
// Client -> server packet XOR layer. C3/C4 requires a separate crypto/serial pipeline.
inline WireBytes filter_plain_packet(WireBytes packet,bool encode) {
 wire_require(packet.size()>=3&&packet.size()<2048,"Invalid packet filter size");
 wire_require(packet[0]==0xC1||packet[0]==0xC2,"Only plain C1/C2 packet filter is implemented");
 const unsigned head=packet[0]==0xC2?3:2;
 wire_require(packet.size()>head,"Missing packet opcode");
 const auto size=head==2?unsigned(packet[1]):(unsigned(packet[1])<<8)|packet[2];
 wire_require(size==packet.size(),"Packet filter length mismatch");
 constexpr std::array<std::uint8_t,32> filter={0xAB,0x11,0xCD,0xFE,0x18,0x23,0xC5,0xA3,0xCA,0x33,0xC1,0xCC,0x66,0x67,0x21,0xF3,0x32,0x12,0x15,0x35,0x29,0xFF,0xFE,0x1D,0x44,0xEF,0xCD,0x41,0x26,0x3C,0x4E,0x4D};
 if(encode)for(unsigned i=head+1;i<packet.size();i++)packet[i]^=packet[i-1]^filter[i%32];
 else for(unsigned i=unsigned(packet.size())-1;i>head;i--)packet[i]^=packet[i-1]^filter[i%32];
 return packet;
}
}
