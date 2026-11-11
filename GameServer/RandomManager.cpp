// RandomManager.cpp: implementation of the CRandomManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RandomManager.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRandomManager::CRandomManager() // OK
{
	this->Clear();
}

CRandomManager::~CRandomManager() // OK
{

}

void CRandomManager::Clear() // OK
{
	this->m_MaxRate = 0;

	this->m_RandomManagerInfo.clear();
}

bool CRandomManager::GetRandomElement(std::intptr_t* value) // OK
{
	if(this->m_MaxRate == 0)
	{
		return 0;
	}

	if(this->m_RandomManagerInfo.empty() != 0)
	{
		return 0;
	}

	int RandomNumber = GetLargeRand()%this->m_MaxRate;

	for(std::vector<RANDOM_MANAGER_INFO>::iterator it=this->m_RandomManagerInfo.begin();it != this->m_RandomManagerInfo.end();it++)
	{
		if(RandomNumber >= it->MinRate && RandomNumber < it->MaxRate)
		{
			(*value) = it->value;
			return 1;
		}
	}

	return 0;
}

#if INTPTR_MAX > INT_MAX
bool CRandomManager::GetRandomElement(int* value)
{
	std::intptr_t selected;
	if(this->GetRandomElement(&selected) == 0)
	{
		return 0;
	}
	(*value) = static_cast<int>(selected);
	return 1;
}
#endif

bool CRandomManager::GetRandomElement(WORD* value) // OK
{
	if(this->m_MaxRate == 0)
	{
		return 0;
	}

	if(this->m_RandomManagerInfo.empty() != 0)
	{
		return 0;
	}

	int RandomNumber = GetLargeRand()%this->m_MaxRate;

	for(std::vector<RANDOM_MANAGER_INFO>::iterator it=this->m_RandomManagerInfo.begin();it != this->m_RandomManagerInfo.end();it++)
	{
		if(RandomNumber >= it->MinRate && RandomNumber < it->MaxRate)
		{
			(*value) = it->value;
			return 1;
		}
	}

	return 0;
}

bool CRandomManager::GetRandomElement(BYTE* value) // OK
{
	if(this->m_MaxRate == 0)
	{
		return 0;
	}

	if(this->m_RandomManagerInfo.empty() != 0)
	{
		return 0;
	}

	int RandomNumber = GetLargeRand()%this->m_MaxRate;

	for(std::vector<RANDOM_MANAGER_INFO>::iterator it=this->m_RandomManagerInfo.begin();it != this->m_RandomManagerInfo.end();it++)
	{
		if(RandomNumber >= it->MinRate && RandomNumber < it->MaxRate)
		{
			(*value) = it->value;
			return 1;
		}
	}

	return 0;
}

void CRandomManager::AddElement(std::intptr_t value,int rate) // OK
{
	if(rate == 0)
	{
		return;
	}

	RANDOM_MANAGER_INFO info;

	info.value = value;

	info.MinRate = this->m_MaxRate;

	info.MaxRate = (this->m_MaxRate=(this->m_MaxRate+rate));

	this->m_RandomManagerInfo.push_back(info);
}
