// ScriptLoader.cpp: implementation of the CScriptLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptLoader.h"
#include "CriticalSection.h"
#include "LuaFunction.h"
#include "Log.h"
#include "Path.h"
#include "ServerInfo.h"
#include "Util.h"

CScriptLoader gScriptLoader;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScriptLoader::CScriptLoader() // OK
{
#if defined(MU_NATIVE_MACOS)
	this->m_luaState = nullptr;
#endif
}

CScriptLoader::~CScriptLoader() // OK
{
#if !defined(MU_NATIVE_MACOS)
	this->OnShutScript();
#endif
	// Native main closes Lua explicitly while gameplay globals are still alive.
}

void CScriptLoader::Init() // OK
{
	this->m_critical.lock();

	this->m_ScriptInfo.clear();

	char wildcard_path[MAX_PATH];

	wsprintf(wildcard_path,"%s*",gPath.GetFullPath("Script\\Bridge\\"));

	WIN32_FIND_DATA data;

	HANDLE file = FindFirstFile(wildcard_path,&data);

	if(file == INVALID_HANDLE_VALUE)
	{
		this->m_critical.unlock();
		return;
	}

	do
	{
		if((data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0 && PathMatchSpec(data.cFileName,"*.lua"))
		{
			char path[MAX_PATH];

			wsprintf(path,"Script\\Bridge\\%s",data.cFileName);

			lua_State* lua = luaL_newstate();

			luaL_openlibs(lua);
			lua_pushcclosure(lua,LuaRequire,0);
			lua_setglobal(lua,"require");
			lua_gc(lua,LUA_GCCOLLECT,0);

			InitLuaFunction(lua);

			if(luaL_loadfile(lua,gPath.GetFullPath(path)) != 0)
			{
				LogAdd(LOG_RED,"[ScriptLoader] Could not load '%s'. %s",data.cFileName,lua_tostring(lua,-1));
				gLog.Output(LOG_SCRIPT,"%s",lua_tostring(lua,-1));
				continue;
			}

			if(lua_pcall(lua,0,0,0) != 0)
			{
				LogAdd(LOG_RED,"[ScriptLoader] Error in Lua-file. %s",lua_tostring(lua,-1));
				gLog.Output(LOG_SCRIPT,"%s",lua_tostring(lua,-1));
				continue;
			}

			this->m_ScriptInfo.insert(std::pair<int,lua_State*>(this->m_ScriptInfo.size()+1,lua));
		}
	} while(FindNextFile(file,&data) != 0);
	FindClose(file);

	this->m_critical.unlock();
}

void CScriptLoader::Load(char* path) // OK
{
	this->m_critical.lock();

	this->m_luaState = 0;

	lua_State* lua = luaL_newstate();

	luaL_openlibs(lua);
	lua_pushcclosure(lua,LuaRequire,0);
	lua_setglobal(lua,"require");
	lua_gc(lua,LUA_GCCOLLECT,0);

	InitLuaFunction(lua);

	if(luaL_loadfile(lua,path) != 0)
    {
		LogAdd(LOG_RED,"[ScriptLoader] Could not load '%s'. %s",path,lua_tostring(lua,-1));
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(lua,-1));
		this->m_critical.unlock();
        return;
    }

	if(lua_pcall(lua,0,0,0) != 0)
    {
		LogAdd(LOG_RED,"[ScriptLoader] Error in Lua-file. %s",lua_tostring(lua,-1));
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(lua,-1));
		this->m_critical.unlock();
        return;
    }

	this->m_luaState = lua;

	this->m_critical.unlock();
}

void CScriptLoader::OnReadScript() // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnReadScript");

		if(lua_pcall(it->second,0,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnReadScript");

	if(lua_pcall(this->m_luaState,0,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnShutScript() // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnShutScript");

		if(lua_pcall(it->second,0,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnShutScript");

	if(lua_pcall(this->m_luaState,0,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnTimerThread() // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnTimerThread");

		if(lua_pcall(it->second,0,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnTimerThread");

	if(lua_pcall(this->m_luaState,0,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

int CScriptLoader::OnCommandManager(int aIndex,int code,char* arg) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCommandManager");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,code);

		lua_pushstring(it->second,arg);

		if(lua_pcall(it->second,3,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 0)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 0;
	}

	lua_getglobal(this->m_luaState,"OnCommandManager");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,code);

	lua_pushstring(this->m_luaState,arg);

	if(lua_pcall(this->m_luaState,3,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 0;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

void CScriptLoader::OnCommandDone(int aIndex,int code) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCommandDone");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,code);

		if(lua_pcall(it->second,2,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnCommandDone");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,code);

	if(lua_pcall(this->m_luaState,2,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnCharacterEntry(int aIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCharacterEntry");

		lua_pushinteger(it->second,aIndex);

		if(lua_pcall(it->second,1,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnCharacterEntry");

	lua_pushinteger(this->m_luaState,aIndex);

	if(lua_pcall(this->m_luaState,1,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnCharacterClose(int aIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCharacterClose");

		lua_pushinteger(it->second,aIndex);

		if(lua_pcall(it->second,1,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnCharacterClose");

	lua_pushinteger(this->m_luaState,aIndex);

	if(lua_pcall(this->m_luaState,1,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

int CScriptLoader::OnNpcTalk(int aIndex,int bIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnNpcTalk");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,bIndex);

		if(lua_pcall(it->second,2,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 0)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 0;
	}

	lua_getglobal(this->m_luaState,"OnNpcTalk");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,bIndex);

	if(lua_pcall(this->m_luaState,2,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 0;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

void CScriptLoader::OnMonsterDie(int aIndex,int bIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnMonsterDie");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,bIndex);

		if(lua_pcall(it->second,2,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnMonsterDie");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,bIndex);

	if(lua_pcall(this->m_luaState,2,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnUserDie(int aIndex,int bIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnUserDie");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,bIndex);

		if(lua_pcall(it->second,2,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnUserDie");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,bIndex);

	if(lua_pcall(this->m_luaState,2,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CScriptLoader::OnUserRespawn(int aIndex,int KillerType) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnUserRespawn");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,KillerType);

		if(lua_pcall(it->second,2,0,0) != 0)
		{
			continue;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return;
	}

	lua_getglobal(this->m_luaState,"OnUserRespawn");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,KillerType);

	if(lua_pcall(this->m_luaState,2,0,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

int CScriptLoader::OnCheckUserTarget(int aIndex,int bIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCheckUserTarget");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,bIndex);

		if(lua_pcall(it->second,2,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 1)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 1;
	}

	lua_getglobal(this->m_luaState,"OnCheckUserTarget");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,bIndex);

	if(lua_pcall(this->m_luaState,2,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 1;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

int CScriptLoader::OnCheckUserKiller(int aIndex,int bIndex) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnCheckUserKiller");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,bIndex);

		if(lua_pcall(it->second,2,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 1)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 1;
	}

	lua_getglobal(this->m_luaState,"OnCheckUserKiller");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,bIndex);

	if(lua_pcall(this->m_luaState,2,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 1;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

int CScriptLoader::OnUserItemPick(int aIndex,int slot) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnUserItemPick");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,slot);

		if(lua_pcall(it->second,2,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 1)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 1;
	}

	lua_getglobal(this->m_luaState,"OnUserItemPick");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,slot);

	if(lua_pcall(this->m_luaState,2,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 1;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

int CScriptLoader::OnUserItemDrop(int aIndex,int slot,int x,int y) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnUserItemDrop");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,slot);

		lua_pushinteger(it->second,x);

		lua_pushinteger(it->second,y);

		if(lua_pcall(it->second,4,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 1)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 1;
	}

	lua_getglobal(this->m_luaState,"OnUserItemDrop");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,slot);

	lua_pushinteger(this->m_luaState,x);

	lua_pushinteger(this->m_luaState,y);

	if(lua_pcall(this->m_luaState,4,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 1;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}

int CScriptLoader::OnUserItemMove(int aIndex,int aFlag,int aSlot,int bFlag,int bSlot) // OK
{
	this->m_critical.lock();

	for(std::map<int,lua_State*>::iterator it=this->m_ScriptInfo.begin();it != this->m_ScriptInfo.end();it++)
	{
		if(it->second == 0)
		{
			continue;
		}

		lua_getglobal(it->second,"OnUserItemMove");

		lua_pushinteger(it->second,aIndex);

		lua_pushinteger(it->second,aFlag);

		lua_pushinteger(it->second,aSlot);

		lua_pushinteger(it->second,bFlag);

		lua_pushinteger(it->second,bSlot);

		if(lua_pcall(it->second,5,1,0) != 0)
		{
			continue;
		}

		int value = lua_tointeger(it->second,-1);
	
		lua_pop(it->second,1);

		if(value != 1)
		{
			this->m_critical.unlock();
			return value;
		}
	}

	if(this->m_luaState == 0)
	{
		this->m_critical.unlock();
		return 1;
	}

	lua_getglobal(this->m_luaState,"OnUserItemMove");

	lua_pushinteger(this->m_luaState,aIndex);

	lua_pushinteger(this->m_luaState,aFlag);

	lua_pushinteger(this->m_luaState,aSlot);

	lua_pushinteger(this->m_luaState,bFlag);

	lua_pushinteger(this->m_luaState,bSlot);

	if(lua_pcall(this->m_luaState,5,1,0) != 0)
	{
		gLog.Output(LOG_SCRIPT,"%s",lua_tostring(this->m_luaState,-1));
		this->m_critical.unlock();
		return 1;
	}

	int value = lua_tointeger(this->m_luaState,-1);
	
	lua_pop(this->m_luaState,1);

	this->m_critical.unlock();
	return value;
}
#if defined(MU_NATIVE_MACOS)
void CScriptLoader::NativeShutdown()
{
	this->m_critical.lock();
	this->OnShutScript();
	for(auto& entry:this->m_ScriptInfo)if(entry.second)lua_close(entry.second);
	this->m_ScriptInfo.clear();
	if(this->m_luaState)lua_close(this->m_luaState);
	this->m_luaState = nullptr;
	this->m_critical.unlock();
}
#endif
