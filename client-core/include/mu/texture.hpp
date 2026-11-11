#pragma once
#include "connect_protocol.hpp"
namespace mu::core {
struct TextureData {unsigned width,height;WireBytes bytes;bool jpeg;};
inline unsigned texture_u16(const WireBytes& b,std::size_t p){wire_require(p+2<=b.size(),"Truncated texture integer");return b[p]|(unsigned(b[p+1])<<8);}
inline TextureData decode_ozt(const WireBytes& b){
 wire_require(b.size()>=22&&b.size()<=16*1024*1024,"Invalid OZT byte size");const unsigned h=4;
 wire_require(b[h+1]==0&&b[h+2]==2,"Unsupported OZT format");unsigned w=texture_u16(b,h+12),height=texture_u16(b,h+14),bits=b[h+16];
 wire_require(w>0&&height>0&&w<=2048&&height<=2048&&(bits==24||bits==32),"Unsupported OZT dimensions/depth");
 unsigned stride=bits/8;std::size_t start=h+18+b[h];wire_require(start+std::size_t(w)*height*stride<=b.size(),"Truncated OZT pixels");
 TextureData out{w,height,WireBytes(std::size_t(w)*height*4),false};bool top=b[h+17]&32,right=b[h+17]&16;
 for(unsigned y=0;y<height;y++)for(unsigned x=0;x<w;x++){
  auto src=start+(std::size_t(top?y:height-y-1)*w+(right?w-x-1:x))*stride,dst=(std::size_t(y)*w+x)*4;
  out.bytes[dst]=b[src+2];out.bytes[dst+1]=b[src+1];out.bytes[dst+2]=b[src];out.bytes[dst+3]=stride==4?b[src+3]:255;
 }return out;
}
inline TextureData unwrap_ozj(const WireBytes& b){
 wire_require(b.size()>28&&b.size()<=16*1024*1024&&b[24]==255&&b[25]==216,"Invalid OZJ wrapper");
 // Native JPEG readers stop at EOI; original assets can contain nonzero trailer bytes.
 const std::size_t end=b.size();std::size_t jpeg_end=0;
 std::size_t pos=26;unsigned width=0,height=0;
 while(pos<end){
  wire_require(b[pos++]==255,"Invalid JPEG marker");while(pos<end&&b[pos]==255)pos++;wire_require(pos<end,"Truncated JPEG marker");unsigned marker=b[pos++];
  if(marker==0xD9){jpeg_end=pos;break;}
  wire_require(marker!=0&&marker!=0xD8&&marker!=0xD9&&!(marker>=0xD0&&marker<=0xD7),"Unsupported JPEG marker");
  wire_require(pos+2<=end,"Truncated JPEG segment");unsigned length=(unsigned(b[pos])<<8)|b[pos+1];wire_require(length>=2&&length<=end-pos,"Invalid JPEG segment size");
  if(marker>=0xC0&&marker<=0xCF&&marker!=0xC4&&marker!=0xC8&&marker!=0xCC){
   wire_require((marker==0xC0||marker==0xC2)&&length>=8&&width==0,"Unsupported JPEG encoding");
   height=(unsigned(b[pos+3])<<8)|b[pos+4];width=(unsigned(b[pos+5])<<8)|b[pos+6];unsigned channels=b[pos+7];
   wire_require(b[pos+2]==8&&(channels==1||channels==3)&&length==8+3*channels&&width>0&&height>0&&width<=2048&&height<=2048,"Unsupported JPEG dimensions/components");
  }pos+=length;
  if(marker==0xDA){
   wire_require(width>0,"Missing JPEG dimensions");
   // Entropy bytes escape FF as FF00; restart markers have no length.
   while(pos<end){if(b[pos]!=255){pos++;continue;}std::size_t marker_start=pos++;
    while(pos<end&&b[pos]==255)pos++;wire_require(pos<end,"Truncated JPEG scan");
    unsigned code=b[pos];if(code==0||(code>=0xD0&&code<=0xD7)){pos++;continue;}pos=marker_start;break;
   }
  }
 }
 wire_require(width>0&&height>0&&jpeg_end>0,"Truncated JPEG end marker");return {width,height,WireBytes(b.begin()+24,b.begin()+jpeg_end),true};
}
}
