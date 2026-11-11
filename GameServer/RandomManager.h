// RandomManager.h: interface for the CRandomManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <climits>
#include <vector>

struct RANDOM_MANAGER_INFO
{
	std::intptr_t value;
	int MinRate;
	int MaxRate;
};

class CRandomManager
{
public:
	CRandomManager();
	virtual ~CRandomManager();
	void Clear();
	bool GetRandomElement(std::intptr_t* value);
	#if INTPTR_MAX > INT_MAX
	bool GetRandomElement(int* value);
	#endif
	bool GetRandomElement(WORD* value);
	bool GetRandomElement(BYTE* value);
	void AddElement(std::intptr_t value,int rate);

	// Weighted payloads can be either integer IDs or in-process pointers.
	// Never serialize this pointer-sized storage as a protocol field.
	template<class T> void AddElement(T* value,int rate)
	{
		this->AddElement(reinterpret_cast<std::intptr_t>(value),rate);
	}

	template<class T> bool GetRandomElement(T** value)
	{
		std::intptr_t selected;
		if(this->GetRandomElement(&selected) == 0)
		{
			return 0;
		}
		(*value) = reinterpret_cast<T*>(selected);
		return 1;
	}
private:
	int m_MaxRate;
	std::vector<RANDOM_MANAGER_INFO> m_RandomManagerInfo;
};
