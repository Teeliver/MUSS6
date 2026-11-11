#pragma once

#include <mu/platform.hpp>
#include <memory>
#include <string>

#define MAX_UDP_PACKET_SIZE 4096

class CSocketManagerUdp
{
public:
	CSocketManagerUdp();
	virtual ~CSocketManagerUdp();
	CSocketManagerUdp(const CSocketManagerUdp&) = delete;
	CSocketManagerUdp& operator=(const CSocketManagerUdp&) = delete;
	bool Start(WORD port);
	bool Connect(char* address,WORD port);
	void Clean();
	bool DataRecv();
	bool DataSend(BYTE* message,int size);
	static DWORD WINAPI ServerRecvThread(CSocketManagerUdp* manager);
	void SetBindAddress(const char* address);
	bool NativePoll(int timeoutMs = 0);
	WORD NativePort();
	std::string NativeLastError();
private:
	struct NativeState;
	std::unique_ptr<NativeState> m_native;
};

extern CSocketManagerUdp gSocketManagerUdp;
