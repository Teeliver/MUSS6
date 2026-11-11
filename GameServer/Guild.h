// Guild.h: interface for the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

enum eGuildStatus
{
	GUILD_STATUS_NONE = -1,
	GUILD_STATUS_MEMBER = 0,
	GUILD_STATUS_BATTLE_MASTER = 32,
	GUILD_STATUS_OFFICE_MASTER = 64,
	GUILD_STATUS_MASTER = 128,
};

enum eGuildType
{
	GUILD_TYPE_JOIN = 1,
	GUILD_TYPE_BREAK = 2,
};

enum eGuildRelation
{
	GUILD_RELATION_NONE = 0,
	GUILD_RELATION_UNION = 1,
	GUILD_RELATION_RIVAL = 2,
};

enum eGuildWarType
{
	GUILD_WAR_TYPE_NORMAL = 0,
	GUILD_WAR_TYPE_SOCCER = 1,
};

enum eGuildWarState
{
	GUILD_WAR_STATE_NONE = 0,
	GUILD_WAR_STATE_DECLARE = 1,
	GUILD_WAR_STATE_ACTIVE = 2,
};

enum eGuildWarTeam
{
	GUILD_WAR_TEAM_BLUE = 0,
	GUILD_WAR_TEAM_RED = 1,
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GUILD_CREATE_RECV
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

struct SDHP_GUILD_DELETE_RECV
{
	PSBMSG_HEAD header; // C1:A0:01
	BYTE result;
	BYTE flag;
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
	BYTE result;
	BYTE flag;
	WORD index;
	char GuildName[9];
	char Name[11];
	WORD server;
};

struct SDHP_GUILD_MEMBER_BREAK_RECV
{
	PSBMSG_HEAD header; // C1:A0:04
	BYTE result;
	BYTE flag;
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_STATUS_RECV
{
	PSBMSG_HEAD header; // C1:A0:05
	char GuildName[9];
	char Name[11];
	BYTE status;
	BYTE type;
	WORD server;
};

struct SDHP_GUILD_ASSIGN_STATUS_RECV
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

struct SDHP_GUILD_ASSIGN_TYPE_RECV
{
	PSBMSG_HEAD header; // C1:A0:07
	BYTE result;
	BYTE flag;
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

struct SDHP_GUILD_INFO_RECV
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

struct SDHP_GUILD_UNION_INFO_RECV
{
	PSWMSG_HEAD header; // C2:A0:0C
	BYTE relation;
	BYTE count;
	char GuildName[9];
	DWORD GuildNumber;
	int GuildMembers[100];
};

struct SDHP_GUILD_UNION_UPDATE_RECV
{
	PSWMSG_HEAD header; // C2:A0:0D
	BYTE update;
	DWORD count;
	int GuildMembers[100];
};

struct SDHP_GUILD_UNION_JOIN_RECV
{
	PSBMSG_HEAD header; // C1:A0:10
	BYTE result;
	BYTE flag;
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_BREAK_RECV
{
	PSBMSG_HEAD header; // C1:A0:11
	BYTE result;
	BYTE flag;
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_LIST_RECV
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

struct SDHP_GUILD_ALLIANCE_KICK_RECV
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
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GUILD_CREATE_SEND
{
	PSBMSG_HEAD header; // C1:A0:00
	WORD index;
	char GuildName[9];
	char Name[11];
	BYTE Mark[32];
	BYTE type;
};

struct SDHP_GUILD_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:A0:01
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
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_BREAK_SEND
{
	PSBMSG_HEAD header; // C1:A0:04
	WORD index;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_MEMBER_GET_INFO_SEND
{
	PSBMSG_HEAD header; // C1:A0:05
	char Name[11];
};

struct SDHP_GUILD_ASSIGN_STATUS_SEND
{
	PSBMSG_HEAD header; // C1:A0:06
	WORD index;
	BYTE type;
	BYTE status;
	char GuildName[9];
	char Name[11];
};

struct SDHP_GUILD_ASSIGN_TYPE_SEND
{
	PSBMSG_HEAD header; // C1:A0:07
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

struct SDHP_GUILD_UNION_JOIN_SEND
{
	PSBMSG_HEAD header; // C1:A0:10
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_BREAK_SEND
{
	PSBMSG_HEAD header; // C1:A0:11
	WORD index[2];
	BYTE relation;
	DWORD GuildNumber[2];
};

struct SDHP_GUILD_UNION_LIST_SEND
{
	PSBMSG_HEAD header; // C1:A0:12
	WORD index;
	DWORD GuildNumber;
};

struct SDHP_GUILD_ALLIANCE_KICK_SEND
{
	PSBMSG_HEAD header; // C1:A0:13
	WORD index;
	BYTE relation;
	char GuildNameA[9];
	char GuildNameB[9];
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_GUILD_REQUEST_RECV
{
	PBMSG_HEAD header; // C1:50
	BYTE index[2];
};

struct PMSG_GUILD_RESULT_RECV
{
	PBMSG_HEAD header; // C1:51
	BYTE result;
	BYTE index[2];
};

struct PMSG_GUILD_DELETE_RECV
{
	PBMSG_HEAD header; // C1:53
	char name[10];
	char PersonalCode[10];
};

struct PMSG_GUILD_MASTER_OPEN_RECV
{
	PBMSG_HEAD header; // C1:54
	BYTE result;
};

struct PMSG_GUILD_CREATE_RECV
{
	PBMSG_HEAD header; // C1:55
	BYTE type;
	char GuildName[8];
	BYTE Mark[32];
};

struct PMSG_GUILD_WAR_REQUEST_RESULT_RECV
{
	PBMSG_HEAD header; // C1:61
	BYTE result;
};

struct PMSG_GUILD_VIEWPORT_REQUEST_RECV
{
	PBMSG_HEAD header; // C1:66
	DWORD GuildNumber;
};

struct PMSG_GUILD_ASSIGN_STATUS_RECV
{
	PBMSG_HEAD header; // C1:E1
	BYTE type;
	BYTE status;
	char name[10];
};

struct PMSG_GUILD_ASSIGN_TYPE_RECV
{
	PBMSG_HEAD header; // C1:E2
	BYTE type;
};

struct PMSG_GUILD_UNION_REQUEST_RECV
{
	PBMSG_HEAD header; // C1:E5
	BYTE relation;
	BYTE type;
	BYTE index[2];
};

struct PMSG_GUILD_UNION_RESULT_RECV
{
	PBMSG_HEAD header; // C1:E6
	BYTE relation;
	BYTE type;
	BYTE result;
	BYTE index[2];
};

struct PMSG_GUILD_ALLIANCE_KICK_RECV
{
	PSBMSG_HEAD header; // C1:EB:01
	char GuildName[8];
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_GUILD_REQUEST_SEND
{
	PBMSG_HEAD header; // C1:50
	BYTE index[2];
};

struct PMSG_GUILD_RESULT_SEND
{
	PBMSG_HEAD header; // C1:51
	BYTE result;
};

struct PMSG_GUILD_LIST_SEND
{
	PWMSG_HEAD header; // C2:52
	BYTE result;
	BYTE count;
	DWORD TotalScore;
	BYTE score;
	char RivalGuild[9];
	#if(GAMESERVER_UPDATE>=701)
	BYTE RivalCount;
	BYTE RivalMark[32];
	#endif
};

struct PMSG_GUILD_LIST
{
	char name[10];
	BYTE number;
	BYTE connected;
	BYTE status;
};

struct PMSG_GUILD_DELETE_SEND
{
	PBMSG_HEAD header; // C1:53
	BYTE result;
};

struct PMSG_GUILD_CREATE_RESULT_SEND
{
	PBMSG_HEAD header; // C1:56
	BYTE result;
	BYTE type;
};

struct PMSG_GUILD_WAR_REQUEST_RESULT_SEND
{
	PBMSG_HEAD header; // C1:60
	BYTE result;
};

struct PMSG_GUILD_WAR_REQUEST_SEND
{
	PBMSG_HEAD header; // C1:61
	char GuildName[8];
	BYTE type;
};

struct PMSG_GUILD_WAR_START_SEND
{
	PBMSG_HEAD header; // C1:62
	char GuildName[8];
	BYTE type;
	BYTE team;
};

struct PMSG_GUILD_WAR_END_SEND
{
	PBMSG_HEAD header; // C1:63
	BYTE result;
	char GuildName[8];
};

struct PMSG_GUILD_WAR_SCORE_SEND
{
	PBMSG_HEAD header; // C1:64
	BYTE score[2];
	BYTE type;
};

struct PMSG_GUILD_VIEWPORT_SEND
{
	PBMSG_HEAD header; // C1:66
	DWORD GuildNumber;
	BYTE type;
	char UnionName[8];
	char GuildName[8];
	BYTE Mark[32];
};

struct PMSG_GUILD_ASSIGN_STATUS_SEND
{
	PBMSG_HEAD header; // C1:E1
	BYTE type;
	BYTE result;
	char name[10];
};

struct PMSG_GUILD_ASSIGN_TYPE_SEND
{
	PBMSG_HEAD header; // C1:E2
	BYTE type;
	BYTE result;
};

struct PMSG_GUILD_UNION_REQUEST_SEND
{
	PBMSG_HEAD header; // C1:E5
	BYTE relation;
	BYTE type;
	BYTE index[2];
};

struct PMSG_GUILD_UNION_RESULT_SEND
{
	PBMSG_HEAD header; // C1:E6
	BYTE relation;
	BYTE type;
	BYTE result;
	BYTE index[2];
};

struct PMSG_GUILD_UNION_LIST_SEND
{
	PWMSG_HEAD header; // C2:E9
	BYTE count;
	BYTE result;
	BYTE UnionNumber;
	BYTE RivalNumber;
};

struct PMSG_GUILD_UNION_LIST
{
	BYTE count;
	BYTE Mark[32];
	char GuildName[8];
};

struct PMSG_GUILD_ALLIANCE_KICK_RESULT_SEND
{
	PSBMSG_HEAD header; // C1:EB:01
	BYTE result;
	BYTE type;
	BYTE relation;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct GUILD_INFO_STRUCT
{
	int GetTimeStamp() // OK
	{
		return this->TimeStamp;
	}

	BOOL CheckTimeStamp(int time) // OK
	{
		return ((time==this->TimeStamp)?1:0);
	}

	void SetTimeStamp() // OK
	{
		this->TimeStamp++;
	}

	void SetGuildUnion(int GuildNumber) // OK
	{
		this->GuildUnion = GuildNumber;
		this->SetTimeStamp();
	}

	void SetGuildRival(int GuildNumber) // OK
	{
		this->GuildRival = GuildNumber;
		this->SetTimeStamp();
	}

	int Number;
	char Name[9];
	BYTE Mark[32];
	BYTE Count;
	BYTE TotalCount;
	char Names[MAX_GUILD_USER][11];
	short Index[MAX_GUILD_USER];
	BYTE Use[MAX_GUILD_USER];
	char Server[MAX_GUILD_USER];
	char TargetGuildName[9];
	short TargetIndex[MAX_GUILD_USER];
	GUILD_INFO_STRUCT* TargetGuildNode;
	BYTE WarDeclareState;
	BYTE WarState;
	BYTE WarType;
	BYTE BattleGroundIndex;
	BYTE BattleTeamCode;
	BYTE PlayScore;
	int TotalScore;
	char Notice[60];
	int GuildStatus[MAX_GUILD_USER];
	BYTE GuildType;
	int GuildUnion;
	int GuildRival;
	int TimeStamp;
	char GuildRivalName[9];
	OBJECTSTRUCT* LifeStone;
	int LifeStoneCount;
	GUILD_INFO_STRUCT* back;
	GUILD_INFO_STRUCT* next;
};

class CGuild
{
public:
	CGuild();
	virtual ~CGuild();
	void DGGuildCreateRecv(SDHP_GUILD_CREATE_RECV* lpMsg);
	void DGGuildDeleteRecv(SDHP_GUILD_DELETE_RECV* lpMsg);
	void DGGuildMemberDisconnectRecv(SDHP_GUILD_MEMBER_DISCONNECT_RECV* lpMsg);
	void DGGuildMemberJoinRecv(SDHP_GUILD_MEMBER_JOIN_RECV* lpMsg);
	void DGGuildMemberBreakRecv(SDHP_GUILD_MEMBER_BREAK_RECV* lpMsg);
	void DGGuildMemberStatusRecv(SDHP_GUILD_MEMBER_STATUS_RECV* lpMsg);
	void DGGuildAssignStatusRecv(SDHP_GUILD_ASSIGN_STATUS_RECV* lpMsg);
	void DGGuildAssignTypeRecv(SDHP_GUILD_ASSIGN_TYPE_RECV* lpMsg);
	void DGGuildScoreUpdateRecv(SDHP_GUILD_SCORE_UPDATE_RECV* lpMsg);
	void DGGuildGlobalChatRecv(SDHP_GUILD_GLOBAL_CHAT_RECV* lpMsg);
	void DGGuildGlobalNoticeRecv(SDHP_GUILD_GLOBAL_NOTICE_RECV* lpMsg);
	void DGGuildInfoRecv(SDHP_GUILD_INFO_RECV* lpMsg);
	void DGGuildUnionInfoRecv(SDHP_GUILD_UNION_INFO_RECV* lpMsg);
	void DGGuildUnionUpdateRecv(SDHP_GUILD_UNION_UPDATE_RECV* lpMsg);
	void DGGuildUnionJoinRecv(SDHP_GUILD_UNION_JOIN_RECV* lpMsg);
	void DGGuildUnionBreakRecv(SDHP_GUILD_UNION_BREAK_RECV* lpMsg);
	void DGGuildUnionListRecv(SDHP_GUILD_UNION_LIST_RECV* lpMsg);
	void DGGuildAllianceKickRecv(SDHP_GUILD_ALLIANCE_KICK_RECV* lpMsg);
	void GDGuildMemberDisconnectSend(int aIndex);
	void GDGuildMemberJoinSend(int aIndex,char* GuildName,char* Name);
	void GDGuildMemberBreakSend(int aIndex,char* GuildName,char* Name);
	void GDGuildMemberGetInfoSend(int aIndex,char* Name);
	void GDGuildScoreUpdateSend(char* GuildName,int score);
	void GDGuildGlobalChatSend(int type,char* GuildName,char* Name,char* message);
	void GDGuildGlobalNoticeSend(char* GuildName,char* message);
	void CGGuildRequestRecv(PMSG_GUILD_REQUEST_RECV* lpMsg,int aIndex);
	void CGGuildResultRecv(PMSG_GUILD_RESULT_RECV* lpMsg,int aIndex);
	void CGGuildListRecv(int aIndex);
	void CGGuildDeleteRecv(PMSG_GUILD_DELETE_RECV* lpMsg,int aIndex);
	void CGGuildMasterOpenRecv(PMSG_GUILD_MASTER_OPEN_RECV* lpMsg, int aIndex);
	void CGGuildCreateRecv(PMSG_GUILD_CREATE_RECV* lpMsg,int aIndex);
	void CGGuildMasterCancelRecv(int aIndex);
	void CGGuildWarRequestResultRecv(PMSG_GUILD_WAR_REQUEST_RESULT_RECV* lpMsg,int aIndex);
	void CGGuildViewportRequestRecv(PMSG_GUILD_VIEWPORT_REQUEST_RECV* lpMsg,int aIndex);
	void CGGuildAssignStatusRecv(PMSG_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex);
	void CGGuildAssignTypeRecv(PMSG_GUILD_ASSIGN_TYPE_RECV* lpMsg,int aIndex);
	void CGGuildUnionRequestRecv(PMSG_GUILD_UNION_REQUEST_RECV* lpMsg,int aIndex);
	void CGGuildUnionResultRecv(PMSG_GUILD_UNION_RESULT_RECV* lpMsg,int aIndex);
	void CGGuildUnionListRecv(int aIndex);
	void CGGuildAllianceKickRecv(PMSG_GUILD_ALLIANCE_KICK_RECV* lpMsg,int aIndex);
	void GCGuildResultSend(int aIndex,int result);
	void GCGuildDeleteSend(int aIndex,int result);
	void GCGuildCreateResultSend(int aIndex,int result,int type);
	void GCGuildWarResultSend(int aIndex,int result);
	void GCGuildWarRequestSend(int aIndex,char* GuildName,int type);
	void GCGuildWarDeclareSend(int aIndex,char* GuildName,int type,int team);
	void GCGuildWarEndSend(int aIndex,int result,char* GuildName);
	void GCGuildWarScoreSend(int aIndex);
	void GCGuildAssignStatusSend(int aIndex,int type,int result,char* name);
	void GCGuildAssignTypeSend(int aIndex,int type,int result);
	void GCGuildUnionRequestSend(int aIndex,int relation,int type,int bIndex);
	void GCGuildUnionResultSend(int aIndex,int relation,int type,int result,int bIndex);
	void GuildWarDeclareSet(int aIndex,char* GuildName,int type,int team);
	void GuildWarRequestSend(int aIndex,char* GuildName,int type);
	int GetMaxGuildUser(LPOBJ lpObj);
	int GetGuildNameCount(LPOBJ lpObj,char* name);
	int GetGuildBattleMasterCount(LPOBJ lpObj);
	int GetGuildOfficeMasterCount(LPOBJ lpObj);
	int gObjTargetGuildWarCheck(LPOBJ lpObj,LPOBJ lpTarget);
	void gObjGuildWarEndSend(LPOBJ lpObj,int result1,int result2);
	void gObjGuildWarEndSend(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2,int result1,int result2);
	void gObjGuildWarEnd(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2);
	int gObjGuildWarProc(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2,int score);
	int gObjGuildWarCheck(LPOBJ lpObj,LPOBJ lpTargetObj);
	int gObjGuildWarMasterClose(LPOBJ lpObj);
	int gObjGetGuildUnionNumber(LPOBJ lpObj);
	void gObjGetGuildUnionName(LPOBJ lpObj,char* UnionName,int size);
	bool gObjCheckRival(LPOBJ lpObj,LPOBJ lpTarget);
	int gObjGetRelationShip(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(GUILD_INFO_STRUCT* lpGuild,int type,char* message,...);
	void DataSendToAll(GUILD_INFO_STRUCT* lpGuild,BYTE* lpMsg,int size);
};

extern CGuild gGuild;