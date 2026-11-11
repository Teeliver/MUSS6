#pragma once
#include "mu/platform.hpp"
#include <vector>

namespace mu::connect {
// Complete datagrams only: no concatenation of fragments from different senders.
std::vector<std::vector<BYTE>> DecodeLiveDatagram(const BYTE* bytes,std::size_t size);
}
