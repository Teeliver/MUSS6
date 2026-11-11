// Connection.h: interface for the CConnection class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if defined(MU_NATIVE_MACOS)

#include <mu/platform.hpp>
#include <memory>
#include <string>

#define MAX_BUFF_SIZE 524288

// Raw JoinServer/DataServer TCP connection. The native runtime polls and dispatches
// these events on its gameplay thread; this class creates no callback threads.
class CConnection
{
public:
	using ProtocolCallback = void (*)(BYTE,BYTE*,int);
	enum NativeEvent : unsigned { NativeRead = 1, NativeWrite = 2, NativeClose = 4 };
	CConnection();
	virtual ~CConnection();
	CConnection(const CConnection&) = delete;
	CConnection& operator=(const CConnection&) = delete;
	void Init(HWND hwnd,void* function);
	void Init(HWND hwnd,ProtocolCallback function);
	bool Connect(char* IpAddress,WORD port,DWORD WinMsg);
	void Disconnect();
	bool CheckState();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg,int size,int aIndex);
	bool DataSendEx();
	// NativeClose remains pending until NativeTakeCloseEvent consumes it. Dispatch
	// readable data first; DataRecv may discover EOF and set the close event itself.
	unsigned NativePollEvents(int timeoutMs = 0);
	bool NativeTakeCloseEvent();
	void NativeSetConnectTimeout(int timeoutMs);
	std::string NativeLastError();
	std::size_t NativePendingSendBytes();
private:
	struct NativeState;
	std::unique_ptr<NativeState> m_native;
};

#else

#include "CriticalSection.h"

#define MAX_BUFF_SIZE 524288

class CConnection
{
public:
	CConnection();
	virtual ~CConnection();
	void Init(HWND hwnd,void* function);
	bool Connect(char* IpAddress,WORD port,DWORD WinMsg);
	void Disconnect();
	bool CheckState();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg,int size,int aIndex);
	bool DataSendEx();
private:
	HWND m_hwnd;
	SOCKET m_socket;
	BYTE m_RecvBuff[MAX_BUFF_SIZE];
	int m_RecvSize;
	BYTE m_SendBuff[MAX_BUFF_SIZE];
	int m_SendSize;
	void (*wsProtocolCore)(BYTE,BYTE*,int);
	CCriticalSection m_critical;
};

#endif
