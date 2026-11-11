#pragma once

#if defined(MU_NATIVE_MACOS)
enum eLogColor { LOG_BLACK=0, LOG_RED=1, LOG_GREEN=2, LOG_BLUE=3, LOG_PINK=4, LOG_CIAN=5, LOG_ORANGE=6, LOG_WHITE=7 };
#else
#include "ServerDisplayer.h"
#endif

void PacketArgumentEncrypt(char* out_buff,char* in_buff,int size);
void ErrorMessageBox(char* message,...);
void LogAdd(eLogColor color,char* text,...);
void ConnectServerTimeoutProc();
int GetUserCount();
int GetFreeClientIndex();
int SearchFreeClientIndex(int* index,int MinIndex,int MaxIndex,DWORD MinTime);

extern int gClientCount;
