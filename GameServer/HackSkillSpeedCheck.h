// HackSkillSpeedCheck.h: interface for the CHackSkillSpeedCheck class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct HACK_SKILL_SPEED_CHECK_INFO
{
	int Index;
	int Class;
	int MinLevel;
	int MaxLevel;
	int MinPhysiSpeed;
	int MaxPhysiSpeed;
	int MinMagicSpeed;
	int MaxMagicSpeed;
	int MaxDelay;
	int MaxCount;
};

class CHackSkillSpeedCheck
{
public:
	CHackSkillSpeedCheck();
	virtual ~CHackSkillSpeedCheck();
	void Load(char* path);
	bool CheckSkillSpeed(int aIndex,int skill);
private:
	std::vector<HACK_SKILL_SPEED_CHECK_INFO> m_HackSkillSpeedCheckInfo;
};

extern CHackSkillSpeedCheck gHackSkillSpeedCheck;