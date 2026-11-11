// JewelOfHarmonyType.cpp: implementation of the CJewelOfHarmonyType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JewelOfHarmonyType.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CJewelOfHarmonyType gJewelOfHarmonyType;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJewelOfHarmonyType::CJewelOfHarmonyType() // OK
{
	this->m_JewelOfHarmonyTypeInfo.clear();
}

CJewelOfHarmonyType::~CJewelOfHarmonyType() // OK
{

}

void CJewelOfHarmonyType::Load(char* path) // OK
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

	this->m_JewelOfHarmonyTypeInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			JEWEL_OF_HARMONY_TYPE_INFO info;

			info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

			info.Level = lpMemScript->GetAsNumber();

			this->m_JewelOfHarmonyTypeInfo.insert(std::pair<int,JEWEL_OF_HARMONY_TYPE_INFO>(info.ItemIndex,info));

			if(IsValidItem(info.ItemIndex) == 0){Console(LOG_RED,"[%02d-%03d] Invalid ItemIndex. [%s]",info.ItemIndex/MAX_ITEM_TYPE,info.ItemIndex%MAX_ITEM_TYPE,path);}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

JEWEL_OF_HARMONY_TYPE_INFO* CJewelOfHarmonyType::GetInfo(int index) // OK
{
	std::map<int, JEWEL_OF_HARMONY_TYPE_INFO>::iterator it = this->m_JewelOfHarmonyTypeInfo.find(index);

	if(it == this->m_JewelOfHarmonyTypeInfo.end())
	{
		return 0;
	}
	else
	{
		return &it->second;
	}
};

bool CJewelOfHarmonyType::CheckJewelOfHarmonyItemType(CItem* lpItem) // OK
{
	JEWEL_OF_HARMONY_TYPE_INFO* lpInfo = this->GetInfo(lpItem->m_Index);

	if(lpInfo != 0)
	{
		if(lpInfo->Level > lpItem->m_Level)
		{
			return 0;
		}
	}

	return 1;
}
