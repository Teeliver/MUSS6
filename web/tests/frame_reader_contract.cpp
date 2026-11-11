#include "mu/frame_reader.hpp"
#include <algorithm>
#include <iostream>
#include <string>
using Reader=mu::core::FrameReader;
using Bytes=Reader::Bytes;
void check(bool ok){if(!ok)throw std::runtime_error("Contract assertion failed");}
template<class F> void rejects(F f){bool threw=false;try{f();}catch(const std::runtime_error&){threw=true;}if(!threw)throw std::runtime_error("Expected rejection");}
int main(){
 const std::vector<Bytes> fixtures={{0xC1,4,0,1},{0xC2,0,5,0xF4,0x06},{0xC3,4,0,0},{0xC4,0,5,0,0}};
 Bytes stream;for(auto& f:fixtures)stream.insert(stream.end(),f.begin(),f.end());
 // Every split boundary, plus all bytes delivered individually.
 for(std::size_t cut=0;cut<=stream.size();cut++){
  Reader r;auto out=r.feed(Bytes(stream.begin(),stream.begin()+cut));auto rest=r.feed(Bytes(stream.begin()+cut,stream.end()));out.insert(out.end(),rest.begin(),rest.end());check(out==fixtures);r.finish();
 }
 Reader single;std::vector<Bytes> out;for(auto b:stream){auto f=single.feed({b});out.insert(out.end(),f.begin(),f.end());}check(out==fixtures);single.finish();
 Reader owner;Bytes input=fixtures[0];auto frames=owner.feed(input);input[2]=99;check(frames[0]==fixtures[0]);
 Bytes wide(16384,0x71);wide[0]=0xC2;wide[1]=0x40;wide[2]=0;Reader max;check(max.feed(wide)==std::vector<Bytes>{wide});max.finish();
 for(auto bytes:std::vector<Bytes>{{0},{0xC1,2},{0xC2,0,3},{0xC4,0x40,1}}){Reader r;rejects([&]{r.feed(bytes);});check(r.failed()&&r.pending_size()==0);rejects([&]{r.feed(fixtures[0]);});}
 for(auto bytes:std::vector<Bytes>{{0xC1},{0xC2,0},{0xC3,4,0}}){Reader r;r.feed(bytes);rejects([&]{r.finish();});}
 Reader cap;rejects([&]{cap.feed(Bytes(Reader::max_buffer+1));});
 // A malformed trailing frame invalidates the entire feed result (no partial delivery).
 Reader tail;Bytes invalid=fixtures[0];invalid.push_back(0);rejects([&]{tail.feed(invalid);});
 std::cout<<"frame-reader contract passed\n";
}
