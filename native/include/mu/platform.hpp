#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <cstddef>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include "mu/calendar.hpp"

// Windows x86 wire scalars must retain their sizes on macOS ARM64/LP64.
using BYTE=std::uint8_t;
using LPBYTE=BYTE*;
using PBYTE=BYTE*;
using WORD=std::uint16_t;
using DWORD=std::uint32_t;
using LONG=std::int32_t;
using LPLONG=LONG*;
using ULONG=std::uint32_t;
using UINT=std::uint32_t;
using USHORT=std::uint16_t;
using BOOL=std::int32_t;
using BOOLEAN=BYTE;
using WAITORTIMERCALLBACK=void(*)(void*,BOOLEAN);
using QWORD=unsigned long long;
using LONGLONG=long long;
using ULONGLONG=unsigned long long;
using INT=int;
using FLOAT=float;
using CHAR=char;
using TCHAR=char;
using LPCSTR=const char*;
using LPSTR=char*;
using LPCTSTR=const char*;
using LPTSTR=char*;
using LPVOID=void*;
using LPCVOID=const void*;
using PVOID=void*;
using LPDWORD=DWORD*;
using DWORD_PTR=std::uintptr_t;
using ULONG_PTR=std::uintptr_t;
using SIZE_T=std::size_t;
using WPARAM=std::uintptr_t;
using LPARAM=std::intptr_t;
using LRESULT=std::intptr_t;
using HANDLE=void*;
using HWND=void*;
using HINSTANCE=void*;
using HDC=void*;
using HBRUSH=void*;
using HFONT=void*;
using HBITMAP=void*;
using HMENU=void*;
using SOCKET=int;
using SOCKADDR=sockaddr;
using SOCKADDR_IN=sockaddr_in;
using GROUP=std::uint32_t;
using LPQOS=void*;
using SQLRETURN=std::int16_t;
using SQLHANDLE=void*;
using SQLINTEGER=std::int32_t;
using SQLSMALLINT=std::int16_t;
using SQLCHAR=unsigned char;
using SQLLEN=std::intptr_t;
inline constexpr SQLRETURN SQL_SUCCESS=0,SQL_SUCCESS_WITH_INFO=1,SQL_ERROR=-1,SQL_NO_DATA=100;
inline constexpr SQLLEN SQL_NULL_DATA=-1;
inline bool SQL_SUCCEEDED(SQLRETURN value){return value==SQL_SUCCESS||value==SQL_SUCCESS_WITH_INFO;}
inline constexpr int INVALID_SOCKET=-1,SOCKET_ERROR=-1;
inline constexpr BOOL TRUE=1,FALSE=0;
inline constexpr std::size_t _TRUNCATE=std::numeric_limits<std::size_t>::max();
inline constexpr DWORD INFINITE=0xffffffffu;
inline constexpr int MAX_PATH=260;
#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1))
#define WINAPI
#define CALLBACK
#define APIENTRY
#define IN
#define OUT
#define FAR
#define __stdcall
#define __cdecl
#define __fastcall
#define __forceinline inline
#define ZeroMemory(destination,size) std::memset((destination),0,(size))
#define CopyMemory(destination,source,size) std::memcpy((destination),(source),(size))
#define MAKEWORD(low,high) static_cast<WORD>((BYTE(low))|(WORD(BYTE(high))<<8))
#define LOBYTE(value) static_cast<BYTE>(value)
#define HIBYTE(value) static_cast<BYTE>((value)>>8)
#define LOWORD(value) static_cast<WORD>(value)
#define HIWORD(value) static_cast<WORD>((value)>>16)
#define _countof(value) (sizeof(value)/sizeof((value)[0]))

struct OVERLAPPED{std::uintptr_t Internal=0,InternalHigh=0;DWORD Offset=0,OffsetHigh=0;HANDLE hEvent=nullptr;};
using WSAOVERLAPPED=OVERLAPPED;
struct WSABUF{ULONG len;char* buf;};
using LPWSABUF=WSABUF*;
struct CRITICAL_SECTION{std::recursive_mutex mutex;};
inline void InitializeCriticalSection(CRITICAL_SECTION*){}
inline void DeleteCriticalSection(CRITICAL_SECTION*){}
inline void EnterCriticalSection(CRITICAL_SECTION* value){value->mutex.lock();}
inline void LeaveCriticalSection(CRITICAL_SECTION* value){value->mutex.unlock();}
inline LONG InterlockedIncrement(LONG* value){return __atomic_add_fetch(value,1,__ATOMIC_SEQ_CST);}
inline LONG InterlockedDecrement(LONG* value){return __atomic_sub_fetch(value,1,__ATOMIC_SEQ_CST);}
inline LONG InterlockedExchange(LONG* value,LONG next){return __atomic_exchange_n(value,next,__ATOMIC_SEQ_CST);}
inline LONG InterlockedExchange(DWORD* value,DWORD next){return static_cast<LONG>(__atomic_exchange_n(value,next,__ATOMIC_SEQ_CST));}
inline long InterlockedExchange(long* value,long next){return __atomic_exchange_n(value,long(LONG(next)),__ATOMIC_SEQ_CST);}
inline LONG InterlockedExchangeAdd(LONG* value,LONG delta){return __atomic_fetch_add(value,delta,__ATOMIC_SEQ_CST);}
inline DWORD InterlockedExchangeAdd(DWORD* value,DWORD delta){return __atomic_fetch_add(value,delta,__ATOMIC_SEQ_CST);}
inline long InterlockedExchangeAdd(long* value,long delta){return __atomic_fetch_add(value,delta,__ATOMIC_SEQ_CST);}
inline int closesocket(SOCKET socket){return ::close(socket);}
inline int WSAGetLastError(){return errno;}
inline constexpr int WSAEWOULDBLOCK=EWOULDBLOCK;
inline constexpr int FD_READ=1,FD_WRITE=2,FD_CLOSE=32;
inline void Sleep(DWORD millis){std::this_thread::sleep_for(std::chrono::milliseconds(millis));}
DWORD GetTickCount();
using LARGE_INTEGER=union{struct{DWORD LowPart;LONG HighPart;};long long QuadPart;};
BOOL QueryPerformanceCounter(LARGE_INTEGER* value);
BOOL QueryPerformanceFrequency(LARGE_INTEGER* value);

struct SYSTEMTIME{WORD wYear,wMonth,wDayOfWeek,wDay,wHour,wMinute,wSecond,wMilliseconds;};
using _SYSTEMTIME=SYSTEMTIME;
struct FILETIME{DWORD dwLowDateTime,dwHighDateTime;};
using _FILETIME=FILETIME;
BOOL SystemTimeToFileTime(const SYSTEMTIME* value,FILETIME* output);
BOOL FileTimeToSystemTime(const FILETIME* value,SYSTEMTIME* output);
BOOL WritePrivateProfileString(const char* section,const char* key,const char* value,const char* path);
inline void OutputDebugString(const char* value){if(value)std::cerr<<value;}
struct RECT{LONG left,top,right,bottom;};
struct POINT{LONG x,y;};
void GetLocalTime(SYSTEMTIME* value);
void GetSystemTime(SYSTEMTIME* value);
DWORD GetPrivateProfileInt(const char* section,const char* key,int fallback,const char* path);
DWORD GetPrivateProfileString(const char* section,const char* key,const char* fallback,char* output,DWORD size,const char* path);
namespace mu{
 std::string native_path(const char* path);
 std::string native_format(const char* format);
 int format_into(char* output,std::size_t size,const char* format,va_list args);
}
inline int strcpy_s(char* output,std::size_t size,const char* input){
 if(!output||!size)return EINVAL;
 if(!input){output[0]=0;return EINVAL;}
 const auto length=std::strlen(input);
 if(length>=size){output[0]=0;return ERANGE;}
 std::memmove(output,input,length+1);return 0;
}
template<std::size_t N>int strcpy_s(char(&output)[N],const char* input){return strcpy_s(output,N,input);}
inline int strncpy_s(char* output,std::size_t size,const char* input,std::size_t count){
 if(!output||!size)return EINVAL;
 if(!input){output[0]=0;return EINVAL;}
 const auto length=std::min(std::strlen(input),count==_TRUNCATE?size-1:count);
 if(length>=size){output[0]=0;return ERANGE;}
 std::memmove(output,input,length);output[length]=0;return 0;
}
template<std::size_t N>int strncpy_s(char(&output)[N],const char* input,std::size_t count){return strncpy_s(output,N,input,count);}
inline int strcat_s(char* output,std::size_t size,const char* input){
 if(!output||!size)return EINVAL;
 const auto length=strnlen(output,size);
 if(length==size){output[0]=0;return EINVAL;}
 const int error=strcpy_s(output+length,size-length,input);if(error)output[0]=0;return error;
}
template<std::size_t N>int strcat_s(char(&output)[N],const char* input){return strcat_s(output,N,input);}
inline int memcpy_s(void* output,std::size_t size,const void* input,std::size_t count){if(count>size||(!output&&count)||(!input&&count))return EINVAL;std::memcpy(output,input,count);return 0;}
inline int vsprintf_s(char* output,std::size_t size,const char* format,va_list args){return mu::format_into(output,size,format,args);}
template<std::size_t N>int vsprintf_s(char(&output)[N],const char* format,va_list args){return mu::format_into(output,N,format,args);}
inline int sprintf_s(char* output,std::size_t size,const char* format,...){va_list args;va_start(args,format);int result=mu::format_into(output,size,format,args);va_end(args);return result;}
template<std::size_t N>int sprintf_s(char(&output)[N],const char* format,...){va_list args;va_start(args,format);int result=mu::format_into(output,N,format,args);va_end(args);return result;}
template<std::size_t N>int wsprintf(char(&output)[N],const char* format,...){va_list args;va_start(args,format);int result=mu::format_into(output,N,format,args);va_end(args);return result;}
// Pointer-only fallback for legacy callers; arrays select the bounded overload above.
template<class Pointer,std::enable_if_t<std::is_same_v<std::remove_reference_t<Pointer>,char*>,int> =0>
int wsprintf(Pointer&& output,const char* format,...){va_list args;va_start(args,format);const auto native=mu::native_format(format);const int result=std::vsprintf(output,native.c_str(),args);va_end(args);return result;}
inline int _stricmp(const char* left,const char* right){return strcasecmp(left,right);}
inline int _strnicmp(const char* left,const char* right,std::size_t size){return strncasecmp(left,right,size);}
inline char* _strupr(char* value){for(char* p=value;*p;p++)if(*p>='a'&&*p<='z')*p-=32;return value;}
inline char* _strlwr(char* value){for(char* p=value;*p;p++)if(*p>='A'&&*p<='Z')*p+=32;return value;}
inline int _strupr_s(char* value,std::size_t size){if(strnlen(value,size)==size)return EINVAL;_strupr(value);return 0;}
template<std::size_t N>int _strupr_s(char(&value)[N]){return _strupr_s(value,N);}
inline long long _atoi64(const char* value){return std::strtoll(value,nullptr,10);}
inline int fopen_s(FILE** output,const char* path,const char* mode){*output=std::fopen(mu::native_path(path).c_str(),mode);return *output?0:errno;}
inline int localtime_s(std::tm* output,const std::time_t* value){return localtime_r(value,output)?0:errno;}
inline int gmtime_s(std::tm* output,const std::time_t* value){return gmtime_r(value,output)?0:errno;}
inline int asctime_s(char* output,std::size_t size,const std::tm* value){if(size<26)return ERANGE;return asctime_r(value,output)?0:errno;}
inline char* strtok_s(char* value,const char* delimiters,char** context){return strtok_r(value,delimiters,context);}
inline void GetSystemTimeAsFileTime(void*)=delete;

// File APIs retained by original text loaders and append-only log code.
inline constexpr DWORD GENERIC_READ=0x80000000u,GENERIC_WRITE=0x40000000u,FILE_SHARE_READ=1;
inline constexpr DWORD OPEN_EXISTING=3,OPEN_ALWAYS=4,CREATE_ALWAYS=2,FILE_ATTRIBUTE_NORMAL=0x80,FILE_ATTRIBUTE_ARCHIVE=0x20;
inline constexpr DWORD FILE_BEGIN=0,FILE_CURRENT=1,FILE_END=2,INVALID_SET_FILE_POINTER=0xffffffffu;
inline constexpr DWORD FILE_ATTRIBUTE_DIRECTORY=0x10,HEAP_ZERO_MEMORY=8;
inline constexpr UINT GPTR=0x40;
inline HANDLE GlobalAlloc(UINT flags,SIZE_T size){return flags&GPTR?std::calloc(1,size):std::malloc(size);}
inline HANDLE GlobalFree(HANDLE memory){std::free(memory);return nullptr;}
struct WIN32_FIND_DATA{DWORD dwFileAttributes=0;char cFileName[260]{};};
HANDLE FindFirstFile(const char* pattern,WIN32_FIND_DATA* output);
BOOL FindNextFile(HANDLE handle,WIN32_FIND_DATA* output);
BOOL FindClose(HANDLE handle);
BOOL PathMatchSpec(const char* path,const char* pattern);
inline HANDLE GetProcessHeap(){return reinterpret_cast<HANDLE>(std::uintptr_t(1));}
inline void* HeapAlloc(HANDLE,DWORD flags,SIZE_T size){return flags&HEAP_ZERO_MEMORY?std::calloc(1,size):std::malloc(size);}
inline BOOL HeapFree(HANDLE,DWORD,void* value){std::free(value);return TRUE;}
HANDLE CreateFile(const char* path,DWORD access,DWORD share,void*,DWORD disposition,DWORD attributes,HANDLE);
BOOL CloseHandle(HANDLE handle);
DWORD GetFileSize(HANDLE handle,DWORD* high);
DWORD SetFilePointer(HANDLE handle,LONG offset,LONG* high,DWORD origin);
BOOL ReadFile(HANDLE handle,void* output,DWORD size,DWORD* read,OVERLAPPED*);
BOOL WriteFile(HANDLE handle,const void* input,DWORD size,DWORD* written,OVERLAPPED*);
BOOL CreateDirectory(const char* path,void*);
DWORD GetLastError();
[[noreturn]]inline void ExitProcess(UINT code){std::exit(code?int(code):EXIT_FAILURE);}
inline constexpr UINT MB_OK=0,MB_ICONERROR=0x10,MB_ICONINFORMATION=0x40,MB_YESNO=4,MB_ICONQUESTION=0x20;
inline constexpr int IDOK=1,IDYES=6,IDNO=7;
inline int MessageBox(HWND,const char* message,const char* title,UINT){std::cerr<<(title?title:"")<<": "<<(message?message:"")<<'\n';return IDOK;}
inline BOOL SetWindowText(HWND,const char*){return TRUE;}
static_assert(sizeof(BYTE)==1&&sizeof(WORD)==2&&sizeof(DWORD)==4&&sizeof(LONG)==4&&sizeof(QWORD)==8,"Windows wire scalar widths");
