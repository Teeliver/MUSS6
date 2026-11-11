// CustomAttack.h: interface for the CCustomAttack class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "Map.h"
#include "User.h"

struct CUSTOM_ATTACK_SKILL_INFO
{
	int Class;
	int Skill;
	int Group;
	int Value;
	int BaseSpeed;
	int MainSpeed;
};

class CCustomAttack
{
public:
	CCustomAttack();
	virtual ~CCustomAttack();
	void MainProc(LPOBJ lpObj);
	void SecondProc(LPOBJ lpObj);
	void AttackOfflineClose(LPOBJ lpObj);
	void ReadCustomAttackInfo(char* section,char* path);
	void Load(char* path);
	void SetInfo(CUSTOM_ATTACK_SKILL_INFO info);
	CUSTOM_ATTACK_SKILL_INFO* GetInfo(int Class,int SkillNumber);
	int GetSkilLAuto(LPOBJ lpObj);
	void CommandCustomAttack(LPOBJ lpObj,char* arg);
	void CommandCustomAttackOffline(LPOBJ lpObj);
	void CustomAttackAutoPotion(LPOBJ lpObj);
	void CustomAttackAutoRepair(LPOBJ lpObj);
	void CustomAttackAutoReload(LPOBJ lpObj);
	void CustomAttackAutoBuff(LPOBJ lpObj);
	void CustomAttackAutoAttack(LPOBJ lpObj);
	int CustomAttackFindTarget(LPOBJ lpObj,int SkillNumber);
	CSkill* CustomAttackFindSkill(LPOBJ lpObj,int SkillNumber);
	void CustomAttackUseSkill(LPOBJ lpObj,LPOBJ lpTarget,CUSTOM_ATTACK_SKILL_INFO* lpInfo,CSkill* lpSkill);
	void CustomAttackSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void CustomAttackMultilAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void CustomAttackDurationlAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
	void CustomAttackRageAttack(LPOBJ lpObj,int aIndex,int SkillNumber);
public:
	std::map<int,CUSTOM_ATTACK_SKILL_INFO> m_CustomAttackSkillInfo[MAX_CLASS];
	int m_CustomAttackSwitch;
	int m_CustomAttackMapZone;
	int m_CustomAttackMapList[MAX_MAP];
	int m_CustomAttackBuffEnable[MAX_ACCOUNT_LEVEL];
	int m_CustomAttackMaxTimeLimit[MAX_ACCOUNT_LEVEL];
	int m_CustomAttackOfflineSwitch;
	int m_CustomAttackOfflineCoinGain[MAX_ACCOUNT_LEVEL];
	int m_CustomAttackOfflineBuffEnable[MAX_ACCOUNT_LEVEL];
	int m_CustomAttackOfflineKeepEnable[MAX_ACCOUNT_LEVEL];
	int m_CustomAttackOfflineMaxTimeLimit[MAX_ACCOUNT_LEVEL];
};

extern CCustomAttack gCustomAttack;
