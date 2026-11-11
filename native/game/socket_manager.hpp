#pragma once

#include <mu/platform.hpp>
#include <memory>
#include <string>

#define MAX_MAIN_PACKET_SIZE 8192
#define MAX_SIDE_PACKET_SIZE 16384
#define MAX_SERVER_WORKER_THREAD 8
#define MAX_IO_OPERATION 2
#define IO_RECV 0
#define IO_SEND 1

// Keep the source layouts: MemoryAllocator owns each user's PER_SOCKET_CONTEXT.
struct IO_MAIN_BUFFER { BYTE buff[MAX_MAIN_PACKET_SIZE]; int size; };
struct IO_SIDE_BUFFER { BYTE buff[MAX_SIDE_PACKET_SIZE]; int size; };
struct IO_CONTEXT { WSAOVERLAPPED overlapped; WSABUF wsabuf; int IoType; int IoSize; };
struct IO_RECV_CONTEXT
{
	WSAOVERLAPPED overlapped; WSABUF wsabuf; int IoType; int IoSize;
	IO_MAIN_BUFFER IoMainBuffer;
};
struct IO_SEND_CONTEXT
{
	WSAOVERLAPPED overlapped; WSABUF wsabuf; int IoType; int IoSize;
	IO_MAIN_BUFFER IoMainBuffer; IO_SIDE_BUFFER IoSideBuffer;
};
struct PER_SOCKET_CONTEXT
{
	SOCKET Socket; int Index; IO_RECV_CONTEXT IoRecvContext; IO_SEND_CONTEXT IoSendContext;
};

class CSocketManager
{
public:
	CSocketManager();
	virtual ~CSocketManager();
	CSocketManager(const CSocketManager&) = delete;
	CSocketManager& operator=(const CSocketManager&) = delete;
	bool Start(WORD port);
	void Clean();
	bool CreateListenSocket();
	// Obsolete IOCP/thread entry points fail explicitly with ENOTSUP on POSIX.
	bool CreateCompletionPort();
	bool CreateAcceptThread();
	bool CreateWorkerThread();
	bool CreateServerQueue();
	bool DataRecv(int index,IO_MAIN_BUFFER* buffer);
	bool DataSend(int index,BYTE* message,int size);
	void Disconnect(int index);
	void OnRecv(int index,DWORD size,IO_RECV_CONTEXT* context);
	void OnSend(int index,DWORD size,IO_SEND_CONTEXT* context);
	static int CALLBACK ServerAcceptCondition(LPWSABUF callerId,LPWSABUF callerData,LPQOS sqos,LPQOS gqos,LPWSABUF calleeId,LPWSABUF calleeData,GROUP* group,CSocketManager* manager);
	static DWORD WINAPI ServerAcceptThread(CSocketManager* manager);
	static DWORD WINAPI ServerWorkerThread(CSocketManager* manager);
	static DWORD WINAPI ServerQueueThread(CSocketManager* manager);
	DWORD GetQueueSize();
	void SetBindAddress(const char* address);
	// Poll I/O and dispatch queued ProtocolCore callbacks on the calling thread.
	bool NativePoll(int timeoutMs = 0);
	WORD NativePort();
	std::string NativeLastError();
private:
	struct NativeState;
	std::unique_ptr<NativeState> m_native;
};

extern CSocketManager gSocketManager;
