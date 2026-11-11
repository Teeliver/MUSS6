// Guild.cpp: implementation of the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Guild.h"
#include "CharacterManager.h"
#include "GuildManager.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "Util.h"

CGuild gGuild;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuild::CGuild() // OK
{

}

CGuild::~CGuild() // OK
{

}

void CGuild::GDGuildCreateRecv(SDHP_GUILD_CREATE_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_CREATE_SEND pMsg;

	pMsg.header.set(0xA0,0x00,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	pMsg.index = lpMsg->index;

	pMsg.GuildNumber = 0;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	memcpy(pMsg.Mark,lpMsg->Mark,sizeof(pMsg.Mark));

	pMsg.type = lpMsg->type;

	if((pMsg.result=gGuildManager.AddGuild(aIndex,lpMsg->GuildName,lpMsg->Name,lpMsg->Mark,lpMsg->type)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->GuildName);

	GUILD_MEMBER_INFO* lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->Name);

	if(lpGuildInfo != 0 && lpGuildMemberInfo != 0)
	{
		pMsg.GuildNumber = lpGuildInfo->dwNumber;

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				pMsg.flag = ((n==aIndex) ? 1 : 0);
				gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}

		gGuildManager.ConnectMember(lpMsg->Name,GetServerCodeByName(lpMsg->Name));

		DGGuildInfoSend(lpGuildInfo->dwNumber);

		DGGuildUnionInfoSend(lpGuildInfo->dwUnionNumber,1);

		DGGuildUnionInfoSend(lpGuildInfo->dwRivalNumber,2);

		DGGuildMemberStatusSend(lpMsg->GuildName,lpMsg->Name,lpGuildMemberInfo->btStatus,lpMsg->type,lpGuildMemberInfo->btServer);
	}
}

void CGuild::GDGuildDeleteRecv(SDHP_GUILD_DELETE_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_DELETE_SEND pMsg;

	pMsg.header.set(0xA0,0x01,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	pMsg.index = lpMsg->index;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	if((pMsg.result=gGuildManager.DelGuild(aIndex,lpMsg->GuildName)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex) ? 1 : 0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildMemberDisconnectRecv(SDHP_GUILD_MEMBER_DISCONNECT_RECV* lpMsg) // OK
{
	SDHP_GUILD_MEMBER_DISCONNECT_SEND pMsg;

	pMsg.header.set(0xA0,0x02,sizeof(pMsg));

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildMemberJoinRecv(SDHP_GUILD_MEMBER_JOIN_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_MEMBER_JOIN_SEND pMsg;

	pMsg.header.set(0xA0,0x03,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	pMsg.index = lpMsg->index;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	pMsg.server = GetServerCodeByName(lpMsg->Name);

	if((pMsg.result=gGuildManager.AddGuildMember(aIndex,lpMsg->GuildName,lpMsg->Name,0,pMsg.server)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex) ? 1 : 0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildMemberBreakRecv(SDHP_GUILD_MEMBER_BREAK_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_MEMBER_BREAK_SEND pMsg;

	pMsg.header.set(0xA0,0x04,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	pMsg.index = lpMsg->index;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	if((pMsg.result=gGuildManager.DelGuildMember(aIndex,lpMsg->Name)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex) ? 1 : 0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildMemberGetInfoRecv(SDHP_GUILD_MEMBER_GET_INFO_RECV* lpMsg,int aIndex) // OK
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetMemberGuildInfo(lpMsg->Name);

	GUILD_MEMBER_INFO* lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->Name);

	if(lpGuildInfo == 0 || lpGuildMemberInfo == 0)
	{
		return;
	}

	gGuildManager.ConnectMember(lpMsg->Name,GetServerCodeByName(lpMsg->Name));

	DGGuildInfoSend(lpGuildInfo->dwNumber);

	DGGuildUnionInfoSend(lpGuildInfo->dwUnionNumber,1);

	DGGuildUnionInfoSend(lpGuildInfo->dwRivalNumber,2);

	DGGuildMemberStatusSend(lpGuildInfo->szName,lpMsg->Name,lpGuildMemberInfo->btStatus,lpGuildInfo->btType,lpGuildMemberInfo->btServer);
}

void CGuild::GDGuildAssignStatusRecv(SDHP_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_ASSIGN_STATUS_SEND pMsg;

	pMsg.header.set(0xA0,0x06,sizeof(pMsg));

	pMsg.result = gGuildManager.SetGuildMemberStatus(lpMsg->Name,lpMsg->status);

	pMsg.index = lpMsg->index;

	pMsg.type = lpMsg->type;

	pMsg.status = lpMsg->status;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex)?1:0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildAssignTypeRecv(SDHP_GUILD_ASSIGN_TYPE_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_ASSIGN_TYPE_SEND pMsg;

	pMsg.header.set(0xA0,0x07,sizeof(pMsg));

	pMsg.result = gGuildManager.SetGuildType(lpMsg->GuildName,lpMsg->type);

	pMsg.index = lpMsg->index;

	pMsg.type = lpMsg->type;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex)?1:0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildScoreUpdateRecv(SDHP_GUILD_SCORE_UPDATE_RECV* lpMsg) // OK
{
	if(gGuildManager.SetGuildScore(lpMsg->GuildName,lpMsg->score) != 0)
	{
		SDHP_GUILD_SCORE_UPDATE_SEND pMsg;

		pMsg.header.set(0xA0,0x08,sizeof(pMsg));

		memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

		pMsg.score = lpMsg->score;

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}
	}
}

void CGuild::GDGuildGlobalChatRecv(SDHP_GUILD_GLOBAL_CHAT_RECV* lpMsg) // OK
{
	SDHP_GUILD_GLOBAL_CHAT_SEND pMsg;

	pMsg.header.set(0xA0,0x09,sizeof(pMsg));

	pMsg.type = lpMsg->type;

	memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpMsg->Name,sizeof(pMsg.Name));

	memcpy(pMsg.message,lpMsg->message,sizeof(pMsg.message));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildGlobalNoticeRecv(SDHP_GUILD_GLOBAL_NOTICE_RECV* lpMsg) // OK
{
	if(gGuildManager.SetGuildNotice(lpMsg->GuildName,lpMsg->message) != 0)
	{
		SDHP_GUILD_GLOBAL_NOTICE_SEND pMsg;

		pMsg.header.set(0xA0,0x0A,sizeof(pMsg));

		memcpy(pMsg.GuildName,lpMsg->GuildName,sizeof(pMsg.GuildName));

		memcpy(pMsg.message,lpMsg->message,sizeof(pMsg.message));

		for(int n=0;n < MAX_SERVER;n++)
		{
			if(gServerManager[n].CheckState() != 0)
			{
				gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
			}
		}
	}
}

void CGuild::GDGuildUnionJoinRecv(SDHP_GUILD_UNION_JOIN_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_UNION_JOIN_SEND pMsg;

	pMsg.header.set(0xA0,0x10,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	memcpy(pMsg.index,lpMsg->index,sizeof(pMsg.index));

	pMsg.relation = lpMsg->relation;

	memcpy(pMsg.GuildNumber,lpMsg->GuildNumber,sizeof(pMsg.GuildNumber));

	GUILD_INFO* lpSourceGuildInfo;

	GUILD_INFO* lpTargetGuildInfo;

	if((lpSourceGuildInfo=gGuildManager.GetGuildInfo(lpMsg->GuildNumber[0])) == 0 || (lpTargetGuildInfo=gGuildManager.GetGuildInfo(lpMsg->GuildNumber[1])) == 0)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	if((pMsg.result=gGuildManager.AddGuildRelationship(aIndex,lpMsg->GuildNumber[0],lpMsg->GuildNumber[1],lpMsg->relation)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n== aIndex)?1:0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildUnionBreakRecv(SDHP_GUILD_UNION_BREAK_RECV* lpMsg,int aIndex) // OK
{
	SDHP_GUILD_UNION_BREAK_SEND pMsg;

	pMsg.header.set(0xA0,0x11,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.flag = 1;

	memcpy(pMsg.index,lpMsg->index,sizeof(pMsg.index));

	pMsg.relation = lpMsg->relation;

	memcpy(pMsg.GuildNumber,lpMsg->GuildNumber,sizeof(pMsg.GuildNumber));

	if((pMsg.result=gGuildManager.DelGuildRelationship(aIndex,lpMsg->GuildNumber[0],lpMsg->relation)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex)?1:0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::GDGuildUnionListRecv(SDHP_GUILD_UNION_LIST_RECV* lpMsg,int aIndex)
{
	BYTE send[2048];

	SDHP_GUILD_UNION_LIST_SEND pMsg;

	pMsg.header.set(0xA0,0x12,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.result = 0;

	pMsg.index = lpMsg->index;

	pMsg.time = 0;

	pMsg.UnionNumber = 0;

	pMsg.RivalNumber = 0;

	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->GuildNumber);

	if(lpGuildInfo != 0)
	{
		pMsg.result = 1;

		SDHP_GUILD_UNION_INFO info;

		DWORD GuildUnionNumber[MAX_GUILD_UNION];

		DWORD GuildRivalNumber[MAX_GUILD_RIVAL];

		pMsg.UnionNumber = (BYTE)gGuildManager.GetUnionList(lpGuildInfo->dwUnionNumber,GuildUnionNumber);

		pMsg.RivalNumber = (BYTE)gGuildManager.GetRivalList(lpGuildInfo->dwRivalNumber,GuildRivalNumber);

		for(int n=0;n < pMsg.UnionNumber;n++)
		{
			GUILD_INFO* lpGuild = gGuildManager.GetGuildInfo(GuildUnionNumber[n]);

			if(lpGuild != 0)
			{
				info.count = lpGuild->GetMemberCount();

				memcpy(info.GuildName,lpGuild->szName,sizeof(info.GuildName));

				memcpy(info.Mark,lpGuild->arMark,sizeof(info.Mark));

				memcpy(&send[size],&info,sizeof(info));

				size += sizeof(info);

				pMsg.count++;
			}
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(aIndex,send,size);
}

void CGuild::GDGuildAllianceKickRecv(SDHP_GUILD_ALLIANCE_KICK_RECV* lpMsg,int aIndex)
{
	SDHP_GUILD_ALLIANCE_KICK_SEND pMsg;

	pMsg.header.set(0xA0,0x13,sizeof(pMsg));

	pMsg.flag = 1;

	pMsg.index = lpMsg->index;

	pMsg.relation = lpMsg->relation;

	pMsg.result = 0;

	memcpy(pMsg.GuildNameA,lpMsg->GuildNameA,sizeof(pMsg.GuildNameA));

	memcpy(pMsg.GuildNameB,lpMsg->GuildNameB,sizeof(pMsg.GuildNameB));

	if((pMsg.result=gGuildManager.SetGuildRelationship(aIndex,lpMsg->GuildNameB,lpMsg->GuildNameA)) != 1)
	{
		gSocketManager.DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			pMsg.flag = ((n==aIndex)?1:0);
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::DGGuildMemberStatusSend(char* GuildName,char* Name,BYTE status,BYTE type,WORD server) // OK
{
	SDHP_GUILD_MEMBER_STATUS_SEND pMsg;

	pMsg.header.set(0xA0,0x05,sizeof(pMsg));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,Name,sizeof(pMsg.Name));

	pMsg.status = status;

	pMsg.type = type;

	pMsg.server = server;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::DGGuildInfoSend(DWORD GuildNumber) // OK
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(GuildNumber);

	if(lpGuildInfo == 0)
	{
		return;
	}

	BYTE send[2048];

	SDHP_GUILD_INFO_SEND pMsg;

	pMsg.header.set(0xA0,0x0B,0);

	int size = sizeof(pMsg);

	pMsg.GuildNumber = lpGuildInfo->dwNumber;

	memcpy(pMsg.GuildName,lpGuildInfo->szName,sizeof(pMsg.GuildName));

	memcpy(pMsg.GuildMaster,lpGuildInfo->szMaster,sizeof(pMsg.GuildMaster));

	memcpy(pMsg.Mark,lpGuildInfo->arMark,sizeof(pMsg.Mark));

	pMsg.score = lpGuildInfo->dwScore;

	pMsg.type = lpGuildInfo->btType;

	pMsg.GuildUnionNumber = lpGuildInfo->dwUnionNumber;

	pMsg.GuildRivalNumber = lpGuildInfo->dwRivalNumber;

	GUILD_INFO* lpRivalInfo = gGuildManager.GetGuildInfo(lpGuildInfo->dwRivalNumber);

	if(lpRivalInfo == 0)
	{
		memset(pMsg.GuildRivalName,0,sizeof(pMsg.GuildRivalName));
	}
	else
	{
		memcpy(pMsg.GuildRivalName,lpRivalInfo->szName,sizeof(pMsg.GuildRivalName));
	}

	pMsg.count = 0;

	SDHP_GUILD_MEMBER_INFO info;

	for(int n=0;n < MAX_GUILD_MEMBER;n++)
	{
		GUILD_MEMBER_INFO* lpGuildMemberInfo = &lpGuildInfo->arGuildMember[n];

		if(lpGuildMemberInfo->IsEmpty() == 0)
		{
			memcpy(info.Name,lpGuildMemberInfo->szGuildMember,sizeof(info.Name));

			info.status = lpGuildMemberInfo->btStatus;

			info.server = lpGuildMemberInfo->btServer;

			memcpy(&send[size],&info,sizeof(info));

			size += sizeof(info);

			pMsg.count++;
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,send,size);
		}
	}
}

void CGuild::DGGuildUnionInfoSend(DWORD GuildNumber,BYTE relation) // OK
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(GuildNumber);

	if(lpGuildInfo == 0)
	{
		return;
	}

	SDHP_GUILD_UNION_INFO_SEND pMsg;

	pMsg.header.set(0xA0,0x0C,sizeof(pMsg));

	pMsg.relation = relation;

	pMsg.count = 0;

	if(relation == 1)
	{
		pMsg.count = (BYTE)gGuildManager.GetUnionList(lpGuildInfo->dwNumber,(DWORD*)pMsg.GuildMembers);
	}
	else if (relation == 2)
	{
		pMsg.count = (BYTE)gGuildManager.GetRivalList(lpGuildInfo->dwNumber,(DWORD*)pMsg.GuildMembers);
	}

	memcpy(pMsg.GuildName,lpGuildInfo->szName,sizeof(pMsg.GuildName));

	pMsg.GuildNumber = lpGuildInfo->dwNumber;

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}

void CGuild::DGGuildUnionUpdateSend(BYTE update,DWORD count,int* GuildMembers) // OK
{
	SDHP_GUILD_UNION_UPDATE_SEND pMsg;

	pMsg.header.set(0xA0,0x0D,sizeof(pMsg));

	pMsg.update = update;

	pMsg.count = count;

	memcpy(pMsg.GuildMembers,GuildMembers,(count*sizeof(int)));

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n,(BYTE*)&pMsg,sizeof(pMsg));
		}
	}
}