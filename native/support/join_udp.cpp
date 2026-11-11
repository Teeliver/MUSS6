#include "stdafx.h"
#include "SocketManagerUdp.h"
CSocketManagerUdp gSocketManagerUdp;
CSocketManagerUdp::CSocketManagerUdp():m_socket(INVALID_SOCKET),m_ServerRecvThread(nullptr),m_RecvSize(0),m_SendSize(0){}
CSocketManagerUdp::~CSocketManagerUdp(){Clean();}
bool CSocketManagerUdp::Start(WORD){return false;} // Join sends heartbeats, never starts a UDP listener.
bool CSocketManagerUdp::Connect(char* address,WORD port){
 Clean();m_SocketAddr={};m_SocketAddr.sin_family=AF_INET;m_SocketAddr.sin_port=htons(port);
 if(inet_pton(AF_INET,address,&m_SocketAddr.sin_addr)!=1)return false;
 m_socket=::socket(AF_INET,SOCK_DGRAM,0);return m_socket!=INVALID_SOCKET;
}
void CSocketManagerUdp::Clean(){if(m_socket!=INVALID_SOCKET)::close(m_socket);m_socket=INVALID_SOCKET;}
bool CSocketManagerUdp::DataRecv(){return false;}
bool CSocketManagerUdp::DataSend(BYTE* data,int size){return m_socket!=INVALID_SOCKET&&size>0&&size<=MAX_UDP_PACKET_SIZE&&::sendto(m_socket,data,size,0,reinterpret_cast<sockaddr*>(&m_SocketAddr),sizeof(m_SocketAddr))==size;}
DWORD CSocketManagerUdp::ServerRecvThread(CSocketManagerUdp*){return 0;}
