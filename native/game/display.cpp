#include "stdafx.h"
#include "ServerDisplayer.h"
#include "ServerInfo.h"
#include "Log.h"
CServerDisplayer gServerDisplayer;
CServerDisplayer::CServerDisplayer():m_hwnd(nullptr),m_log{},m_count(0){}
CServerDisplayer::~CServerDisplayer()=default;
void CServerDisplayer::Init(HWND window){
 m_hwnd=window;
 gLog.AddLog(1,"LOG");
 gLog.AddLog(gServerInfo.m_WriteChatLog,"./LOG/CHAT_LOG");
 gLog.AddLog(gServerInfo.m_WriteCommandLog,"./LOG/COMMAND_LOG");
 gLog.AddLog(gServerInfo.m_WriteTradeLog,"./LOG/TRADE_LOG");
 gLog.AddLog(gServerInfo.m_WriteConnectLog,"./LOG/CONNECT_LOG");
 gLog.AddLog(gServerInfo.m_WriteHackLog,"./LOG/HACK_LOG");
 gLog.AddLog(gServerInfo.m_WriteCashShopLog,"./LOG/CASH_SHOP_LOG");
 gLog.AddLog(gServerInfo.m_WriteChaosMixLog,"./LOG/CHAOS_MIX_LOG");
 gLog.AddLog(gServerInfo.m_WriteScriptLog,"./LOG/SCRIPT_LOG");
 gLog.AddLog(1,"./LOG/PROTOCOL_LOG");
}
// The headless runtime reports status in its CLI. GDI paint operations have no native surface.
void CServerDisplayer::Run(){}
void CServerDisplayer::SetWindowName(){}
void CServerDisplayer::LogTextPaint(){}
void CServerDisplayer::PaintBarInfo(){}
void CServerDisplayer::LogAddText(eLogColor,char* text,int size){std::cerr.write(text,size);std::cerr<<'\n';}
