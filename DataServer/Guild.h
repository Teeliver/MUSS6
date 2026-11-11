// Guild.h: interface for the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GUILD_CREATE_RECV
{
	PSBMSG_HEAD header; // C1:A0:00
	WORD index;
	char GuildName[9];
	char Name[11];
	BYTE Mark[32];
	BYTE type;
};

struct SDHP_GUILD_DELETE_RECV
{
	PSBMSG_HEAD header; // C1:A0:01
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_DISCONNECT_RECV
{
	PSBMSG_HEAD header; // C1:A0:02
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_JOIN_RECV
{
	PSBMSG_HEAD header; // C1:A0:03
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_BREAK_RECV
{
	PSBMSG_HEAD header; // C1:A0:04
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_GET_INFO_RECV
{
	PSBMSG_HEAD header; // C1:A0:05
	char Name[11];
};

struct SDHP_GUILD_ASSIGN_STATUS_RECV
{
	PSBMSG_HEAD header; // C1:A0:06
	WORD index;
	BYTE type;
	BYTE status;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_ASSIGN_TYPE_RECV
{
	PSBMSG_HEAD header; // C1:A0:07
	WORD index;
	BYTE type;
	char GuildName[9];
};

struct SDHP_GUILD_SCORE_UPDATE_RECV
{
	PSBMSG_HEAD header; // C1:A0:08
	char GuildName[9];
	DWORD score;
};

struct SDHP_GUILD_GLOBAL_CHAT_RECV
{
	PSBMSG_HEAD header; // C1:A0:09
	BYTE type;
	char GuildName[9];
	char Name[11];
	char message[60];
};

struct SDHP_GUILD_GLOBAL_NOTICE_RECV
{
	PSBMSG_HEAD header; // C1:A0:0A
	char GuildName[9];
	char message[60];
};

struct SDHP_GUILD_UNION_JOIN_RECV
{
	PSBMSG_HEAD header; // C1:A0:10
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_BREAK_RECV
{
	PSBMSG_HEAD header; // C1:A0:11
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_LIST_RECV
{
	PSBMSG_HEAD header; // C1:A0:12
	WORD index;
	DWORD GuildNumber;
};

struct SDHP_GUILD_ALLIANCE_KICK_RECV
{
	PSBMSG_HEAD header; // C1:A0:13
	WORD index;
	BYTE relation;
	char GuildNameA[9];
	char GuildNameB[9];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GUILD_CREATE_SEND
{
	PSBMSG_HEAD header; // C1:A0:00
	BYTE result;
	BYTE flag;
	WORD index;
	DWORD GuildNumber;
	char GuildName[9];
	char Name[11];
	BYTE Mark[32];
	BYTE type;
};

struct SDHP_GUILD_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:A0:01
	BYTE result;
	BYTE flag;
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_DISCONNECT_SEND
{
	PSBMSG_HEAD header; // C1:A0:02
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_JOIN_SEND
{
	PSBMSG_HEAD header; // C1:A0:03
	BYTE result;
	BYTE flag;
	WORD index;
	char GuildName[9];
	char Name[11];
	WORD server;
};

struct SDHP_GUILD_MEMBER_BREAK_SEND
{
	PSBMSG_HEAD header; // C1:A0:04
	BYTE result;
	BYTE flag;
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_STATUS_SEND
{
	PSBMSG_HEAD header; // C1:A0:05
	char GuildName[9];
	char Name[11];
	BYTE status;
	BYTE type;
	WORD server;
};

struct SDHP_GUILD_ASSIGN_STATUS_SEND
{
	PSBMSG_HEAD header; // C1:A0:06
	BYTE result;
	BYTE flag;
	WORD index;
	BYTE type;
	BYTE status;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_ASSIGN_TYPE_SEND
{
	PSBMSG_HEAD header; // C1:A0:07
	BYTE result;
	BYTE flag;
	WORD index;
	BYTE type;
	char GuildName[9];
};

struct SDHP_GUILD_SCORE_UPDATE_SEND
{
	PSBMSG_HEAD header; // C1:A0:08
	char GuildName[9];
	DWORD score;
};

struct SDHP_GUILD_GLOBAL_CHAT_SEND
{
	PSBMSG_HEAD header; // C1:A0:09
	BYTE type;
	char GuildName[9];
	char Name[11];
	char message[60];
};

struct SDHP_GUILD_GLOBAL_NOTICE_SEND
{
	PSBMSG_HEAD header; // C1:A0:0A
	char GuildName[9];
	char message[60];
};

struct SDHP_GUILD_INFO_SEND
{
	PSWMSG_HEAD header; // C2:A0:0B
	DWORD GuildNumber;
	char GuildName[9];
	char GuildMaster[11];
	BYTE Mark[32];
	DWORD score;
	BYTE type;
	DWORD GuildUnionNumber;
	DWORD GuildRivalNumber;
	char GuildRivalName[9];
	BYTE count;
};

struct SDHP_GUILD_MEMBER_INFO
{
	char Name[11];
	BYTE status;
	WORD server;
};

struct SDHP_GUILD_UNION_INFO_SEND
{
	PSWMSG_HEAD header; // C2:A0:0C
	BYTE relation;
	BYTE count;
	char GuildName[9];
	DWORD GuildNumber;
	int GuildMembers[100];
};

struct SDHP_GUILD_UNION_UPDATE_SEND
{
	PSWMSG_HEAD header; // C2:A0:0D
	BYTE update;
	DWORD count;
	int GuildMembers[100];
};

struct SDHP_GUILD_UNION_JOIN_SEND
{
	PSBMSG_HEAD header; // C1:A0:10
	BYTE result;
	BYTE flag;
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_BREAK_SEND
{
	PSBMSG_HEAD header; // C1:A0:11
	BYTE result;
	BYTE flag;
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_LIST_SEND
{
	PSWMSG_HEAD header; // C2:A0:12
	BYTE count;
	BYTE result;
	WORD index;
	DWORD time;
	BYTE UnionNumber;
	BYTE RivalNumber;
};

struct SDHP_GUILD_UNION_INFO
{
	BYTE count;
	BYTE Mark[32];
	char GuildName[9];
};

struct SDHP_GUILD_ALLIANCE_KICK_SEND
{
	PSBMSG_HEAD header; // C1:EB:13
	BYTE flag;
	WORD index;
	BYTE result;
	BYTE relation;
	char GuildNameA[9];
	char GuildNameB[9];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CGuild
{
public:
	CGuild();
	virtual ~CGuild();
	void GDGuildCreateRecv(SDHP_GUILD_CREATE_RECV* lpMsg,int aIndex);
	void GDGuildDeleteRecv(SDHP_GUILD_DELETE_RECV* lpMsg,int aIndex);
	void GDGuildMemberDisconnectRecv(SDHP_GUILD_MEMBER_DISCONNECT_RECV* lpMsg);
	void GDGuildMemberJoinRecv(SDHP_GUILD_MEMBER_JOIN_RECV* lpMsg,int aIndex);
	void GDGuildMemberBreakRecv(SDHP_GUILD_MEMBER_BREAK_RECV* lpMsg,int aIndex);
	void GDGuildMemberGetInfoRecv(SDHP_GUILD_MEMBER_GET_INFO_RECV* lpMsg,int aIndex);
	void GDGuildAssignStatusRecv(SDHP_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex);
	void GDGuildAssignTypeRecv(SDHP_GUILD_ASSIGN_TYPE_RECV* lpMsg,int aIndex);
	void GDGuildScoreUpdateRecv(SDHP_GUILD_SCORE_UPDATE_RECV* lpMsg);
	void GDGuildGlobalChatRecv(SDHP_GUILD_GLOBAL_CHAT_RECV* lpMsg);
	void GDGuildGlobalNoticeRecv(SDHP_GUILD_GLOBAL_NOTICE_RECV* lpMsg);
	void GDGuildUnionJoinRecv(SDHP_GUILD_UNION_JOIN_RECV* lpMsg,int aIndex);
	void GDGuildUnionBreakRecv(SDHP_GUILD_UNION_BREAK_RECV* lpMsg,int aIndex);
	void GDGuildUnionListRecv(SDHP_GUILD_UNION_LIST_RECV* lpMsg,int aIndex);
	void GDGuildAllianceKickRecv(SDHP_GUILD_ALLIANCE_KICK_RECV* lpMsg,int aIndex);
	void DGGuildMemberStatusSend(char* GuildName,char* Name,BYTE status,BYTE type,WORD server);
	void DGGuildInfoSend(DWORD GuildNumber);
	void DGGuildUnionInfoSend(DWORD GuildNumber,BYTE relation);
	void DGGuildUnionUpdateSend(BYTE update,DWORD count,int* GuildMembers);
};

extern CGuild gGuild;