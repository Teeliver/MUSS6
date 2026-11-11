// Guild.cpp: implementation of the CGuild class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Guild.h"
#include "ArcaBattle.h"
#include "BattleSoccer.h"
#include "BattleSoccerManager.h"
#include "CastleSiege.h"
#include "CastleSiegeSync.h"
#include "CustomArena.h"
#include "DefaultClassInfo.h"
#include "GameMain.h"
#include "GensSystem.h"
#include "GuildClass.h"
#include "GuildMatching.h"
#include "Map.h"
#include "MapManager.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Union.h"
#include "Util.h"
#include "Viewport.h"

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

void CGuild::DGGuildCreateRecv(SDHP_GUILD_CREATE_RECV* lpMsg) // OK
{
	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj != 0 && lpMsg->flag != 0)
	{
		if(lpMsg->result == 0)
		{
			this->GCGuildCreateResultSend(lpObj->Index,0,lpMsg->type);

			if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_GUILD_CREATE)
			{
				lpObj->Interface.use = 0;
			}
		}
		else
		{
			this->GCGuildCreateResultSend(lpObj->Index,lpMsg->result,lpMsg->type);
		}
	}

	if(lpMsg->result == 1)
	{
		GUILD_INFO_STRUCT* lpGuild = gGuildClass.AddGuild(lpMsg->GuildNumber,lpMsg->GuildName,lpMsg->Mark,lpMsg->Name,0);

		if(lpObj != 0 && lpMsg->flag != 0 && lpGuild != 0)
		{
			lpObj->Guild = lpGuild;
			
			lpObj->GuildNumber = lpMsg->GuildNumber;
			
			lpObj->GuildStatus = GUILD_STATUS_MASTER;
			
			lpGuild->GuildRival = GUILD_RELATION_NONE;
			
			lpGuild->GuildUnion = GUILD_RELATION_NONE;
			
			lpGuild->TimeStamp = 0;
			
			lpObj->GuildUnionTimeStamp = 0;
			
			memcpy(lpObj->GuildName,lpMsg->GuildName,sizeof(lpObj->GuildName));

			gViewport.GCViewportSimpleGuildSend(lpObj);
		}
	}
}

void CGuild::DGGuildDeleteRecv(SDHP_GUILD_DELETE_RECV* lpMsg) // OK
{
	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj != 0 && lpMsg->flag != 0 && strcmp(lpMsg->Name,lpObj->Name) == 0)
	{
		this->GCGuildDeleteSend(lpObj->Index,lpMsg->result);

		if(lpMsg->result == 1 || lpMsg->result == 4)
		{
			gViewport.DestroyViewportGuild(lpObj->Index,1);
		}
	}

	if(lpMsg->result == 1 || lpMsg->result == 4)
	{
		GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

		if(lpGuild == 0)
		{
			return;
		}

		GUILD_INFO_STRUCT* lpRival = gGuildClass.SearchGuild_Number(lpGuild->GuildRival);

		if(lpRival != 0)
		{
			lpRival->GuildRival = GUILD_RELATION_NONE;

			lpRival->GuildRivalName[0] = 0;
		}

		lpGuild->GuildUnion = GUILD_RELATION_NONE;

		lpGuild->GuildRival = GUILD_RELATION_NONE;

		for(int n=0;n < MAX_GUILD_USER;n++)
		{
			if(lpGuild->Use[n] > 0 && lpGuild->Index[n] >= 0)
			{
				if(gObjIsNameValid(lpGuild->Index[n],lpGuild->Names[n]) != 0)
				{
					LPOBJ lpTarget = &gObj[lpGuild->Index[n]];

					lpTarget->Guild = 0;

					lpTarget->GuildNumber = 0;

					lpTarget->GuildStatus = GUILD_STATUS_NONE;

					lpTarget->GuildUnionTimeStamp = 0;

					memset(lpTarget->GuildName,0,sizeof(lpTarget->GuildName));

					if(lpObj->Name != lpTarget->Name)
					{
						this->GCGuildDeleteSend(lpTarget->Index,1);

						gViewport.DestroyViewportGuild(lpTarget->Index,0);
					}

					gViewport.GCViewportSimpleUnionSend(lpTarget);
				}
			}
		}

		gUnionManager.DelUnion(lpGuild->Number);

		gGuildClass.DeleteGuild(lpMsg->GuildName);
	}
}

void CGuild::DGGuildMemberDisconnectRecv(SDHP_GUILD_MEMBER_DISCONNECT_RECV* lpMsg) // OK
{
	gGuildClass.CloseMember(lpMsg->GuildName,lpMsg->Name);
}

void CGuild::DGGuildMemberJoinRecv(SDHP_GUILD_MEMBER_JOIN_RECV* lpMsg) // OK
{
	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj != 0 && lpMsg->flag != 0)
	{
		this->GCGuildResultSend(lpObj->Index,lpMsg->result);
	}

	if(lpMsg->result == 1)
	{
		GUILD_INFO_STRUCT* lpGuild = gGuildClass.AddMember(lpMsg->GuildName,lpMsg->Name,lpObj->Index,-1,GUILD_STATUS_MEMBER,lpMsg->server);

		if(lpObj != 0 && lpMsg->flag != 0 && lpGuild != 0)
		{
			lpObj->Guild = lpGuild;

			lpObj->GuildStatus = GUILD_STATUS_MEMBER;

			lpObj->GuildNumber = lpGuild->Number;

			lpObj->GuildUnionTimeStamp = 0;

			memcpy(lpObj->GuildName,lpMsg->GuildName,sizeof(lpObj->GuildName));

			gViewport.GCViewportSimpleGuildSend(lpObj);
		}
	}
}

void CGuild::DGGuildMemberBreakRecv(SDHP_GUILD_MEMBER_BREAK_RECV* lpMsg) // OK
{
	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj != 0 && lpMsg->flag != 0)
	{
		if(strcmp(lpMsg->Name,lpObj->Name) == 0)
		{
			this->GCGuildDeleteSend(lpObj->Index,lpMsg->result);
		}
		else
		{
			this->GCGuildDeleteSend(lpObj->Index,5);
		}
	}

	if(lpMsg->result == 1)
	{
		LPOBJ lpTarget = gObjFind(lpMsg->Name);

		if(lpTarget != 0)
		{
			gViewport.DestroyViewportGuild(lpTarget->Index,0);
			
			lpTarget->Guild = 0;
			
			lpTarget->GuildNumber = 0;
			
			lpTarget->GuildStatus = GUILD_STATUS_NONE;
			
			lpTarget->GuildUnionTimeStamp = 0;
			
			memset(lpTarget->GuildName,0,sizeof(lpTarget->GuildName));
		}

		gGuildClass.DelMember(lpMsg->GuildName,lpMsg->Name);
	}
}

void CGuild::DGGuildMemberStatusRecv(SDHP_GUILD_MEMBER_STATUS_RECV* lpMsg) // OK
{
	LPOBJ lpObj = gObjFind(lpMsg->Name);

	if(lpObj != 0)
	{
		if(gServerInfo.m_ServerType == 1)
		{
			gCastleSiege.GetCsJoinSide(lpMsg->GuildName,&lpObj->CsJoinSide,&lpObj->CsGuildInvolved);
		
			gCastleSiege.NotifySelfCsJoinSide(lpObj->Index);
		}

		memcpy(lpObj->GuildName,lpMsg->GuildName,sizeof(lpObj->GuildName));

		lpObj->Guild = gGuildClass.SearchGuild(lpObj->GuildName);

		if(lpObj->Guild != 0)
		{
			lpObj->GuildStatus = lpMsg->status;

			lpObj->GuildNumber = lpObj->Guild->Number;
			
			gGuildClass.ConnectUser(lpObj->Guild,lpMsg->GuildName,lpObj->Name,lpObj->Index,lpMsg->server);

			if(strlen(lpObj->Guild->Notice) > 0)
			{
				gNotice.GCNoticeSend(lpObj->Index,2,0,0,0,0,0,"%s",lpObj->Guild->Notice);
			}

			BYTE send[256];

			PMSG_VIEWPORT_SEND pMsg;

			pMsg.header.set(0x65,0);

			int size = sizeof(pMsg);

			pMsg.count = 0;

			PMSG_VIEWPORT_GUILD info;

			info.number = lpObj->Guild->Number;

			info.status = lpObj->GuildStatus;

			info.type = lpObj->Guild->GuildType;

			info.relationship = 0;

			info.index[0] = SET_NUMBERHB(lpObj->Index) & 0x7F;
			info.index[1] = SET_NUMBERLB(lpObj->Index);

			if(strcmp(lpObj->Guild->Names[0],lpObj->Name) == 0)
			{
				info.index[0] |= GUILD_STATUS_MASTER;
			}

			#if(GAMESERVER_UPDATE>=401)

			if(gCastleSiegeSync.CheckCastleOwnerMember(lpObj->Index) != 0 || gCastleSiegeSync.CheckCastleOwnerUnionMember(lpObj->Index) != 0)
			{
				info.owner = 1;
			}
			else
			{
				info.owner = 0;
			}

			#endif

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;

			pMsg.header.size[0] = SET_NUMBERHB(size);
			pMsg.header.size[1] = SET_NUMBERLB(size);

			memcpy(send,&pMsg,sizeof(pMsg));

			DataSend(lpObj->Index,send,size);

			if(lpObj->Guild->WarState != GUILD_WAR_STATE_NONE)
			{
				this->GCGuildWarScoreSend(lpObj->Index);
				
				this->GCGuildWarDeclareSend(lpObj->Index,lpObj->Guild->TargetGuildName,GUILD_WAR_STATE_NONE,GUILD_WAR_TEAM_BLUE);
			}
		}
		else
		{
			if(gServerInfo.m_ServerType == 1)
			{
				lpObj->CsJoinSide = 0;
				lpObj->CsGuildInvolved = 0;
			}
		}
	}

	gGuildClass.SetServer(lpMsg->GuildName,lpMsg->Name,lpMsg->server);
}

void CGuild::DGGuildAssignStatusRecv(SDHP_GUILD_ASSIGN_STATUS_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

	if(lpMsg->flag == 1)
	{
		if(lpGuild == 0)
		{
			lpMsg->result = 0;
		}

		this->GCGuildAssignStatusSend(lpMsg->index,lpMsg->type,lpMsg->result,lpMsg->Name);

		if(lpMsg->result == 0)
		{
			return;
		}

		gGuildClass.SetGuildMemberStatus(lpMsg->GuildName,lpMsg->Name,lpMsg->status);
	}
	else
	{
		if(lpGuild == 0)
		{
			return;
		}

		gGuildClass.SetGuildMemberStatus(lpMsg->GuildName,lpMsg->Name,lpMsg->status);

		if(lpMsg->result == 0)
		{
			return;
		}
	}

	if(lpMsg->status == GUILD_STATUS_OFFICE_MASTER)
	{
		this->NoticeSendToAll(lpGuild,0,gMessage.GetMessage(190),lpMsg->Name);
	}
	else if(lpMsg->status == GUILD_STATUS_BATTLE_MASTER)
	{
		this->NoticeSendToAll(lpGuild,0,gMessage.GetMessage(191),lpMsg->Name);
	}
	else
	{
		this->NoticeSendToAll(lpGuild,0,gMessage.GetMessage(192),lpMsg->Name);
	}
}

void CGuild::DGGuildAssignTypeRecv(SDHP_GUILD_ASSIGN_TYPE_RECV*lpMsg)
{
	GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

	if(lpMsg->flag == 1)
	{
		if(lpGuild == 0)
		{
			lpMsg->result = 0;
		}

		this->GCGuildAssignTypeSend(lpMsg->index,lpMsg->type,lpMsg->result);

		if(lpMsg->result == 0)
		{
			return;
		}

		gGuildClass.SetGuildType(lpMsg->GuildName,lpMsg->type);
	}
	else
	{
		GUILD_INFO_STRUCT*lpNode = gGuildClass.SearchGuild(lpMsg->GuildName);

		if(lpNode == 0)
		{
			return;
		}

		gGuildClass.SetGuildType(lpMsg->GuildName,lpMsg->type);

		if(lpMsg->result == 0)
		{
			return;
		}
	}

	this->NoticeSendToAll(lpGuild,0,gMessage.GetMessage(193),lpMsg->GuildName,lpMsg->type);
}

void CGuild::DGGuildScoreUpdateRecv(SDHP_GUILD_SCORE_UPDATE_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

	if(lpGuild != 0)
	{
		lpGuild->TotalScore = lpMsg->score;
	}
}

void CGuild::DGGuildGlobalChatRecv(SDHP_GUILD_GLOBAL_CHAT_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

	if(lpGuild == 0)
	{
		return;
	}

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	memcpy(pMsg.name,lpMsg->Name,sizeof(pMsg.name));

	memcpy(pMsg.message,lpMsg->message,sizeof(pMsg.message));

	if(lpMsg->type == 0)
	{
		this->DataSendToAll(lpGuild,(BYTE*)&pMsg,pMsg.header.size);
	}
	else
	{
		int UnionCount = 0;

		int UnionGuildList[100] = {0};

		if(gUnionManager.GetGuildUnionMemberList(lpGuild->GuildUnion,UnionCount,UnionGuildList) != 0)
		{
			for(int i=0;i < UnionCount;i++)
			{
				GUILD_INFO_STRUCT* lpTarget = gGuildClass.SearchGuild_Number(UnionGuildList[i]);

				this->DataSendToAll(lpTarget,(BYTE*)&pMsg,pMsg.header.size);
			}
		}
	}
}

void CGuild::DGGuildGlobalNoticeRecv(SDHP_GUILD_GLOBAL_NOTICE_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild(lpMsg->GuildName);

	if(lpGuild == 0)
	{
		return;
	}

	if(lpGuild->Count == 0)
	{
		return;
	}

	memcpy(lpGuild->Notice,lpMsg->message,sizeof(lpGuild->Notice));

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] > 0 && lpGuild->Index[n] >= 0)
		{
			if(gObjIsNameValid(lpGuild->Index[n],lpGuild->Names[n]) != 0)
			{
				gNotice.GCNoticeSend(lpGuild->Index[n],2,0,0,0,0,0,"%s",lpMsg->message);
			}
		}
	}
}

void CGuild::DGGuildInfoRecv(SDHP_GUILD_INFO_RECV* lpMsg) // OK
{
	gGuildClass.AddGuild(lpMsg->GuildNumber,lpMsg->GuildName,lpMsg->Mark,lpMsg->GuildMaster,lpMsg->score);
	
	gGuildClass.SetGuildType(lpMsg->GuildMaster,lpMsg->type);

	GUILD_INFO_STRUCT*lpGuild = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber);

	if(lpGuild != 0)
	{
		lpGuild->GuildUnion = lpMsg->GuildUnionNumber;

		lpGuild->GuildRival = lpMsg->GuildRivalNumber;

		memcpy(lpGuild->GuildRivalName,lpMsg->GuildRivalName,sizeof(lpGuild->GuildRivalName));
	}

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_GUILD_MEMBER_INFO* lpInfo = (SDHP_GUILD_MEMBER_INFO*)(((BYTE*)lpMsg)+sizeof(SDHP_GUILD_INFO_RECV)+(sizeof(SDHP_GUILD_MEMBER_INFO)*n));

		if(gGuildClass.AddMember(lpMsg->GuildName,lpInfo->Name,-1,lpMsg->count,lpInfo->status,lpInfo->server) == 0)
		{
			break;
		}
	}
}

void CGuild::DGGuildUnionInfoRecv(SDHP_GUILD_UNION_INFO_RECV*lpMsg)
{
	if(lpMsg->relation == GUILD_RELATION_UNION)
	{
		gUnionManager.AddUnion(lpMsg->GuildNumber,lpMsg->GuildName);
		gUnionManager.SetGuildUnionMemberList(lpMsg->GuildNumber,lpMsg->count,lpMsg->GuildMembers);
	}
	else if(lpMsg->relation == GUILD_RELATION_RIVAL)
	{
		gUnionManager.AddUnion(lpMsg->GuildNumber,lpMsg->GuildName);
		gUnionManager.SetGuildRivalMemberList(lpMsg->GuildNumber,lpMsg->count,lpMsg->GuildMembers);
	}
}

void CGuild::DGGuildUnionUpdateRecv(SDHP_GUILD_UNION_UPDATE_RECV* lpMsg) // OK
{
	if(lpMsg->update == 16)
	{
		GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild_Number(lpMsg->GuildMembers[0]);

		if(lpGuild != 0)
		{
			lpGuild->GuildUnion = GUILD_RELATION_NONE;

			lpGuild->SetTimeStamp();

			this->NoticeSendToAll(lpGuild,1,gMessage.GetMessage(201),lpGuild->Name);
		}
	}

	for(DWORD i=0;i < lpMsg->count;i++)
	{
		GUILD_INFO_STRUCT* lpGuild = gGuildClass.SearchGuild_Number(lpMsg->GuildMembers[i]);

		if(lpGuild != 0)
		{
			lpGuild->SetTimeStamp();
		}
	}
}

void CGuild::DGGuildUnionJoinRecv(SDHP_GUILD_UNION_JOIN_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuildA = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber[0]);

	GUILD_INFO_STRUCT* lpGuildB = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber[1]);

	if(lpMsg->flag == 1)
	{
		if(lpGuildA == 0 || lpGuildB == 0)
		{
			lpMsg->result = 0;
		}

		this->GCGuildUnionResultSend(lpMsg->index[0],lpMsg->relation,GUILD_TYPE_JOIN,lpMsg->result,lpMsg->index[1]);

		this->GCGuildUnionResultSend(lpMsg->index[1],lpMsg->relation,GUILD_TYPE_JOIN,lpMsg->result,lpMsg->index[0]);
	}
	else
	{
		if(lpGuildA == 0 || lpGuildB == 0)
		{
			return;
		}
	}

	if (lpMsg->result == 0 || lpMsg->result == 16)
	{
		return;
	}

	if(lpMsg->relation == GUILD_RELATION_UNION)
	{
		lpGuildA->SetGuildUnion(lpGuildB->Number);
		
		lpGuildB->SetGuildUnion(lpGuildB->Number);
	}
	else if(lpMsg->relation == GUILD_RELATION_RIVAL)
	{
		lpGuildA->SetGuildRival(lpGuildB->Number);
		
		lpGuildB->SetGuildRival(lpGuildA->Number);

		memcpy(lpGuildA->GuildRivalName,lpGuildB->Name,sizeof(lpGuildA->GuildRivalName));
		
		memcpy(lpGuildB->GuildRivalName,lpGuildA->Name,sizeof(lpGuildB->GuildRivalName));
	}

	if(lpMsg->relation == GUILD_RELATION_UNION)
	{
		this->NoticeSendToAll(lpGuildA,1,gMessage.GetMessage(194),lpGuildB->Name);

		this->NoticeSendToAll(lpGuildB,1,gMessage.GetMessage(196),lpGuildA->Name,lpGuildB->Name);
	}
	else if(lpMsg->relation == GUILD_RELATION_RIVAL)
	{
		this->NoticeSendToAll(lpGuildA,1,gMessage.GetMessage(195),lpGuildB->Name);
		
		this->NoticeSendToAll(lpGuildB,1,gMessage.GetMessage(195),lpGuildA->Name);
	}
}

void CGuild::DGGuildUnionBreakRecv(SDHP_GUILD_UNION_BREAK_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuildA = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber[0]);

	GUILD_INFO_STRUCT* lpGuildB = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber[1]);

	if(lpMsg->flag == 1)
	{
		if(lpMsg->relation == GUILD_RELATION_UNION)
		{
			if(lpGuildA == 0)
			{
				lpMsg->result = 0;
			}

			this->GCGuildUnionResultSend(lpMsg->index[0],lpMsg->relation,GUILD_TYPE_BREAK,lpMsg->result,lpMsg->index[1]);
		}
		else if(lpMsg->relation == GUILD_RELATION_RIVAL)
		{
			if(lpGuildA == 0 || lpGuildB == 0)
			{
				lpMsg->result = 0;
			}

			this->GCGuildUnionResultSend(lpMsg->index[0],lpMsg->relation,GUILD_TYPE_BREAK,lpMsg->result,lpMsg->index[1]);

			this->GCGuildUnionResultSend(lpMsg->index[1],lpMsg->relation,GUILD_TYPE_BREAK,lpMsg->result,lpMsg->index[0]);
		}
	}
	else
	{
		if(lpMsg->relation == GUILD_RELATION_UNION && lpGuildA == 0)
		{
			return;
		}
		else if(lpMsg->relation == GUILD_RELATION_RIVAL && (lpGuildA == 0 || lpGuildB == 0))
		{
			return;
		}
	}

	if(lpMsg->result == 0 || lpMsg->result == 16)
	{
		return;
	}

	if(lpMsg->relation == GUILD_RELATION_UNION)
	{
		lpGuildA->SetGuildUnion(0);
	}
	else if(lpMsg->relation == GUILD_RELATION_RIVAL)
	{
		lpGuildA->SetGuildRival(0);

		lpGuildB->SetGuildRival(0);

		lpGuildA->GuildRivalName[0] = 0;

		lpGuildB->GuildRivalName[0] = 0;
	}

	if (lpMsg->relation == GUILD_RELATION_UNION)
	{
		this->NoticeSendToAll(lpGuildA,1,gMessage.GetMessage(197));
	}
	else if(lpMsg->relation == GUILD_RELATION_RIVAL)
	{
		this->NoticeSendToAll(lpGuildA,1,gMessage.GetMessage(200),lpGuildB->Name);

		this->NoticeSendToAll(lpGuildB,1,gMessage.GetMessage(200),lpGuildA->Name);
	}
}

void CGuild::DGGuildUnionListRecv(SDHP_GUILD_UNION_LIST_RECV* lpMsg) // OK
{
	BYTE send[4096];

	PMSG_GUILD_UNION_LIST_SEND pMsg;

	pMsg.header.set(0xE9,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.result = lpMsg->result;
	
	pMsg.UnionNumber = lpMsg->UnionNumber;

	pMsg.RivalNumber = lpMsg->RivalNumber;

	PMSG_GUILD_UNION_LIST info;

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_GUILD_UNION_INFO* lpInfo = (SDHP_GUILD_UNION_INFO*)(((BYTE*)lpMsg)+sizeof(SDHP_GUILD_UNION_LIST_RECV)+(sizeof(SDHP_GUILD_UNION_INFO)*n));

		info.count = lpInfo->count;

		memcpy(info.Mark,lpInfo->Mark,sizeof(info.Mark));

		memcpy(info.GuildName,lpInfo->GuildName,sizeof(info.GuildName));

		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpMsg->index,send,size);
}

void CGuild::DGGuildAllianceKickRecv(SDHP_GUILD_ALLIANCE_KICK_RECV* lpMsg) // OK
{
	GUILD_INFO_STRUCT* lpGuildA = gGuildClass.SearchGuild(lpMsg->GuildNameA);

	GUILD_INFO_STRUCT* lpGuildB = gGuildClass.SearchGuild(lpMsg->GuildNameB);

	if(lpMsg->flag == 1)
	{
		if(lpGuildA == 0)
		{
			lpMsg->result = 0;
		}

		PMSG_GUILD_ALLIANCE_KICK_RESULT_SEND pMsg;

		pMsg.header.set(0xEB,0x01,sizeof(pMsg));
		
		pMsg.result = lpMsg->result;
		
		pMsg.type = GUILD_TYPE_BREAK;

		pMsg.relation = GUILD_RELATION_UNION;

		DataSend(lpMsg->index,(BYTE*)&pMsg,sizeof(pMsg));
	}
	else
	{
		if(lpGuildB == 0)
		{
			return;
		}
	}

	if(lpMsg->result == 0 || lpMsg->result == 16)
	{
		return;
	}

	if(lpGuildB != 0)
	{
		lpGuildB->SetGuildUnion(0);
	}

	this->NoticeSendToAll(lpGuildA,1,gMessage.GetMessage(198),lpMsg->GuildNameA,lpMsg->GuildNameB);

	this->NoticeSendToAll(lpGuildB,1,gMessage.GetMessage(197));
}

void CGuild::GDGuildMemberDisconnectSend(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Guild == 0)
	{
		return;
	}

	SDHP_GUILD_MEMBER_DISCONNECT_SEND pMsg;

	pMsg.header.set(0xA0,0x02,sizeof(pMsg));

	memcpy(pMsg.GuildName,lpObj->Guild->Name,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,lpObj->Name,sizeof(pMsg.Name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::GDGuildMemberJoinSend(int aIndex,char* GuildName,char* Name) // OK
{
	SDHP_GUILD_MEMBER_JOIN_SEND pMsg;

	pMsg.header.set(0xA0,0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,Name,sizeof(pMsg.Name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::GDGuildMemberBreakSend(int aIndex,char* GuildName,char* Name) // OK
{
	SDHP_GUILD_MEMBER_BREAK_SEND pMsg;

	pMsg.header.set(0xA0,0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,Name,sizeof(pMsg.Name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::GDGuildMemberGetInfoSend(int aIndex,char* Name) // OK
{
	SDHP_GUILD_MEMBER_GET_INFO_SEND pMsg;

	pMsg.header.set(0xA0,0x05,sizeof(pMsg));

	memcpy(pMsg.Name,Name,sizeof(pMsg.Name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::GDGuildScoreUpdateSend(char* GuildName,int score) // OK
{
	SDHP_GUILD_SCORE_UPDATE_SEND pMsg;

	pMsg.header.set(0xA0,0x08,sizeof(pMsg));

	pMsg.score = score;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,-1);
}

void CGuild::GDGuildGlobalChatSend(int type,char* GuildName,char* Name,char* message) // OK
{
	SDHP_GUILD_GLOBAL_CHAT_SEND pMsg;

	pMsg.header.set(0xA0,0x09,sizeof(pMsg));

	pMsg.type = type;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,Name,sizeof(pMsg.Name));

	memcpy(pMsg.message,message,sizeof(pMsg.message));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,-1);
}

void CGuild::GDGuildGlobalNoticeSend(char* GuildName,char* message) // OK
{
	SDHP_GUILD_GLOBAL_NOTICE_SEND pMsg;

	pMsg.header.set(0xA0,0x0A,sizeof(pMsg));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	memcpy(pMsg.message,message,sizeof(pMsg.message));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,-1);
}

void CGuild::CGGuildRequestRecv(PMSG_GUILD_REQUEST_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if((lpTarget->Option & 1) == 0)
	{
		this->GCGuildResultSend(aIndex,0);
		return;
	}

	if(lpTarget->GuildNumber == 0)
	{
		this->GCGuildResultSend(aIndex,4);
		return;
	}

	if(strcmp(lpTarget->Guild->Names[0],lpTarget->Name) != 0)
	{
		this->GCGuildResultSend(aIndex,4);
		return;
	}

	if(lpObj->GuildNumber > 0)
	{
		this->GCGuildResultSend(aIndex,5);
		return;
	}

	if(lpObj->Interface.use != 0 || lpTarget->Interface.use != 0)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	if(lpTarget->Guild->WarState != 0)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	if(lpObj->Level < 6)
	{
		this->GCGuildResultSend(aIndex,7);
		return;
	}

	if(lpTarget->Guild->TotalCount >= this->GetMaxGuildUser(lpTarget))
	{
		this->GCGuildResultSend(aIndex,2);
		return;
	}

	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,163);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[bIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,161);
		return;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily != gObj[bIndex].GensFamily)
	{
		this->GCGuildResultSend(aIndex,162);
		return;
	}

	#endif

	if((lpTarget->Helper.Started != 0 || lpTarget->Helper.Offline != 0) && lpTarget->Helper.AutoAcceptGuild != 0)
	{
		gGuildMatching.GDGuildMatchingJoinCancelSend(aIndex,1);

		this->GDGuildMemberJoinSend(aIndex,lpTarget->Guild->Name,lpObj->Name);

		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_GUILD;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = bIndex;

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_GUILD;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = aIndex;

	PMSG_GUILD_REQUEST_SEND pMsg;

	pMsg.header.set(0x50,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);

	DataSend(bIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::CGGuildResultRecv(PMSG_GUILD_RESULT_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_GUILD)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_GUILD)
	{
		return;
	}

	if(lpMsg->result == 0)
	{
		this->GCGuildResultSend(bIndex,0);
		goto CLEAR_JUMP;
	}

	if((lpObj->Option & 1) == 0)
	{
		this->GCGuildResultSend(bIndex,0);
		goto CLEAR_JUMP;
	}

	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildResultSend(bIndex,4);
		goto CLEAR_JUMP;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		this->GCGuildResultSend(bIndex,4);
		goto CLEAR_JUMP;
	}

	if(lpTarget->GuildNumber > 0)
	{
		this->GCGuildResultSend(bIndex,5);
		goto CLEAR_JUMP;
	}

	if(lpObj->Guild->WarState != 0)
	{
		this->GCGuildResultSend(bIndex,6);
		goto CLEAR_JUMP;
	}

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildResultSend(bIndex,6);
		goto CLEAR_JUMP;
	}

	if(lpTarget->Level < 6)
	{
		this->GCGuildResultSend(bIndex,7);
		goto CLEAR_JUMP;
	}

	if(lpObj->Guild->TotalCount >= this->GetMaxGuildUser(lpObj))
	{
		this->GCGuildResultSend(bIndex,2);
		goto CLEAR_JUMP;
	}

	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,163);
		goto CLEAR_JUMP;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[bIndex].GensFamily == GENS_FAMILY_NONE)
	{
		this->GCGuildResultSend(aIndex,161);
		goto CLEAR_JUMP;
	}

	if(gServerInfo.m_GensSystemGuildLock != 0 && gObj[aIndex].GensFamily != gObj[bIndex].GensFamily)
	{
		this->GCGuildResultSend(aIndex,162);
		goto CLEAR_JUMP;
	}

	#endif

	gGuildMatching.GDGuildMatchingJoinCancelSend(bIndex,1);

	this->GDGuildMemberJoinSend(bIndex,lpObj->Guild->Name,lpTarget->Name);

	CLEAR_JUMP:

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = 0;
	lpObj->TargetNumber = -1;

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = 0;
	lpTarget->TargetNumber = -1;
}

void CGuild::CGGuildListRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0 || lpObj->GuildNumber == 0)
	{
		return;
	}

	BYTE send[2048];

	PMSG_GUILD_LIST_SEND pMsg;

	pMsg.header.set(0x52,0);

	int size = sizeof(pMsg);

	pMsg.result = 1;

	pMsg.count = 0;

	pMsg.TotalScore = lpObj->Guild->TotalScore;

	pMsg.score = lpObj->Guild->PlayScore;

	GUILD_INFO_STRUCT* lpRivalGuild = gGuildClass.SearchGuild_Number(lpObj->Guild->GuildRival);

	if(lpRivalGuild == 0)
	{
		memset(pMsg.RivalGuild,0,sizeof(pMsg.RivalGuild));

		#if(GAMESERVER_UPDATE>=701)

		pMsg.RivalCount = 0;

		memset(pMsg.RivalMark,0,sizeof(pMsg.RivalMark));

		#endif
	}
	else
	{
		memcpy(pMsg.RivalGuild,lpRivalGuild->Name,sizeof(pMsg.RivalGuild));

		#if(GAMESERVER_UPDATE>=701)

		pMsg.RivalCount = lpRivalGuild->Count;

		memcpy(pMsg.RivalMark,lpRivalGuild->Mark,sizeof(pMsg.RivalMark));

		#endif
	}

	PMSG_GUILD_LIST info;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] == 0)
		{
			continue;
		}

		memcpy(info.name,lpObj->Guild->Names[n],sizeof(info.name));

		info.number = lpObj->Guild->Server[n];

		info.connected = (lpObj->Guild->Server[n] & 0x7F) | ((lpObj->Guild->Server[n]>=0)?GUILD_STATUS_MASTER:GUILD_STATUS_MEMBER);

		info.status = lpObj->Guild->GuildStatus[n];

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(aIndex,send,size);
	}
}

void CGuild::CGGuildDeleteRecv(PMSG_GUILD_DELETE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(gServerInfo.m_GuildDeleteSwitch == 0)
	{
		this->GCGuildDeleteSend(aIndex,0);
		return;
	}

	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildDeleteSend(aIndex,0);
		return;
	}

	if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildDeleteSend(aIndex,0);
		return;
	}

	char name[11] = {0};

	char PersonalCode[14] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));

	memcpy(PersonalCode,lpMsg->PersonalCode,sizeof(lpMsg->PersonalCode));

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) == 0)
	{
		if(strcmp(lpObj->Name,name) != 0)
		{
			if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
			{
				this->GCGuildDeleteSend(aIndex,0);
				return;
			}
			else
			{
				GDGuildMemberBreakSend(aIndex,lpObj->Guild->Name,name);
				return;
			}
		}

		if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
		{
			this->GCGuildDeleteSend(aIndex,0);
			return;
		}

		if(gServerInfo.m_GuildOwnerDestroyLimit != 0 && gCastleSiegeSync.CheckCastleOwnerMember(aIndex) != 0)
		{
			this->GCGuildDeleteSend(aIndex,0);
			return;
		}

		gObjGuildWarMasterClose(lpObj);

		gGuildMatching.GDGuildMatchingCancelSend(aIndex,1);

		SDHP_GUILD_DELETE_SEND pMsg;

		pMsg.header.set(0xA0,0x01,sizeof(pMsg));

		pMsg.index = aIndex;

		memcpy(pMsg.GuildName,lpObj->Guild->Name,sizeof(pMsg.GuildName));

		memcpy(pMsg.Name,name,sizeof(pMsg.Name));

		gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
	}
	else
	{
		if(strcmp(lpObj->Name,name) != 0)
		{
			this->GCGuildDeleteSend(aIndex,0);
			return;
		}

		if(gObjCheckPersonalCode(aIndex,PersonalCode) == 0)
		{
			this->GCGuildDeleteSend(aIndex,0);
			return;
		}

		GDGuildMemberBreakSend(aIndex,lpObj->Guild->Name,name);
	}
}

void CGuild::CGGuildMasterOpenRecv(PMSG_GUILD_MASTER_OPEN_RECV*lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->result == 1)
	{
		if(lpObj->Level < gServerInfo.m_GuildCreateMinLevel[lpObj->AccountLevel])
		{
			goto CLEAR_JUMP;
		}

		if(lpObj->Reset < gServerInfo.m_GuildCreateMinReset[lpObj->AccountLevel])
		{
			goto CLEAR_JUMP;
		}

		PBMSG_HEAD pMsg;

		pMsg.set(0x55,sizeof(pMsg));

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.size); 

		return;
	}
	else
	{
		goto CLEAR_JUMP;
	}

	CLEAR_JUMP:

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_GUILD_CREATE)
	{
		lpObj->Interface.use = 0;
	}
}

void CGuild::CGGuildCreateRecv(PMSG_GUILD_CREATE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	char GuildName[9] = {0};

	memcpy(GuildName,lpMsg->GuildName,sizeof(lpMsg->GuildName));

	if(strlen(GuildName) < 3 || strlen(GuildName) > 9)
	{
		this->GCGuildCreateResultSend(aIndex,2,0);
	}
	else
	{
		gGuildMatching.GDGuildMatchingJoinCancelSend(aIndex,1);

		SDHP_GUILD_CREATE_SEND pMsg;

		pMsg.header.set(0xA0,0x00,sizeof(pMsg));

		pMsg.index = aIndex;

		memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

		memcpy(pMsg.Name,lpObj->Name,sizeof(pMsg.Name));

		memcpy(pMsg.Mark,lpMsg->Mark,sizeof(pMsg.Mark));

		pMsg.type = lpMsg->type;

		gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_GUILD_CREATE)
	{
		lpObj->Interface.use = 0;
	}
}

void CGuild::CGGuildMasterCancelRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->GuildNumber > 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type == INTERFACE_GUILD_CREATE)
	{
		lpObj->Interface.use = 0;
	}
}

void CGuild::CGGuildWarRequestResultRecv(PMSG_GUILD_WAR_REQUEST_RESULT_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	GUILD_INFO_STRUCT* lpGuild1 = lpObj->Guild;

	if(lpGuild1 == 0)
	{
		this->GCGuildWarResultSend(aIndex,0);
		return;
	}

	GUILD_INFO_STRUCT* lpGuild2 = lpGuild1->TargetGuildNode;

	if(lpGuild2 != 0 && lpMsg->result != 0)
	{
		LPOBJ lpTarget = &gObj[lpGuild2->Index[0]];

		if(gCustomArena.CheckMap(lpTarget->Map) != 0 || BC_MAP_RANGE(lpTarget->Map) != 0 || CC_MAP_RANGE(lpTarget->Map) != 0 || DS_MAP_RANGE(lpTarget->Map) != 0 || IT_MAP_RANGE(lpTarget->Map) != 0)
		{
			lpMsg->result = 0;
		}
	}

	if(lpMsg->result == 0)
	{
		if(lpGuild2 != 0)
		{
			if(lpGuild2->WarDeclareState == GUILD_WAR_STATE_DECLARE)
			{
				lpGuild1->WarDeclareState = GUILD_WAR_STATE_NONE;

				lpGuild2->WarDeclareState = GUILD_WAR_STATE_NONE;

				if(lpGuild2->Index[0] >= 0)
				{
					this->GCGuildWarResultSend(lpGuild2->Index[0],6);
				}
			}
		}
	}
	else
	{
		if(lpGuild1->WarDeclareState == GUILD_WAR_STATE_DECLARE)
		{
			if(lpGuild2 != 0)
			{
				if(lpGuild2->WarDeclareState == GUILD_WAR_STATE_DECLARE)
				{
					if(lpGuild1->WarType == GUILD_WAR_TYPE_SOCCER)
					{
						lpGuild1->BattleGroundIndex = gCheckBlankBattleGround();

						if (lpGuild1->BattleGroundIndex == 255)
						{
							lpGuild1->WarDeclareState = GUILD_WAR_STATE_NONE;
							lpGuild1->WarState = GUILD_WAR_STATE_NONE;

							lpGuild2->WarDeclareState = GUILD_WAR_STATE_NONE;
							lpGuild2->WarState = GUILD_WAR_STATE_NONE;

							this->GCGuildWarResultSend(aIndex,4);

							return;
						}

						lpGuild2->BattleGroundIndex = lpGuild1->BattleGroundIndex;

						if(gBSGround[0]->m_BallIndex >= 0)
						{
							gObjMonsterRegen(&gObj[gBSGround[0]->m_BallIndex]);
						}

						BattleSoccerGoalEnd(0);
						
						lpGuild1->PlayScore = 0;
						lpGuild2->PlayScore = 0;

						gBattleGroundEnable(lpGuild1->BattleGroundIndex,1);

						gSetBattleTeamMaster(lpGuild1->BattleGroundIndex,GUILD_WAR_TEAM_BLUE,lpGuild1->Name,lpGuild1);
						
						gSetBattleTeamMaster(lpGuild1->BattleGroundIndex,GUILD_WAR_TEAM_RED,lpGuild2->Name,lpGuild2);
					}

					lpGuild1->WarDeclareState = GUILD_WAR_STATE_ACTIVE;
					lpGuild1->WarState = GUILD_WAR_STATE_DECLARE;
					lpGuild1->PlayScore = 0;
					lpGuild1->BattleTeamCode = GUILD_WAR_TEAM_BLUE;

					lpGuild2->WarDeclareState = GUILD_WAR_STATE_ACTIVE;
					lpGuild2->WarState = GUILD_WAR_STATE_DECLARE;
					lpGuild2->PlayScore = 0;
					lpGuild2->BattleTeamCode = GUILD_WAR_TEAM_RED;

					for(int n=0;n < MAX_GUILD_USER;n++)
					{
						if(lpGuild1->Use[n] && lpGuild1->Index[n] >= 0)
						{
							if(n > 0 && lpGuild1->WarType == 1)
							{
								if(gObj[lpGuild1->Index[0]].PartyNumber >= 0 && gObj[lpGuild1->Index[0]].PartyNumber == gObj[lpGuild1->Index[n]].PartyNumber)
								{
									this->GuildWarDeclareSet(lpGuild1->Index[n],lpGuild2->Name,lpGuild2->WarType,lpGuild2->BattleTeamCode);
								}
							}
							else
							{
								this->GuildWarDeclareSet(lpGuild1->Index[n],lpGuild2->Name,lpGuild2->WarType,lpGuild2->BattleTeamCode);
							}
						}
					}

					for(int n=0;n < MAX_GUILD_USER;n++)
					{
						if(lpGuild2->Use[n] && lpGuild2->Index[n] >= 0)
						{
							if(n > 0 && lpGuild1->WarType == 1)
							{
								if(gObj[lpGuild2->Index[0]].PartyNumber >= 0 && gObj[lpGuild2->Index[0]].PartyNumber == gObj[lpGuild2->Index[n]].PartyNumber)
								{
									this->GuildWarDeclareSet(lpGuild2->Index[n],lpGuild1->Name,lpGuild2->WarType,lpGuild2->BattleTeamCode);
								}
							}
							else 
							{
								this->GuildWarDeclareSet(lpGuild2->Index[n],lpGuild1->Name,lpGuild2->WarType,lpGuild2->BattleTeamCode);
							}
						}
					}

					if(lpGuild1->WarType == GUILD_WAR_TYPE_SOCCER)
					{
						gObjAddMsgSendDelay(&gObj[aIndex],5,aIndex,10000,0);

						this->NoticeSendToAll(lpGuild1,1,gMessage.GetMessage(287));

						this->NoticeSendToAll(lpGuild2,1,gMessage.GetMessage(287));					
					}
				}
			}
		}
	}
}

void CGuild::CGGuildViewportRequestRecv(PMSG_GUILD_VIEWPORT_REQUEST_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	GUILD_INFO_STRUCT* lpInfo = gGuildClass.SearchGuild_Number(lpMsg->GuildNumber);

	if(lpInfo == 0)
	{
		return;
	}

	PMSG_GUILD_VIEWPORT_SEND pMsg;

	pMsg.header.set(0x66,sizeof(pMsg));

	pMsg.GuildNumber = lpInfo->Number;

	pMsg.type = lpInfo->GuildType;

	CUnionInfo* lpUnionInfo = gUnionManager.SearchUnion(lpInfo->GuildUnion);

	if(lpUnionInfo == 0)
	{
		memset(pMsg.UnionName,0,sizeof(pMsg.UnionName));
	}
	else
	{
		memcpy(pMsg.UnionName,lpUnionInfo->m_szMasterGuild,sizeof(pMsg.UnionName));
	}

	memcpy(pMsg.GuildName,lpInfo->Name,sizeof(pMsg.GuildName));

	memcpy(pMsg.Mark,lpInfo->Mark,sizeof(pMsg.Mark));

	DataSend(aIndex,(BYTE*)&pMsg,sizeof(pMsg));
}

void CGuild::CGGuildAssignStatusRecv(PMSG_GUILD_ASSIGN_STATUS_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->type != 1 && lpMsg->type != 2 && lpMsg->type != 3)
	{
		return;
	}

	char name[11] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));

	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,16,name);
		return;
	}

	if(strcmp(lpObj->Name,name) == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(this->GetGuildNameCount(lpObj,name) == 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,17,name);
		return;
	}

	if(lpMsg->status != GUILD_STATUS_MEMBER && lpMsg->status != GUILD_STATUS_BATTLE_MASTER && lpMsg->status != GUILD_STATUS_OFFICE_MASTER)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	if(lpMsg->status == GUILD_STATUS_BATTLE_MASTER && this->GetGuildBattleMasterCount(lpObj) >= (((lpObj->Level+lpObj->MasterLevel)/200)+1))
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	if(lpMsg->status == GUILD_STATUS_OFFICE_MASTER && this->GetGuildOfficeMasterCount(lpObj) > 0)
	{
		this->GCGuildAssignStatusSend(aIndex,lpMsg->type,18,name);
		return;
	}

	SDHP_GUILD_ASSIGN_STATUS_SEND pMsg;

	pMsg.header.set(0xA0,0x06,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.type = lpMsg->type;

	pMsg.status = lpMsg->status;

	memcpy(pMsg.GuildName,lpObj->Guild->Name,sizeof(pMsg.GuildName));

	memcpy(pMsg.Name,name,sizeof(pMsg.Name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::CGGuildAssignTypeRecv(PMSG_GUILD_ASSIGN_TYPE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}
	
	if(lpObj->GuildNumber == 0)
	{
		this->GCGuildAssignTypeSend(aIndex,lpMsg->type,16);
		return;
	}

	if(lpObj->GuildStatus != GUILD_STATUS_MASTER)
	{
		this->GCGuildAssignTypeSend(aIndex,lpMsg->type,17);
		return;
	}
	
	SDHP_GUILD_ASSIGN_TYPE_SEND pMsg;

	pMsg.header.set(0xA0,0x07,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.type = lpMsg->type;
	
	memcpy(pMsg.GuildName,lpObj->Guild->Name,sizeof(pMsg.GuildName));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::CGGuildUnionRequestRecv(PMSG_GUILD_UNION_REQUEST_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(IT_MAP_RANGE(lpObj->Map) != 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if((lpTarget->Option & 1) == 0)
	{
		return;
	}

	if(gArcaBattle.GetState() > AB_STATE_EMPTY && gArcaBattle.GetState() < AB_STATE_CLEAN)
	{
		this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,33,bIndex);
		return;
	}

	if(gCastleSiegeSync.GetCastleState() >= CASTLESIEGE_STATE_NOTIFY && gCastleSiegeSync.GetCastleState() <= CASTLESIEGE_STATE_STARTSIEGE)
	{
		this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,16,bIndex);
		return;
	}

	if(lpObj->Guild == 0 || lpTarget->Guild == 0)
	{
		this->GCGuildResultSend(aIndex,5);
		return;
	}

	if(lpObj->GuildStatus != GUILD_STATUS_MASTER && lpTarget->GuildStatus != GUILD_STATUS_MASTER)
	{
		this->GCGuildResultSend(aIndex,4);
		return;
	}

	if(lpObj->Interface.use != 0 || lpTarget->Interface.use != 0)
	{
		this->GCGuildResultSend(aIndex,6);
		return;
	}

	if(lpObj->GuildStatus != GUILD_STATUS_MASTER || lpTarget->GuildStatus != GUILD_STATUS_MASTER)
	{
		this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,17,bIndex);
		return;
	}

	if(lpMsg->type == GUILD_TYPE_JOIN)
	{
		if(lpMsg->relation == GUILD_RELATION_UNION)
		{
			if(lpTarget->Guild->Count < gServerInfo.m_GuildAllianceMinUser)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,29,bIndex);
				return;
			}

			if(lpTarget->Guild->GuildUnion != 0 && lpTarget->Guild->GuildUnion == lpTarget->Guild->Number)
			{
				if(gUnionManager.GetGuildRelationShipCount(lpTarget->Guild->GuildUnion,1) >= ((gServerInfo.m_GuildAllianceMaxCount>MAX_UNION_MEMBER)?MAX_UNION_MEMBER:gServerInfo.m_GuildAllianceMaxCount) )
				{
					this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,30,bIndex);
					return;
				}
			}

			if(strcmp(lpObj->GuildName,gCastleSiegeSync.GetCastleOwnerGuild()) == 0)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,23,bIndex);
				return;
			}

			if(lpObj->Guild->GuildUnion != 0)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,23,bIndex);
				return;
			}

			if(gObjGetRelationShip(lpObj,lpTarget) == GUILD_RELATION_RIVAL)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,22,bIndex);
				return;
			}

			if(lpTarget->Guild->GuildUnion && lpTarget->Guild->Number != lpTarget->Guild->GuildUnion)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,27,bIndex);
				return;
			}
		}
		else if(lpMsg->relation == GUILD_RELATION_RIVAL)
		{
			if(lpObj->Guild->GuildRival != 0 || lpTarget->Guild->GuildRival != 0)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,24,bIndex);
				return;
			}

			if(gObjGetRelationShip(lpObj,lpTarget) == GUILD_RELATION_UNION)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,21,bIndex);
				return;
			}
		}
	}
	else if(lpMsg->type == GUILD_TYPE_BREAK)
	{
		if(lpMsg->relation == GUILD_RELATION_UNION)
		{
			if(lpObj->Guild->GuildUnion == 0)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,25,bIndex);
				return;
			}
		}
		else if(lpMsg->relation == GUILD_RELATION_RIVAL)
		{
			if(lpObj->Guild->GuildRival == 0 || lpTarget->Guild->GuildRival == 0)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,25,bIndex);
				return;
			}

			if(lpObj->Guild->GuildRival != lpTarget->Guild->Number || lpTarget->Guild->GuildRival != lpObj->Guild->Number)
			{
				this->GCGuildUnionResultSend(aIndex,lpMsg->relation,lpMsg->type,28,bIndex);
				return;
			}
		}
	}		
	
	if(lpMsg->type == GUILD_TYPE_BREAK && lpMsg->relation == GUILD_RELATION_UNION)
	{
		SDHP_GUILD_UNION_BREAK_SEND pMsg;

		pMsg.header.set(0xA0, 0x11, sizeof(pMsg));

		pMsg.index[0] = aIndex;

		pMsg.index[1] = -1;

		pMsg.relation = lpMsg->relation;

		pMsg.GuildNumber[0] = lpObj->Guild->Number;

		pMsg.GuildNumber[1] = lpObj->Guild->GuildUnion;

		gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.header.size, aIndex);

		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_GUILD_REQUEST;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = bIndex;

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_GUILD_REQUEST;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = aIndex;

	this->GCGuildUnionRequestSend(bIndex,lpMsg->relation,lpMsg->type,aIndex);
}

void CGuild::CGGuildUnionResultRecv(PMSG_GUILD_UNION_RESULT_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpMsg->result == 1)
	{
		if(lpMsg->type == GUILD_TYPE_JOIN)
		{
			SDHP_GUILD_UNION_JOIN_SEND pMsg;

			pMsg.header.set(0xA0,0x10,sizeof(pMsg));

			pMsg.index[0] = bIndex;

			pMsg.index[1] = aIndex;

			pMsg.relation = lpMsg->relation;

			pMsg.GuildNumber[0] = lpTarget->Guild->Number;

			pMsg.GuildNumber[1] = lpObj->Guild->Number;

			gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
		}
		else if(lpMsg->type == GUILD_TYPE_BREAK)
		{
			SDHP_GUILD_UNION_BREAK_SEND pMsg;

			pMsg.header.set(0xA0, 0x11, sizeof(pMsg));

			pMsg.index[0] = aIndex;

			pMsg.index[1] = bIndex;

			pMsg.relation = lpMsg->relation;

			pMsg.GuildNumber[0] = lpObj->Guild->Number;

			pMsg.GuildNumber[1] = lpTarget->Guild->Number;

			gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.header.size, aIndex);
		}
	}
	else
	{
		this->GCGuildUnionResultSend(bIndex,lpMsg->relation,lpMsg->type,32,bIndex);
	}

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = 0;
	lpObj->TargetNumber = -1;

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = 0;
	lpTarget->TargetNumber = -1;
}

void CGuild::CGGuildUnionListRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	if(lpObj->Guild == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	if(lpObj->Guild->GuildUnion == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	SDHP_GUILD_UNION_LIST_SEND pMsg;

	pMsg.header.set(0xA0,0x12, sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.GuildNumber = lpObj->Guild->GuildUnion;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::CGGuildAllianceKickRecv(PMSG_GUILD_ALLIANCE_KICK_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	if(lpObj->Guild == 0)
	{
		this->GCGuildResultSend(aIndex,3);
		return;
	}

	if(lpObj->Guild->GuildUnion == 0)
	{
		this->GCGuildUnionResultSend(aIndex,GUILD_RELATION_UNION,GUILD_TYPE_BREAK,25,0);
		return;
	}

	if(lpObj->GuildStatus != GUILD_STATUS_MASTER || lpObj->Guild->Number != lpObj->Guild->GuildUnion)
	{
		this->GCGuildUnionResultSend(aIndex,GUILD_RELATION_UNION,GUILD_TYPE_BREAK,17,0);
		return;
	}

	char GuildName[9] = {0};

	memcpy(GuildName,lpMsg->GuildName,sizeof(lpMsg->GuildName));

	if(strcmp(lpObj->Guild->Name,GuildName) == 0)
	{
		this->GCGuildUnionResultSend(aIndex,GUILD_RELATION_UNION,GUILD_TYPE_BREAK,17,0);
		return;
	}

	SDHP_GUILD_ALLIANCE_KICK_SEND pMsg;

	pMsg.header.set(0xA0,0x13,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.relation = GUILD_RELATION_UNION;

	memcpy(pMsg.GuildNameA,lpObj->Guild->Name,sizeof(pMsg.GuildNameA));

	memcpy(pMsg.GuildNameB,GuildName,sizeof(pMsg.GuildNameB));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size,aIndex);
}

void CGuild::GCGuildResultSend(int aIndex,int result) // OK
{
	PMSG_GUILD_RESULT_SEND pMsg;

	pMsg.header.set(0x51,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildDeleteSend(int aIndex,int result) // OK
{
	PMSG_GUILD_DELETE_SEND pMsg;

	pMsg.header.set(0x53,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildCreateResultSend(int aIndex,int result,int type) // OK
{
	PMSG_GUILD_CREATE_RESULT_SEND pMsg;

	pMsg.header.set(0x56,sizeof(pMsg));

	pMsg.result = result;

	pMsg.type = type;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarResultSend(int aIndex,int result) // OK
{
	PMSG_GUILD_WAR_REQUEST_RESULT_SEND pMsg;

	pMsg.header.set(0x60,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarRequestSend(int aIndex,char* GuildName,int type) // OK
{
	PMSG_GUILD_WAR_REQUEST_SEND pMsg;

	pMsg.header.set(0x61,sizeof(pMsg));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	pMsg.type = type;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarDeclareSend(int aIndex,char* GuildName,int type,int team) // OK
{
	PMSG_GUILD_WAR_START_SEND pMsg;

	pMsg.header.set(0x62,sizeof(pMsg));

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	pMsg.type = type;

	pMsg.team = team;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarEndSend(int aIndex,int result,char* GuildName) // OK
{
	PMSG_GUILD_WAR_END_SEND pMsg;

	pMsg.header.set(0x63,sizeof(pMsg));

	pMsg.result = result;

	memcpy(pMsg.GuildName,GuildName,sizeof(pMsg.GuildName));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildWarScoreSend(int aIndex) // OK
{
	if(gObj[aIndex].GuildNumber == 0)
	{
		return;
	}

	PMSG_GUILD_WAR_SCORE_SEND pMsg;

	pMsg.header.set(0x64,sizeof(pMsg));

	pMsg.score[0] = ((gObj[aIndex].Guild==0)?0:gObj[aIndex].Guild->PlayScore);

	pMsg.score[1] = ((gObj[aIndex].Guild->TargetGuildNode==0)?0:gObj[aIndex].Guild->TargetGuildNode->PlayScore);

	pMsg.type = GUILD_WAR_TYPE_NORMAL;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildAssignStatusSend(int aIndex,int type,int result,char* name) // OK
{
	PMSG_GUILD_ASSIGN_STATUS_SEND pMsg;

	pMsg.header.set(0xE1,sizeof(pMsg));

	pMsg.type = type;

	pMsg.result = result;

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildAssignTypeSend(int aIndex,int type,int result) // OK
{
	PMSG_GUILD_ASSIGN_TYPE_SEND pMsg;

	pMsg.header.set(0xE2,sizeof(pMsg));

	pMsg.type = type;

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildUnionRequestSend(int aIndex,int relation,int type,int bIndex) // OK
{
	PMSG_GUILD_UNION_REQUEST_SEND pMsg;

	pMsg.header.set(0xE5,sizeof(pMsg));

	pMsg.relation = relation;

	pMsg.type = type;

	pMsg.index[0] = SET_NUMBERHB(bIndex);

	pMsg.index[1] = SET_NUMBERLB(bIndex);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GCGuildUnionResultSend(int aIndex,int relation,int type,int result,int bIndex) // OK
{
	PMSG_GUILD_UNION_RESULT_SEND pMsg;

	pMsg.header.set(0xE6,sizeof(pMsg));

	pMsg.relation = relation;

	pMsg.type = type;

	pMsg.result = result;

	pMsg.index[0] = SET_NUMBERHB(bIndex);

	pMsg.index[1] = SET_NUMBERLB(bIndex);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CGuild::GuildWarDeclareSet(int aIndex,char* GuildName,int type,int team) // OK
{
	this->GCGuildWarDeclareSend(aIndex,GuildName,type,team);

	this->GCGuildWarScoreSend(aIndex);

	if(type == GUILD_WAR_TYPE_SOCCER)
	{
		gObj[aIndex].IsInBattleGround = 1;

		if(gServerInfo.m_PKLimitFree == 0 && gObj[aIndex].PKLevel > 4)
		{
			return;
		}

		gObjTeleport(aIndex,6,60,153);
	}
}

void CGuild::GuildWarRequestSend(int aIndex,char* GuildName,int type) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(gMapManager.GetMapGuildWarEnable(lpObj->Map) == 0)
	{
		return;
	}

	if(lpObj->GuildNumber == 0 || lpObj->Guild == 0)
	{
		this->GCGuildWarResultSend(aIndex,3);
		return;
	}

	if(lpObj->Guild->WarState == GUILD_WAR_STATE_DECLARE || lpObj->Guild->WarDeclareState == GUILD_WAR_STATE_DECLARE)
	{
		this->GCGuildWarResultSend(aIndex,4);
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpObj->PKLevel > 4)
	{
		this->GCGuildWarResultSend(aIndex,4);
		return;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name) != 0)
	{
		this->GCGuildWarResultSend(aIndex,5);
		return;
	}

	if(!strncmp(lpObj->Guild->Name,GuildName,8))
	{
		return;
	}

	GUILD_INFO_STRUCT* lpInfo = gGuildClass.SearchGuild(GuildName);

	if(lpInfo == 0)
	{
		this->GCGuildWarResultSend(aIndex,0);
		return;
	}

	if(lpInfo->WarState == GUILD_WAR_STATE_DECLARE || lpInfo->WarDeclareState == GUILD_WAR_STATE_DECLARE)
	{
		this->GCGuildWarResultSend(aIndex,4);
		return;
	}

	if(lpObj->Guild->GuildUnion != 0 && lpObj->Guild->GuildUnion == lpInfo->GuildUnion)
	{
		return;
	}

	LPOBJ lpTarget = gObjFind(lpInfo->Names[0]);

	if(lpTarget == 0)
	{
		this->GCGuildWarResultSend(aIndex,2);
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpTarget->PKLevel > 4)
	{
		this->GCGuildWarResultSend(aIndex,4);
		return;
	}

	if(gMapManager.GetMapGuildWarEnable(lpTarget->Map) == 0)
	{
		return;
	}

	if((lpTarget->Option & 1) != 1)
	{
		this->GCGuildWarResultSend(aIndex,4);
		return;
	}

	this->GCGuildWarResultSend(aIndex,1);

	this->GCGuildWarRequestSend(lpTarget->Index,lpObj->Guild->Name,type);

	lpObj->Guild->WarDeclareState = GUILD_WAR_STATE_DECLARE;
	lpObj->Guild->WarType = type;
	strcpy_s(lpObj->Guild->TargetGuildName,lpInfo->Name);
	lpObj->Guild->TargetGuildNode = lpInfo;

	lpInfo->WarDeclareState = GUILD_WAR_STATE_DECLARE;
	lpInfo->WarType = type;
	strcpy_s(lpInfo->TargetGuildName,lpObj->Guild->Name);
	lpInfo->TargetGuildNode = lpObj->Guild;
}

int CGuild::GetMaxGuildUser(LPOBJ lpObj) // OK
{
	int MaxGuildUser = 0;

	if(lpObj->Class == CLASS_DL)
	{
		MaxGuildUser = ((lpObj->Reset==0)?(((lpObj->Level+lpObj->MasterLevel)/10)+(lpObj->Leadership/10)):gServerInfo.m_GuildInsertMaxUser2);

		MaxGuildUser = ((MaxGuildUser>gServerInfo.m_GuildInsertMaxUser2)?gServerInfo.m_GuildInsertMaxUser2:MaxGuildUser);
	}
	else
	{
		MaxGuildUser = ((lpObj->Reset==0)?((lpObj->Level+lpObj->MasterLevel)/10):gServerInfo.m_GuildInsertMaxUser1);

		MaxGuildUser = ((MaxGuildUser>gServerInfo.m_GuildInsertMaxUser1)?gServerInfo.m_GuildInsertMaxUser1:MaxGuildUser);
	}

	return ((MaxGuildUser>MAX_GUILD_USER)?MAX_GUILD_USER:MaxGuildUser);
}

int CGuild::GetGuildNameCount(LPOBJ lpObj,char* name) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && strcmp(lpObj->Guild->Names[n],name) == 0)
		{
			count++;
		}
	}

	return count;
}

int CGuild::GetGuildBattleMasterCount(LPOBJ lpObj) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && lpObj->Guild->GuildStatus[n] == GUILD_STATUS_BATTLE_MASTER)
		{
			count++;
		}
	}

	return count;
}

int CGuild::GetGuildOfficeMasterCount(LPOBJ lpObj) // OK
{
	int count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] != 0 && lpObj->Guild->GuildStatus[n] == GUILD_STATUS_OFFICE_MASTER)
		{
			count++;
		}
	}

	return count;
}

int CGuild::gObjTargetGuildWarCheck(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER)
	{
		return 0;
	}

	if(lpObj->GuildNumber < 1 || lpTarget->GuildNumber < 1)
	{
		return 0;
	}

	if(lpObj->Guild->WarState != GUILD_WAR_STATE_DECLARE || lpTarget->Guild->WarState != GUILD_WAR_STATE_DECLARE)
	{
		return 0;
	}

	if(strcmp(lpObj->Guild->TargetGuildName,lpTarget->Guild->Name))
	{
		return 0;
	}

	return 1;
}

void CGuild::gObjGuildWarEndSend(LPOBJ lpObj,int result1,int result2) // OK
{
	if(lpObj == 0)
	{
		return;
	}

	if(lpObj->Guild == 0)
	{
		return;
	}

	if(lpObj->Guild->WarState != GUILD_WAR_STATE_DECLARE)
	{
		return;
	}

	if(lpObj->Guild->TargetGuildNode == 0)
	{
		return;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->Use[n] && lpObj->Guild->Index[n] >= 0)
		{
			gObj[lpObj->Guild->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpObj->Guild->Index[n],result1,lpObj->Guild->TargetGuildNode->Name);
		}
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpObj->Guild->TargetGuildNode->Use[n] && lpObj->Guild->TargetGuildNode->Index[n] >= 0)
		{
			gObj[lpObj->Guild->TargetGuildNode->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpObj->Guild->TargetGuildNode->Index[n],result2,lpObj->Guild->Name);
		}
	}
}

void CGuild::gObjGuildWarEndSend(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2,int result1,int result2) // OK
{
	if(lpGuild1 == 0 || lpGuild2 == 0)
	{
		return;
	}

	if(lpGuild1->WarState != GUILD_WAR_STATE_DECLARE || lpGuild2->WarState != GUILD_WAR_STATE_DECLARE)
	{
		return;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild1->Use[n] && lpGuild1->Index[n] >= 0)
		{
			gObj[lpGuild1->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpGuild1->Index[n],result1,lpGuild2->Name);
		}		
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild2->Use[n] && lpGuild2->Index[n] >= 0)
		{
			gObj[lpGuild2->Index[n]].IsInBattleGround = 0;
			gGuild.GCGuildWarEndSend(lpGuild2->Index[n],result2,lpGuild1->Name);
		}
	}
}

void CGuild::gObjGuildWarEnd(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2) // OK
{
	if(lpGuild1 == 0 || lpGuild2 == 0)
	{
		return;
	}

	lpGuild1->WarDeclareState = GUILD_WAR_STATE_NONE;
	lpGuild1->WarState = GUILD_WAR_STATE_NONE;

	if(lpGuild2 != 0)
	{
		lpGuild2->WarDeclareState = GUILD_WAR_STATE_NONE;
		lpGuild2->WarState = GUILD_WAR_STATE_NONE;

		if(lpGuild1->TargetGuildNode != 0)
		{
			GUILD_INFO_STRUCT * lpTguild = lpGuild1->TargetGuildNode;

			lpGuild1->TargetGuildNode = 0;
			lpTguild->TargetGuildNode = 0;
		}
	}
	else
	{
		lpGuild1->TargetGuildNode = 0;
	}
}

int CGuild::gObjGuildWarProc(GUILD_INFO_STRUCT* lpGuild1,GUILD_INFO_STRUCT* lpGuild2,int score) // OK
{
	if(lpGuild1 == 0 || lpGuild2 == 0)
	{
		return 0;
	}

	int MaxScore = 20;

	lpGuild1->PlayScore += score;

	if(lpGuild1->WarType == GUILD_WAR_TYPE_SOCCER)
	{
		SetBattleTeamScore(lpGuild1->BattleGroundIndex,lpGuild1->BattleTeamCode,lpGuild1->PlayScore);
		MaxScore = 100;
	}

	if(lpGuild1->PlayScore >= MaxScore)
	{
		if(lpGuild1->PlayScore > MaxScore && lpGuild2->PlayScore == 0)
		{
			lpGuild1->TotalScore += 3;
		}
		else if(lpGuild1->PlayScore > MaxScore && lpGuild2->PlayScore <= 10)
		{
			lpGuild1->TotalScore += 2;
		}
		else
		{
			lpGuild1->TotalScore += 1;
		}

		gGuild.GDGuildScoreUpdateSend(lpGuild1->Name,lpGuild1->TotalScore);

		gGuild.GDGuildScoreUpdateSend(lpGuild2->Name,lpGuild2->TotalScore);

		return 1;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild1->Use[n] && lpGuild1->Index[n] >= 0)
		{
			gGuild.GCGuildWarScoreSend(lpGuild1->Index[n]);
		}
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild2->Use[n] && lpGuild2->Index[n] >= 0)
		{
			gGuild.GCGuildWarScoreSend(lpGuild2->Index[n]);
		}
	}

	return 0;
}

int CGuild::gObjGuildWarCheck(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(gObjTargetGuildWarCheck(lpObj,lpTarget) == 0)
	{
		return 0;
	}

	int Score = (strcmp(lpTarget->Name,lpTarget->Guild->Names[0]) == 0) ? 2 : 1;

	GUILD_INFO_STRUCT*lpGuild = lpObj->Guild;
	GUILD_INFO_STRUCT*lpTargetGuild = lpTarget->Guild;

	if(gObjGuildWarProc(lpGuild,lpTargetGuild,Score) == 1)
	{
		if(lpGuild->WarType == GUILD_WAR_TYPE_SOCCER)
		{
			gObjGuildWarEndSend(lpGuild,lpTargetGuild,1,0);
			gBattleGroundEnable(lpGuild->BattleGroundIndex,0);
			gObjGuildWarEnd(lpGuild,lpTargetGuild);
		}
		else
		{
			gObjGuildWarEndSend(lpObj,1,0);
			gObjAddMsgSendDelay(lpObj,4,lpObj->Index,2000,0);
		}
	}

	return 1;
}

int CGuild::gObjGuildWarMasterClose(LPOBJ lpObj) // OK
{
	if(lpObj->GuildNumber < 1)
	{
		return 0;
	}

	if(strcmp(lpObj->Guild->Names[0],lpObj->Name))
	{
		return 0;
	}

	gObjGuildWarEndSend(lpObj,3,2);

	if(lpObj->Guild->WarType == GUILD_WAR_TYPE_SOCCER && lpObj->Guild->BattleGroundIndex < 1)
	{
		gBattleGroundEnable(lpObj->Guild->BattleGroundIndex,0);
	}

	gObjGuildWarEnd(lpObj->Guild,lpObj->Guild->TargetGuildNode);

	return 1;
}

int CGuild::gObjGetGuildUnionNumber(LPOBJ lpObj) // OK
{
	if(lpObj->Guild == 0)
	{
		return 0;
	}

	return (lpObj->Guild->GuildUnion == GUILD_RELATION_NONE) ? lpObj->Guild->Number : lpObj->Guild->GuildUnion;
}

void CGuild::gObjGetGuildUnionName(LPOBJ lpObj,char* UnionName,int size) // OK
{
	memset(UnionName,0,sizeof(UnionName));

	if(lpObj->Guild == 0 || lpObj->Guild->GuildUnion == 0)
	{
		return;
	}

	CUnionInfo* lpUnion = gUnionManager.SearchUnion(lpObj->Guild->GuildUnion);

	if(lpUnion != 0)
	{
		memcpy(UnionName,lpUnion->m_szMasterGuild,size);
	}
}

bool CGuild::gObjCheckRival(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER)
	{
		return 0;
	}

	if(lpObj->Guild == 0 || lpTarget->Guild == 0)
	{
		return 0;
	}

	if(lpObj->Guild->GuildRival == GUILD_RELATION_NONE || lpTarget->Guild->GuildRival == GUILD_RELATION_NONE)
	{
		return 0;
	}

	if(gUnionManager.GetGuildRelationShip((lpObj->Guild->GuildUnion==GUILD_RELATION_NONE) ? lpObj->Guild->Number : lpObj->Guild->GuildUnion,lpTarget->Guild->Number) == 2)
	{
		return 1;
	}

	return 0;
}

int CGuild::gObjGetRelationShip(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj == 0 || lpTarget == 0)
	{
		return 0;
	}

	if(lpObj->Guild == 0 || lpTarget->Guild == 0)
	{
		return 0;
	}

	return gUnionManager.GetGuildRelationShip((lpObj->Guild->GuildUnion==GUILD_RELATION_NONE) ? lpObj->Guild->Number : lpObj->Guild->GuildUnion,lpTarget->Guild->Number);
	

	//int iUnion = (lpObj->Guild->GuildUnion==GUILD_RELATION_NONE)?lpObj->Guild->Number:lpObj->Guild->GuildUnion;
	
	//int iUnion2 = (lpTarget->Guild->GuildUnion == GUILD_RELATION_NONE) ? lpTarget->Guild->Number : lpTarget->Guild->GuildUnion;
	
	//return gUnionManager.GetGuildRelationShip(iUnion,iUnion2);
}

void CGuild::NoticeSendToAll(GUILD_INFO_STRUCT* lpGuild,int type,char* message,...) // OK
{
	if(lpGuild == 0)
	{
		return;
	}

	if(lpGuild->Count == 0)
	{
		return;
	}

	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] > 0 && lpGuild->Index[n] >= 0)
		{
			if(gObjIsNameValid(lpGuild->Index[n],lpGuild->Names[n]) != 0)
			{
				gNotice.GCNoticeSend(lpGuild->Index[n],type,0,0,0,0,0,buff);
			}
		}
	}
}

void CGuild::DataSendToAll(GUILD_INFO_STRUCT* lpGuild,BYTE* lpMsg,int size) // OK
{
	if(lpGuild == 0)
	{
		return;
	}

	if(lpGuild->Count == 0)
	{
		return;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] > 0 && lpGuild->Index[n] >= 0)
		{
			if(gObjIsNameValid(lpGuild->Index[n],lpGuild->Names[n]) != 0)
			{
				DataSend(lpGuild->Index[n],lpMsg,size);
			}
		}
	}
}