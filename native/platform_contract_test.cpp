#include "mu/platform.hpp"
#include <filesystem>
#include <fcntl.h>
int main(){
 const auto root=std::filesystem::temp_directory_path()/("mu-platform-test-"+std::to_string(getpid()));
 std::filesystem::create_directory(root);
 const auto ini=(root/"config.ini").string();
 {std::ofstream file(ini);file<<"[MixedCase]\nName=\"server value\"\nPort=0xDA69\nSigned=-1\nBad=oops\n";}
 char text[32]{};GetPrivateProfileString("mixedcase","name","missing",text,sizeof(text),ini.c_str());assert(std::string(text)=="server value");
 assert(GetPrivateProfileInt("mixedcase","port",0,ini.c_str())==55913);
 assert(GetPrivateProfileInt("mixedcase","signed",0,ini.c_str())==0xffffffffu);
 assert(GetPrivateProfileInt("mixedcase","bad",7,ini.c_str())==0);
 assert(GetPrivateProfileInt("mixedcase","absent",7,ini.c_str())==7);
 assert(WritePrivateProfileString("MixedCase","Port","55901",ini.c_str()));
 assert(GetPrivateProfileInt("mixedcase","port",0,ini.c_str())==55901);
 char format[64]{};assert(wsprintf(format,"%I64d",9223372036854775807ll)==19);assert(std::string(format)=="9223372036854775807");
 char bounded[4]={'x','x','x','x'};assert(wsprintf(bounded,"%s","overflow")<0);assert(bounded[0]==0);
 SYSTEMTIME epoch{1970,1,4,1,0,0,0,123};FILETIME ticks{};assert(SystemTimeToFileTime(&epoch,&ticks));
 assert(((std::uint64_t(ticks.dwHighDateTime)<<32)|ticks.dwLowDateTime)==116444736001230000ull);
 SYSTEMTIME round{};assert(FileTimeToSystemTime(&ticks,&round));assert(round.wYear==1970&&round.wMonth==1&&round.wDay==1&&round.wMilliseconds==123);
 const auto binary=(root/"binary.dat").string();HANDLE file=CreateFile(binary.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);assert(file!=INVALID_HANDLE_VALUE);
 const BYTE bytes[]={0,0xff,1,0,8};DWORD count=0;assert(WriteFile(file,bytes,sizeof(bytes),&count,nullptr)&&count==5);
 assert(GetFileSize(file,nullptr)==5);assert(SetFilePointer(file,0,nullptr,FILE_BEGIN)==0);
 BYTE result[5]{};assert(ReadFile(file,result,sizeof(result),&count,nullptr)&&count==5);assert(std::memcmp(bytes,result,5)==0);assert(CloseHandle(file));
 WIN32_FIND_DATA entry{};HANDLE find=FindFirstFile((root/"*.DAT").c_str(),&entry);assert(find!=INVALID_HANDLE_VALUE);assert(std::string(entry.cFileName)=="binary.dat");assert(!FindNextFile(find,&entry));assert(FindClose(find));
 std::filesystem::remove_all(root);
 return 0;
}
