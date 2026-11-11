#pragma once
#include "mu/platform.hpp"
#include "frame_decoder.hpp"
#include <array>
#include <deque>
#include <string>
#include <vector>

#define MAX_MAIN_PACKET_SIZE 2048
#define MAX_SIDE_PACKET_SIZE 2048
#define MAX_QUEUE_SIZE 2048

struct IO_RECV_CONTEXT { mu::connect::FrameDecoder decoder; };
struct IO_SEND_CONTEXT { std::vector<BYTE> pending; std::size_t offset=0; };

// Native replacement of the original Windows IOCP transport. The reused
// protocol/registry/client classes still call the same Start/DataSend/Disconnect.
class CSocketManager {
public:
  CSocketManager();
  ~CSocketManager();
  bool Start(WORD port);
  void Clean();
  bool DataSend(int index,BYTE* message,int size);
  void Disconnect(int index);
  DWORD GetQueueSize();
  void SetBindAddress(const std::string& address);
  void Poll(int timeoutMilliseconds);
private:
  struct PendingPacket { int index; std::uint64_t generation; std::vector<BYTE> bytes; };
  int listen_=-1;
  bool started_=false;
  std::string bindAddress_="0.0.0.0";
  std::array<std::uint64_t,100> generations_{};
  std::deque<PendingPacket> queue_;
  void Accept();
  void Receive(int index);
  void Flush(int index);
  void Dispatch();
};
extern CSocketManager gSocketManager;
