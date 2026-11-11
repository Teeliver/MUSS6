#pragma once

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP

#define GAMESERVER_VERSION "[2.0.7] %s (EX%03d-%s) [%s]"

#ifndef GAMESERVER_EXTRA
#define GAMESERVER_EXTRA 1
#endif

#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

#ifndef GAMESERVER_LANGUAGE
#define GAMESERVER_LANGUAGE 1
#endif

#ifndef GAMESERVER_SHOP
#define GAMESERVER_SHOP 0
#endif

#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 1
#endif

// System Include
#if defined(MU_NATIVE_MACOS)
#include "mu/platform.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <string>
#include <lua.hpp>
#else
#include <windows.h>
#include <winsock2.h>
#include <mswSock.h>
#include <commctrl.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <Rpc.h>
#include <algorithm>
#include <string>
#include <atltime.h>
#include <dbghelp.h>
#include <Psapi.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include "..\\..\\..\\Util\\lua\\include\\lua.hpp"
//#include "..\\..\\..\\Util\\pugixml\\pugixml.hpp"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"..\\..\\..\\Util\\lua\\lua52.lib")

#if(GAMESERVER_UPDATE>=701)
#pragma comment(lib,"..\\..\\..\\Util\\cryptopp\\Release\\cryptlib.lib")
#pragma comment(lib,"..\\..\\..\\Util\\mapm\\mapm.lib")
#endif

#endif // MU_NATIVE_MACOS system headers

extern char CustomerName[32];
extern WORD CustomerLicenseId;

#if !defined(MU_NATIVE_MACOS)
typedef unsigned __int64 QWORD;
#endif

#if defined(MU_NATIVE_MACOS)
// Windows x86 protection markers have no machine-code equivalent on native macOS.
#define PROTECT_START
#define PROTECT_FINAL
#else
#define PROTECT_START _asm _emit 0xEB _asm _emit 0x0E _asm _emit 0x14 _asm _emit 0x28 _asm _emit 0xCD _asm _emit 0xDB _asm _emit 0x50 _asm _emit 0x63 _asm _emit 0xAF _asm _emit 0x22 _asm _emit 0x75 _asm _emit 0xEE _asm _emit 0xB5 _asm _emit 0xD8 _asm _emit 0x43 _asm _emit 0x72
#define PROTECT_FINAL _asm _emit 0xEB _asm _emit 0x0E _asm _emit 0x37 _asm _emit 0xF2 _asm _emit 0xCD _asm _emit 0xDB _asm _emit 0x50 _asm _emit 0x63 _asm _emit 0xAF _asm _emit 0x22 _asm _emit 0x75 _asm _emit 0xEE _asm _emit 0xB5 _asm _emit 0xD8 _asm _emit 0x85 _asm _emit 0x61
#endif // MU_NATIVE_MACOS protection markers



/* update changelog */

// Atualização sistema de noticia - Notice.cpp