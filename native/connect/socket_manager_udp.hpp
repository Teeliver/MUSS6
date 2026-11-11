#pragma once
#include "mu/platform.hpp"
#include <string>

#define MAX_UDP_PACKET_SIZE 4096
class CSocketManagerUdp {
public:
  CSocketManagerUdp();
  ~CSocketManagerUdp();
  bool Start(WORD port);
  void Clean();
  void SetBindAddress(const std::string& address);
  void Poll();
private:
  int socket_=-1;
  std::string bindAddress_="0.0.0.0";
};
extern CSocketManagerUdp gSocketManagerUdp;
