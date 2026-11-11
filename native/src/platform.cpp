#include "mu/platform.hpp"
#include <filesystem>
#include <fcntl.h>
#include <sys/stat.h>
#include <fnmatch.h>

namespace{
 std::string trim(std::string value){const auto first=value.find_first_not_of(" \t\r\n"),last=value.find_last_not_of(" \t\r\n");return first==std::string::npos?"":value.substr(first,last-first+1);}
 std::string fold(std::string value){for(auto& c:value)if(c>='A'&&c<='Z')c+=32;return value;}
 bool ini_value(const char* path,const char* section,const char* key,std::string& output){
  std::ifstream input(mu::native_path(path),std::ios::binary);if(!input)return false;
  std::string line,current;const auto wanted_section=fold(section?section:""),wanted_key=fold(key?key:"");
  while(std::getline(input,line)){
   line=trim(line);if(line.empty()||line[0]==';'||line[0]=='#')continue;
   if(line[0]=='['){const auto end=line.find(']');if(end!=std::string::npos)current=fold(trim(line.substr(1,end-1)));continue;}
   if(current!=wanted_section)continue;const auto equals=line.find('=');if(equals==std::string::npos||fold(trim(line.substr(0,equals)))!=wanted_key)continue;
   output=trim(line.substr(equals+1));if(output.size()>=2&&((output.front()=='"'&&output.back()=='"')||(output.front()=='\''&&output.back()=='\'')))output=output.substr(1,output.size()-2);return true;
  }
  return false;
 }
 struct NativeFile{int fd;};
 int fd(HANDLE value){return value&&value!=INVALID_HANDLE_VALUE?static_cast<NativeFile*>(value)->fd:-1;}
 void time_parts(SYSTEMTIME* output,bool utc){
  const auto now=std::chrono::system_clock::now();const auto epoch=std::chrono::system_clock::to_time_t(now);std::tm value{};
  if(utc)gmtime_r(&epoch,&value);else localtime_r(&epoch,&value);
  *output={WORD(value.tm_year+1900),WORD(value.tm_mon+1),WORD(value.tm_wday),WORD(value.tm_mday),WORD(value.tm_hour),WORD(value.tm_min),WORD(value.tm_sec),WORD(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()%1000)};
 }
}
namespace mu{
 std::string native_path(const char* path){std::string value=path?path:"";std::replace(value.begin(),value.end(),'\\','/');return value;}
 std::string native_format(const char* format){
  std::string value=format?format:"";std::size_t at=0;
  while((at=value.find("I64",at))!=std::string::npos){value.replace(at,3,"ll");at+=2;}
  return value;
 }
 int format_into(char* output,std::size_t size,const char* format,va_list args){
  if(!output||!size||!format)return -1;const auto native=native_format(format);const int count=std::vsnprintf(output,size,native.c_str(),args);
  if(count<0||std::size_t(count)>=size){output[0]=0;return -1;}return count;
 }
}
DWORD GetTickCount(){static const auto boot=std::chrono::steady_clock::now();return DWORD(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-boot).count());}
BOOL QueryPerformanceCounter(LARGE_INTEGER* value){value->QuadPart=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();return TRUE;}
BOOL QueryPerformanceFrequency(LARGE_INTEGER* value){value->QuadPart=1000000000;return TRUE;}
void GetLocalTime(SYSTEMTIME* value){time_parts(value,false);}
void GetSystemTime(SYSTEMTIME* value){time_parts(value,true);}
DWORD GetPrivateProfileInt(const char* section,const char* key,int fallback,const char* path){
 std::string value;if(!ini_value(path,section,key,value))return DWORD(fallback);
 char* end=nullptr;const bool hex=value.size()>2&&value[0]=='0'&&(value[1]=='x'||value[1]=='X');const long result=std::strtol(value.c_str(),&end,hex?16:10);return end==value.c_str()?0:DWORD(result);
}
DWORD GetPrivateProfileString(const char* section,const char* key,const char* fallback,char* output,DWORD size,const char* path){
 if(!size||!output)return 0;std::string value;if(!ini_value(path,section,key,value))value=fallback?fallback:"";const auto count=std::min(value.size(),std::size_t(size-1));std::memcpy(output,value.data(),count);output[count]=0;return DWORD(count);
}
HANDLE CreateFile(const char* path,DWORD access,DWORD,void*,DWORD disposition,DWORD,HANDLE){
 int flags=(access&GENERIC_READ)?((access&GENERIC_WRITE)?O_RDWR:O_RDONLY):O_WRONLY;
 if(disposition==OPEN_ALWAYS)flags|=O_CREAT;if(disposition==CREATE_ALWAYS)flags|=O_CREAT|O_TRUNC;
 const int descriptor=::open(mu::native_path(path).c_str(),flags,0600);if(descriptor<0)return INVALID_HANDLE_VALUE;return new NativeFile{descriptor};
}
BOOL CloseHandle(HANDLE handle){if(fd(handle)<0)return FALSE;const int result=::close(fd(handle));delete static_cast<NativeFile*>(handle);return result==0;}
DWORD GetFileSize(HANDLE handle,DWORD* high){struct stat status{};if(fstat(fd(handle),&status)!=0)return 0xffffffffu;const auto size=std::uint64_t(status.st_size);if(high)*high=DWORD(size>>32);return DWORD(size);}
DWORD SetFilePointer(HANDLE handle,LONG offset,LONG* high,DWORD origin){
 const std::int64_t requested=high?(std::int64_t(*high)*4294967296ll+std::uint32_t(offset)):offset;
 const int whence=origin==FILE_END?SEEK_END:origin==FILE_CURRENT?SEEK_CUR:SEEK_SET;const auto value=lseek(fd(handle),requested,whence);if(value<0)return INVALID_SET_FILE_POINTER;if(high)*high=LONG(std::uint64_t(value)>>32);return DWORD(value);
}
BOOL ReadFile(HANDLE handle,void* output,DWORD size,DWORD* count,OVERLAPPED*){
 std::size_t total=0;while(total<size){const auto result=::read(fd(handle),static_cast<char*>(output)+total,size-total);if(result<0){if(errno==EINTR)continue;if(count)*count=DWORD(total);return FALSE;}if(!result)break;total+=std::size_t(result);}if(count)*count=DWORD(total);return TRUE;
}
BOOL WriteFile(HANDLE handle,const void* input,DWORD size,DWORD* count,OVERLAPPED*){
 std::size_t total=0;while(total<size){const auto result=::write(fd(handle),static_cast<const char*>(input)+total,size-total);if(result<0){if(errno==EINTR)continue;if(count)*count=DWORD(total);return FALSE;}if(!result)return FALSE;total+=std::size_t(result);}if(count)*count=DWORD(total);return TRUE;
}
BOOL CreateDirectory(const char* path,void*){std::error_code error;const auto value=mu::native_path(path);std::filesystem::create_directories(value,error);return !error;}
DWORD GetLastError(){return DWORD(errno);}
BOOL SystemTimeToFileTime(const SYSTEMTIME* value,FILETIME* output){
 if(!value||!output||value->wYear<1601)return FALSE;
 std::tm parts{};parts.tm_year=value->wYear-1900;parts.tm_mon=value->wMonth-1;parts.tm_mday=value->wDay;parts.tm_hour=value->wHour;parts.tm_min=value->wMinute;parts.tm_sec=value->wSecond;
 const std::time_t epoch=timegm(&parts);const auto ticks=std::uint64_t(std::int64_t(epoch)+11644473600ll)*10000000ull+std::uint64_t(value->wMilliseconds)*10000;
 output->dwLowDateTime=DWORD(ticks);output->dwHighDateTime=DWORD(ticks>>32);return TRUE;
}
BOOL FileTimeToSystemTime(const FILETIME* value,SYSTEMTIME* output){
 if(!value||!output)return FALSE;const auto ticks=(std::uint64_t(value->dwHighDateTime)<<32)|value->dwLowDateTime;
 const std::time_t epoch=std::int64_t(ticks/10000000)-11644473600ll;std::tm parts{};if(!gmtime_r(&epoch,&parts))return FALSE;
 *output={WORD(parts.tm_year+1900),WORD(parts.tm_mon+1),WORD(parts.tm_wday),WORD(parts.tm_mday),WORD(parts.tm_hour),WORD(parts.tm_min),WORD(parts.tm_sec),WORD((ticks%10000000)/10000)};return TRUE;
}
BOOL WritePrivateProfileString(const char* section,const char* key,const char* value,const char* path){
 if(!section||!key||!value||!path)return FALSE;
 const auto filename=mu::native_path(path);std::ifstream input(filename);std::vector<std::string> lines;std::string line,current;bool inside=false,sectionFound=false,written=false;
 while(std::getline(input,line)){
  const auto text=trim(line);
  if(!text.empty()&&text[0]=='['){
   if(inside&&!written){lines.push_back(std::string(key)+"="+value);written=true;}
   const auto end=text.find(']');current=end==std::string::npos?"":fold(trim(text.substr(1,end-1)));inside=current==fold(section);sectionFound|=inside;
  }
  const auto equals=text.find('=');
  if(inside&&equals!=std::string::npos&&fold(trim(text.substr(0,equals)))==fold(key)){if(!written){lines.push_back(std::string(key)+"="+value);written=true;}continue;}
  lines.push_back(line);
 }
 if(!sectionFound)lines.push_back(std::string("[")+section+"]");if(!written)lines.push_back(std::string(key)+"="+value);
 const auto temporary=filename+".mu-native.tmp";std::ofstream output(temporary,std::ios::trunc);if(!output)return FALSE;for(const auto& entry:lines)output<<entry<<'\n';output.close();if(!output)return FALSE;
 std::error_code error;std::filesystem::rename(temporary,filename,error);return !error;
}
namespace{
 struct NativeFind{std::vector<std::filesystem::directory_entry> entries;std::size_t position=0;};
 BOOL find_entry(NativeFind* state,WIN32_FIND_DATA* output){
  if(!state||!output||state->position>=state->entries.size())return FALSE;
  const auto& entry=state->entries[state->position++];std::error_code error;
  *output=WIN32_FIND_DATA{};output->dwFileAttributes=entry.is_directory(error)?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_NORMAL;
  return strcpy_s(output->cFileName,entry.path().filename().string().c_str())==0;
 }
}
BOOL PathMatchSpec(const char* path,const char* pattern){return path&&pattern&&fnmatch(pattern,path,FNM_CASEFOLD)==0;}
HANDLE FindFirstFile(const char* pattern,WIN32_FIND_DATA* output){
 if(!pattern||!output)return INVALID_HANDLE_VALUE;
 const std::filesystem::path path(mu::native_path(pattern));const auto parent=path.has_parent_path()?path.parent_path():std::filesystem::path(".");
 auto state=std::make_unique<NativeFind>();std::error_code error;
 for(std::filesystem::directory_iterator it(parent,error),end;!error&&it!=end;it.increment(error))
  if(PathMatchSpec(it->path().filename().string().c_str(),path.filename().string().c_str()))state->entries.push_back(*it);
 if(error||state->entries.empty())return INVALID_HANDLE_VALUE;
 std::sort(state->entries.begin(),state->entries.end(),[](const auto& a,const auto& b){return a.path().filename().string()<b.path().filename().string();});
 if(!find_entry(state.get(),output))return INVALID_HANDLE_VALUE;return state.release();
}
BOOL FindNextFile(HANDLE handle,WIN32_FIND_DATA* output){return handle&&handle!=INVALID_HANDLE_VALUE?find_entry(static_cast<NativeFind*>(handle),output):FALSE;}
BOOL FindClose(HANDLE handle){if(!handle||handle==INVALID_HANDLE_VALUE)return FALSE;delete static_cast<NativeFind*>(handle);return TRUE;}
