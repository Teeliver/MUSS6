// GuildClass.cpp: implementation of the CGuildClass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuildClass.h"
#include "Guild.h"
#include "Viewport.h"
#include "Util.h"

CGuildClass gGuildClass;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGuildClass::CGuildClass() // OK
{
	this->head = 0;
	this->tail = 0;
}

CGuildClass::~CGuildClass() // OK
{
	this->AllDelete();
}

void CGuildClass::Init() // OK
{
	this->m_GuildMap.clear();
	this->m_GuildNumberMap.clear();
}

GUILD_INFO_STRUCT* CGuildClass::AddGuild(int GuildNumber,char* GuildName,BYTE* Mark,char* GuildMaster,int score) // OK
{
	GUILD_INFO_STRUCT* pSearchGuild = this->SearchGuild(GuildName);

	if(pSearchGuild != 0)
	{
		return pSearchGuild;
	}

	GUILD_INFO_STRUCT* pNewNode = (GUILD_INFO_STRUCT*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(GUILD_INFO_STRUCT));

	if(pNewNode != 0)
	{
		memset(pNewNode->Name,0,sizeof(pNewNode->Name));
		strcpy_s(pNewNode->Name,GuildName);
		strcpy_s(pNewNode->Names[0], GuildMaster);

		if(Mark != 0)
		{
			memcpy(pNewNode->Mark,Mark,sizeof(pNewNode->Mark));
		}

		pNewNode->Number = GuildNumber;
		pNewNode->Count = 0;
		pNewNode->TotalCount = 0;
		pNewNode->PlayScore = 0;
		pNewNode->TotalScore = score;
		pNewNode->next = 0;
		pNewNode->WarDeclareState = GUILD_WAR_STATE_NONE;
		pNewNode->WarState = GUILD_WAR_STATE_NONE;
		memset(pNewNode->Notice,0,sizeof(pNewNode->Notice));
		pNewNode->GuildUnion = GUILD_RELATION_NONE;
		pNewNode->GuildRival = GUILD_RELATION_NONE;

		for(int i=0;i < MAX_GUILD_USER;i++)
		{
			pNewNode->Use[i] = 0;
			pNewNode->Index[i] = -1;
			pNewNode->Server[i] = -1;
			pNewNode->GuildStatus[i] = -1;
		}

		pNewNode->LifeStone = 0;

		this->BuildMemberTotal(pNewNode);
		this->AddTail(pNewNode);

		return pNewNode;
	}

	return 0;
}

GUILD_INFO_STRUCT*CGuildClass::AddMember(GUILD_INFO_STRUCT* lpGuild,char* Name,int aIndex,int score,int server) // OK
{
	int blank = -1;

	if(lpGuild == 0)
	{
		return 0;
	}

	if(strcmp(Name,lpGuild->Names[0]) == 0)
	{
		blank = 0;
	}
	else
	{
		for(int i=1;i < MAX_GUILD_USER;i++)
		{
			if(lpGuild->Use[i] != 0)
			{
				if(strcmp(lpGuild->Names[i],Name) == 0)
				{
					if(aIndex != -1)
					{
						lpGuild->Use[i] = 1;
						lpGuild->Index[i] = aIndex;
						lpGuild->Server[i] = server;
					}

					this->BuildMemberTotal(lpGuild);

					return 0;
				}
			}
			else if(blank < 0)
			{
				blank = i;
			}
		}
	}

	if(blank < 0)
	{
		return 0;
	}

	strcpy_s(lpGuild->Names[blank],Name);
	lpGuild->Use[blank] = 0;
	lpGuild->Server[blank] = server;

	if(aIndex != -1)
	{
		lpGuild->Index[blank] = aIndex;
		lpGuild->Count++;
	}

	if(score > 0)
	{
		lpGuild->TotalCount = score;
	}

	this->BuildMemberTotal(lpGuild);

	return lpGuild;
}

GUILD_INFO_STRUCT* CGuildClass::AddMember(char* GuildName,char* Name,int aIndex,int count,int status,int server) // OK
{
	GUILD_INFO_STRUCT* pNode = this->SearchGuild(GuildName);

	int blank = -1;

	if(pNode == 0)
	{
		return 0;
	}

	this->BuildMemberTotal(pNode);

	if(strcmp(Name,pNode->Names[0] ) == 0)
	{
		blank = 0;
	}
	else
	{
		for(int i=1;i < MAX_GUILD_USER;i++)
		{
			if(pNode->Use[i] != 0)
			{
				if(strcmp(pNode->Names[i],Name) == 0)
				{
					if( aIndex != -1 )
					{
						pNode->Use[i] = 1;
						pNode->Index[i] = aIndex;
						pNode->Server[i] = server;
						pNode->GuildStatus[i] = GUILD_STATUS_MASTER;;
					}

					this->BuildMemberTotal(pNode);

					return 0;
				}
			}
			else if(blank < 0)
			{
				blank = i;
			}
		}
	}

	if(blank < 0)
	{
		return 0;
	}

	strcpy_s(pNode->Names[blank],Name);
	pNode->Use[blank] = 1;
	pNode->Server[blank] = server;
	pNode->GuildStatus[blank] = status;

	if(aIndex != -1)
	{
		pNode->Index[blank] = aIndex;
		pNode->Count++;
	}

	if(count > 0)
	{
		pNode->TotalCount = count;
	}

	this->BuildMemberTotal(pNode);

	return pNode;
}

void CGuildClass::AddTail(GUILD_INFO_STRUCT* lpGuild) // OK
{
	lpGuild->back = this->tail;
	
	lpGuild->next = 0;

	if(this->head == 0)
	{
		this->head = lpGuild;
	}
	else
	{
		this->tail->next = lpGuild;
	}

	this->tail = lpGuild;
	
	this->m_GuildMap[lpGuild->Name] = lpGuild;

	this->m_GuildNumberMap[lpGuild->Number] = lpGuild;
}

void CGuildClass::Print() // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->head;

	while (lpGuild != 0)
	{
		lpGuild = lpGuild->next;
	}
}

void CGuildClass::PrintGuild(char* GuildName) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);
}

void CGuildClass::AllDelete() // OK
{
	GUILD_INFO_STRUCT*lpGuild = this->head;

	while (lpGuild != 0)
	{
		lpGuild = lpGuild->next;

		HeapFree(GetProcessHeap(),0,lpGuild);
	}

	this->head = 0;

	this->tail = 0;

	this->m_GuildMap.clear();

	this->m_GuildNumberMap.clear();
}

int CGuildClass::DeleteGuild(char* GuildName) // OK
{
	GUILD_INFO_STRUCT*pNode = this->SearchGuild(GuildName);

	GUILD_INFO_STRUCT*pprev;

	GUILD_INFO_STRUCT*pnext;

	if(pNode == 0)
	{
		return 0;
	}

	std::map<std::string,GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildMap.find(GuildName);

	if(Itor != this->m_GuildMap.end())
	{
		this->m_GuildMap.erase(Itor);
	}

	std::map<int ,GUILD_INFO_STRUCT *>::iterator _Itor = this->m_GuildNumberMap.find(pNode->Number);

	if(_Itor != this->m_GuildNumberMap.end())
	{
		this->m_GuildNumberMap.erase(_Itor);
	}

	pprev = pNode->back;
	pnext = pNode->next;

	if(pprev == 0 && pnext != 0)
	{
		pnext->back = 0;
		this->head = pnext;
	}
	else if(pprev != 0 && pnext == 0)
	{
		pprev->next = 0;
		this->tail = pprev;
	}
	else if(pprev != 0 && pnext != 0)
	{
		pprev->next = pnext;
		pnext->back = pprev;
	}

	HeapFree(GetProcessHeap(),0,pNode);

	if(pprev == 0 && pnext == 0)
	{
		this->head=0;
		this->tail = 0;
	}

	return 1;
}

GUILD_INFO_STRUCT* CGuildClass::SearchGuild(char* GuildName) // OK
{
	std::map<std::string,GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildMap.find(GuildName);

	if(Itor == this->m_GuildMap.end())
	{
		return 0;
	}

	return (*(Itor)).second;
}

GUILD_INFO_STRUCT* CGuildClass::SearchGuild_Number(int GuildNumber) // OK
{
	std::map<int ,GUILD_INFO_STRUCT *>::iterator Itor = this->m_GuildNumberMap.find(GuildNumber);

	if(Itor == this->m_GuildNumberMap.end())
	{
		return 0;
	}

	return (*(Itor)).second;
}

GUILD_INFO_STRUCT* CGuildClass::SearchGuild_NumberAndId(int GuildNumber,char* Name) // OK
{
	GUILD_INFO_STRUCT*pNode = this->SearchGuild_Number(GuildNumber);

	if( pNode != NULL )
	{
		for( int i=0;i<MAX_GUILD_USER;i++)
		{
			if( strcmp(pNode->Names[i],Name) == 0 )
			{
				return pNode;
			}
		}
	}

	return NULL;
}

int CGuildClass::ConnectUser(GUILD_INFO_STRUCT* lpGuild,char* GuildName,char* Name,int aIndex,int server) // OK
{
	if(lpGuild == 0)
	{
		return 0;
	}

	if(strcmp(GuildName,lpGuild->Name) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0)
			{
				lpGuild->Index[n] = aIndex;
				lpGuild->Server[n] = server;

				if(n == 0)
				{
					lpGuild->PlayScore = 0;
					lpGuild->WarDeclareState = GUILD_WAR_STATE_NONE;
					lpGuild->WarState = GUILD_WAR_STATE_NONE;
				}
				
				return 1;
			}
		}
	}
	
	this->AddMember(lpGuild,Name,aIndex,-1,server);

	return 0;
}

int CGuildClass::GetGuildMemberStatus(char* GuildName,char* Name) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return -1;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0 )
			{
				return lpGuild->GuildStatus[n];
			}
		}
	}

	return -1;
}

int CGuildClass::SetGuildMemberStatus(char* GuildName,char* Name,int status) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return -1;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0)
			{
				lpGuild->GuildStatus[n] = status;

				if(lpGuild->Index[n] > 0)
				{
					if(gObjIsConnectedGP(lpGuild->Index[n]) == 0)
					{
						return 0;
					}
		
					LPOBJ lpObj = &gObj[lpGuild->Index[n]];

					if(strcmp(lpObj->Name,Name) == 0)
					{
						lpObj->GuildStatus = status;

						gViewport.GCViewportSimpleGuildSend(lpObj);
					}

				}

				return 1;
			}
		}
	}

	return 0;
}

int CGuildClass::GetGuildType(char* GuildName) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return -1;
	}

	return lpGuild->GuildType;
}

int CGuildClass::SetGuildType(char* GuildName,int type) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return 0;
	}

	lpGuild->GuildType = type;

	for( int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(lpGuild->Index[n] > 0)
			{
				if(gObjIsConnectedGP(lpGuild->Index[n]) == 0)
				{
					continue;
				}

				LPOBJ lpObj = &gObj[lpGuild->Index[n]];

				gViewport.GCViewportSimpleGuildSend(lpObj);
			}
		}
	}

	return 1;
}

int CGuildClass::BuildMemberTotal(GUILD_INFO_STRUCT* lpGuild) // OK
{
	if(lpGuild == 0)
	{
		return 0;
	}

	lpGuild->TotalCount = 0;

	lpGuild->Count = 0;

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			lpGuild->TotalCount++;

			if(lpGuild->Index[n] != 0)
			{
				lpGuild->Count++;
			}
		}
	}

	return 1;
}

int CGuildClass::DelMember(char* GuildName,char* Name) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0)
			{
				lpGuild->Use[n] = 0;

				lpGuild->Count--;

				this->BuildMemberTotal(lpGuild);

				return 1;
			}
		}
	}

	return 0;
}

int CGuildClass::CloseMember(char* GuildName,char* Name) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0)
			{
				lpGuild->Index[n] = -1;
				lpGuild->Count--;
				lpGuild->Server[n] = -1;

				this->BuildMemberTotal(lpGuild);

				return 1;
			}
		}
	}

	return 0;
}

int CGuildClass::SetServer(char* GuildName,char* Name,int server) // OK
{
	GUILD_INFO_STRUCT* lpGuild = this->SearchGuild(GuildName);

	if(lpGuild == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GUILD_USER;n++)
	{
		if(lpGuild->Use[n] != 0)
		{
			if(strcmp(lpGuild->Names[n],Name) == 0)
			{
				lpGuild->Server[n] = server;

				return 1;
			}
		}
	}

	return 0;
}
