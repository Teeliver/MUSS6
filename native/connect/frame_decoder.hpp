#pragma once
#include "mu/platform.hpp"
#include <functional>
#include <stdexcept>
#include <vector>

namespace mu::connect {
// ConnectServer's original C1/C2 framing, with complete-header validation.
class FrameDecoder {
public:
  explicit FrameDecoder(std::size_t maximum=2048):maximum_(maximum) {}
  void Reset() { pending_.clear(); }
  std::size_t Buffered() const { return pending_.size(); }
  std::size_t Capacity() const { return maximum_-pending_.size(); }
  void Feed(const BYTE* bytes,std::size_t size,const std::function<void(std::vector<BYTE>)>& receive) {
    if(size>Capacity()) throw std::runtime_error("Connect TCP receive buffer limit exceeded");
    pending_.insert(pending_.end(),bytes,bytes+size);
    while(!pending_.empty()) {
      if(pending_[0]!=0xC1 && pending_[0]!=0xC2) throw std::runtime_error("Connect frame type must be C1/C2");
      const std::size_t header=pending_[0]==0xC1?3:4;
      if(pending_.size()<header) return;
      const std::size_t length=pending_[0]==0xC1?pending_[1]:(std::size_t(pending_[1])<<8)|pending_[2];
      if(length<header || length>maximum_) throw std::runtime_error("Connect frame length out of range");
      if(pending_.size()<length) return;
      std::vector<BYTE> packet(pending_.begin(),pending_.begin()+length);
      pending_.erase(pending_.begin(),pending_.begin()+length);
      receive(std::move(packet));
    }
  }
private:
  std::size_t maximum_;
  std::vector<BYTE> pending_;
};
}
