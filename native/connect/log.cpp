#include "stdafx.h"
#include "Log.h"

CLog gLog;
CLog::CLog():m_count(0) { std::memset(m_LogInfo,0,sizeof(m_LogInfo)); }
CLog::~CLog()=default;
void CLog::AddLog(BOOL active,char* directory) {
  if(m_count>=MAX_LOG) return;
  auto& log=m_LogInfo[m_count++]; log.Active=active;
  strcpy_s(log.Directory,directory);
}
void CLog::Output(eLogType type,char* text,...) {
  va_list args; va_start(args,text);
  std::fprintf(stderr,"[ConnectServer:%d] ",int(type));
  std::vfprintf(stderr,text,args); std::fputc('\n',stderr);
  va_end(args);
}
