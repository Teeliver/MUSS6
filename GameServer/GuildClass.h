#pragma once

#include "User.h"

class CGuildClass
{
public:
	CGuildClass();
	virtual ~CGuildClass();
	void Init();
	GUILD_INFO_STRUCT* AddGuild(int GuildNumber,char* GuildName,BYTE* Mark,char* GuildMaster,int score);
	GUILD_INFO_STRUCT* AddMember(GUILD_INFO_STRUCT* lpGuild,char* Name,int aIndex,int score,int server);
	GUILD_INFO_STRUCT* AddMember(char* GuildName,char* Name,int aIndex,int count,int status,int server);
	void AddTail(GUILD_INFO_STRUCT* lpGuild);
	void Print();
	void PrintGuild(char* GuildName);
	void AllDelete();
	int DeleteGuild(char* GuildName);
	GUILD_INFO_STRUCT* SearchGuild(char* GuildName);
	GUILD_INFO_STRUCT* SearchGuild_Number(int GuildNumber);
	GUILD_INFO_STRUCT* SearchGuild_NumberAndId(int GuildNumber,char* Name);
	int ConnectUser(GUILD_INFO_STRUCT* lpGuild,char* GuildName,char* Name,int aIndex,int server);
	int GetGuildMemberStatus(char* GuildName,char* Name);
	int SetGuildMemberStatus(char* GuildName,char* Name,int status);
	int GetGuildType(char* GuildName);
	int SetGuildType(char* GuildName,int type);
	int BuildMemberTotal(GUILD_INFO_STRUCT* lpGuild);
	int DelMember(char* GuildName,char* Name);
	int CloseMember(char* GuildName,char* Name);
	int SetServer(char* GuildName,char* Name,int server);
private:
	GUILD_INFO_STRUCT* head;
	GUILD_INFO_STRUCT* tail;
	std::map<std::string,GUILD_INFO_STRUCT*> m_GuildMap;
	std::map<int,GUILD_INFO_STRUCT*> m_GuildNumberMap;
};

extern CGuildClass gGuildClass;
