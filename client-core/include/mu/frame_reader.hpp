#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace mu::core {
// Input must already have transport decoding applied. This does not decrypt C3/C4.
// Length/type rules follow native/client/client.cpp Socket::try_frame.
class FrameReader {
public:
 using Bytes = std::vector<std::uint8_t>;
 static constexpr std::size_t max_frame = 16384;
 static constexpr std::size_t max_buffer = 262144;
 std::vector<Bytes> feed(const Bytes& bytes) {
  require(!failed_, "Frame reader is failed; create a new session");
  require(bytes.size() <= max_buffer - pending_.size(), "Inbound buffering limit exceeded");
  pending_.insert(pending_.end(), bytes.begin(), bytes.end());
  std::vector<Bytes> frames;
  std::size_t offset = 0;
  while (offset < pending_.size()) {
   auto type = pending_[offset];
   require(type >= 0xC1 && type <= 0xC4, "Unexpected frame type");
   const bool wide = type == 0xC2 || type == 0xC4;
   const std::size_t header = wide ? 3 : 2;
   const auto left = pending_.size() - offset;
   if (left < header) break;
   const std::size_t size = wide ? (std::size_t(pending_[offset+1]) << 8) | pending_[offset+2] : pending_[offset+1];
   require(size >= header + 1 && size <= max_frame, "Invalid inbound frame length");
   if (left < size) break;
   frames.emplace_back(pending_.begin()+offset, pending_.begin()+offset+size);
   offset += size;
  }
  if (offset) pending_.erase(pending_.begin(), pending_.begin()+offset);
  return frames;
 }
 void finish() {
  require(!failed_, "Frame reader is failed; create a new session");
  require(pending_.empty(), "Peer closed connection before complete packet");
 }
 std::size_t pending_size() const { return pending_.size(); }
 bool failed() const { return failed_; }
private:
 Bytes pending_;
 bool failed_ = false;
 void require(bool ok, const char* message) {
  if (!ok) { failed_ = true; pending_.clear(); throw std::runtime_error(message); }
 }
};
}
