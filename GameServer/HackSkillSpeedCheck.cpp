// HackSkillSpeedCheck.cpp: implementation of the CHackSkillSpeedCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HackSkillSpeedCheck.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Log.h"
#include "Util.h"

CHackSkillSpeedCheck gHackSkillSpeedCheck;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHackSkillSpeedCheck::CHackSkillSpeedCheck() // OK
{
	this->m_HackSkillSpeedCheckInfo.clear();
}

CHackSkillSpeedCheck::~CHackSkillSpeedCheck() // OK
{

}

void CHackSkillSpeedCheck::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_HackSkillSpeedCheckInfo.clear();

	try
	{
		while (true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			HACK_SKILL_SPEED_CHECK_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Class = lpMemScript->GetAsNumber();

			info.MinLevel = lpMemScript->GetAsNumber();

			info.MaxLevel = lpMemScript->GetAsNumber();

			info.MinPhysiSpeed = lpMemScript->GetAsNumber();

			info.MaxPhysiSpeed = lpMemScript->GetAsNumber();
			
			info.MinMagicSpeed = lpMemScript->GetAsNumber();

			info.MaxMagicSpeed = lpMemScript->GetAsNumber();
			
			info.MaxDelay = lpMemScript->GetAsNumber();
			
			info.MaxCount = lpMemScript->GetAsNumber();

			this->m_HackSkillSpeedCheckInfo.push_back(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CHackSkillSpeedCheck::CheckSkillSpeed(int aIndex,int skill) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if((GetTickCount()-lpObj->HackSkillPenalty) >= (DWORD)gServerInfo.m_CheckSkillSpeedPenalty)
	{
		lpObj->HackSkillPenalty = 0;
	}

	if(lpObj->HackSkillPenalty != 0)
	{
		return 0;
	}

	if(strcmp(gServerInfo.m_ServerDebuggerName,lpObj->Name) == 0 && (gServerInfo.m_Debugger == 2 || gServerInfo.m_Debugger == -1))
	{
		Console(LOG_GREEN,"Skill(%d) PhysiSpeed(%d) MagicSpeed(%d) Delay(%d) Count(%d)",skill,lpObj->PhysiSpeed,lpObj->MagicSpeed,(GetTickCount()-lpObj->HackSkillDelay[skill]),lpObj->HackSkillCount[skill]);
	}

	for(std::vector<HACK_SKILL_SPEED_CHECK_INFO>::iterator it=this->m_HackSkillSpeedCheckInfo.begin();it != this->m_HackSkillSpeedCheckInfo.end();it++)
	{
		if(it->Index != skill)
		{
			continue;
		}

		if(it->Class != -1 && it->Class != lpObj->Class)
		{
			continue;
		}

		if(it->MinLevel != -1 && it->MinLevel > lpObj->Level)
		{
			continue;
		}

		if(it->MaxLevel != -1 && it->MaxLevel < lpObj->Level)
		{
			continue;
		}

		if(it->MinPhysiSpeed != -1 && it->MinPhysiSpeed > lpObj->PhysiSpeed)
		{
			continue;
		}

		if(it->MaxPhysiSpeed != -1 && it->MaxPhysiSpeed < lpObj->PhysiSpeed)
		{
			continue;
		}

		if(it->MinMagicSpeed != -1 && it->MinMagicSpeed > lpObj->MagicSpeed)
		{
			continue;
		}

		if(it->MaxMagicSpeed != -1 && it->MaxMagicSpeed < lpObj->MagicSpeed)
		{
			continue;
		}

		if((GetTickCount()-lpObj->HackSkillDelay[skill]) >= (DWORD)it->MaxDelay)
		{
			lpObj->HackSkillDelay[skill] = GetTickCount();
			lpObj->HackSkillCount[skill] = 0;
		}

		if((++lpObj->HackSkillCount[skill]) > it->MaxCount)
		{
			lpObj->HackSkillPenalty = GetTickCount();

			Console(LOG_RED,"[%s][%s] Skill [%d] Count [%d] MaxCount [%d]",lpObj->Account,lpObj->Name,skill,lpObj->HackSkillCount[skill],it->MaxCount);

			gLog.Output(LOG_HACK,"[HackSkillSpeedCheck][%s][%s] Skill count error (Index: %d,Speed: [%d]S[%d]M[%d])",lpObj->Account,lpObj->Name,skill,lpObj->HackSkillCount[skill],it->MaxPhysiSpeed,it->MaxMagicSpeed);

			if(gServerInfo.m_CheckSkillSpeedAction == 1)
			{
				GCNewMessageSend(aIndex,0,0,gMessage.GetMessage(719));
			}
			else if(gServerInfo.m_CheckSkillSpeedAction == 2)
			{
				gObjUserKill(aIndex);
			}

			return 0;
		}

	}

	return 1;
}