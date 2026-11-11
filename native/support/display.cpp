#include "stdafx.h"
#include "ServerDisplayer.h"
CServerDisplayer gServerDisplayer;
CServerDisplayer::CServerDisplayer():m_hwnd(nullptr),m_count(0){}
CServerDisplayer::~CServerDisplayer()=default;
void CServerDisplayer::Init(HWND window){m_hwnd=window;}
void CServerDisplayer::Run(){}
void CServerDisplayer::SetWindowName(){}
void CServerDisplayer::LogTextPaint(){}
void CServerDisplayer::PaintBarInfo(){}
void CServerDisplayer::LogAddText(eLogColor,char* text,int size){std::cerr.write(text,size);std::cerr<<'\n';}
