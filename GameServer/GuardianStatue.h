// GuardianStatue.h: interface for the CGuardianStatue class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CGuardianStatue
{
public:
	CGuardianStatue();
	virtual ~CGuardianStatue();
	void GuardianStatueAct(int aIndex);
};

extern CGuardianStatue gGuardianStatue;