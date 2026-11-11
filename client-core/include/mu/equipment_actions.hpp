#pragma once
#include "connect_protocol.hpp"
#include <array>
namespace mu::core {
// Narrow Item.bmd field reader; validates the entire original table checksum.
inline bool item_two_hand(const WireBytes& b,unsigned index){
 constexpr unsigned size=84*8192;wire_require(index<8192&&b.size()==size+4,"Unsupported Item BMD layout/index");
 auto u32=[&](unsigned n){return std::uint32_t(b[n])|(std::uint32_t(b[n+1])<<8)|(std::uint32_t(b[n+2])<<16)|(std::uint32_t(b[n+3])<<24);};
 std::uint32_t key=0xE2F1,result=key<<9;for(unsigned offset=0;offset<size;offset+=4){auto value=u32(offset);if((offset/4+key)%2)result+=value;else result^=value;if(offset%16==0)result^=(key+result)>>(offset/4%8+1);}
 wire_require(result==u32(size),"Item BMD checksum mismatch");return (b[index*84+30]^0xFC)!=0;
}
inline std::array<unsigned,3> equipped_actions(unsigned cls,int right,int left,bool two){
 wire_require(cls<7&&right>=-1&&right<8192&&left>=-1&&left<8192,"Invalid action equipment identity");
 std::array<unsigned,3> out={cls==2?2u:cls==5?3u:1u,(cls==2||cls==5)?16u:15u,25};
 if(right>=0){unsigned group=unsigned(right)/512,type=unsigned(right)%512;if(group<=2){if(two){if(group==0&&(type==21||type==23||type==25||type==31))out={142,143,144};else out={5,18,28};}else out={4,17,(left>=0&&left/512<=2)?27u:26u};return out;}if(group==3){out={(type==1||type==2)?6u:two?7u:4u,type==1?19u:20u,29};return out;}if(group==5){if(type>=14&&type<=20)out={10,23,32};else out={two?7u:4u,two?20u:17u,two?29u:26u};return out;}}
 if(left>=0&&left/512==4&&left%512!=7)return {8,21,30};if(right>=0&&right/512==4&&right%512>=8&&right%512!=15)return {9,22,31};return out;
}
inline unsigned equipped_attack(int right,int left,bool two){
 wire_require(right>=-1&&right<8192&&left>=-1&&left<8192,"Invalid attack equipment identity");
 if(left>=0&&left/512==4&&left%512!=7)return 50;
 if(right>=0){unsigned group=unsigned(right)/512,type=unsigned(right)%512;if(group<=2)return two?43:39;if(group==3)return (type==1||type==2)?46:47;if(group==4&&type>=8&&type!=15)return 51;if(group==5)return two?148:39;}
 else if(left>=0&&left/512<=2)return 41;return 38;
}

}
