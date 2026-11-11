#pragma once
#include "mu/platform.hpp"
#include <filesystem>
// dyld.h's legacy TRUE/FALSE enumerators collide with Windows-compatible scalars.
extern "C" int _NSGetExecutablePath(char* buffer,std::uint32_t* size);
namespace mu::launch {
inline std::filesystem::path executable_directory(){
 std::uint32_t size=0;_NSGetExecutablePath(nullptr,&size);
 if(!size)throw std::runtime_error("Could not locate native executable");
 std::vector<char> path(size);
 if(_NSGetExecutablePath(path.data(),&size)!=0)throw std::runtime_error("Could not locate native executable");
 return std::filesystem::weakly_canonical(path.data()).parent_path();
}
inline bool has_role(const std::filesystem::path& dir,const std::string& kind){
 for(const auto& name:{kind+".ini",std::string("AllowableIpList.txt")})
  if(!std::filesystem::is_regular_file(dir/name))return false;
 if(kind=="DataServer"&&!std::filesystem::is_regular_file(dir/"BadSyntax.txt"))return false;
 char port[128]{};
 GetPrivateProfileString((kind+"Info").c_str(),(kind+"Port").c_str(),"",port,sizeof(port),(dir/(kind+".ini")).c_str());
 return port[0]!=0;
}
inline std::filesystem::path service_config(const std::string& kind,const std::string& role,
 const std::filesystem::path& executableDir,const std::filesystem::path& cwd){
 const std::filesystem::path paths[]={cwd,executableDir/"config"/role,executableDir/"config",
  executableDir/"../../config"/role,cwd/"native/config"/role};
 for(const auto& path:paths)if(has_role(path,kind))return std::filesystem::weakly_canonical(path);
 throw std::runtime_error("No default "+kind+" configuration found. Pass --config DIR containing the service INI and runtime files.");
}
inline std::filesystem::path mysql_config(const std::filesystem::path& configDir,
 const std::filesystem::path& executableDir,const std::filesystem::path&){
 // Finder normally starts in HOME: never take an unrelated HOME/mysql.ini.
 const std::filesystem::path paths[]={configDir/"mysql.ini",executableDir/"mysql.ini",
  executableDir/"../../mysql.ini",executableDir/"config/mysql.ini"};
 for(const auto& path:paths)if(std::filesystem::is_regular_file(path))return std::filesystem::weakly_canonical(path);
 return {}; // Caller may use MU_MYSQL_* exclusively. Never select a template.
}
}
