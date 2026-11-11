// ItemStack.cpp: implementation of the CItemStack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemStack.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CItemStack gItemStack;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemStack::CItemStack() // OK
{
	this->m_ItemStackInfo.clear();
}

CItemStack::~CItemStack() // OK
{

}

void CItemStack::Load(char* path) // OK
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

	this->m_ItemStackInfo.clear();

	try
	{
		while (true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_STACK_INFO info;

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.Level = lpMemScript->GetAsNumber();

					info.MaxStack = lpMemScript->GetAsNumber();

					info.CreateItemIndex = -1;

					this->m_ItemStackInfo.push_back(info);

					if(IsValidItem(info.ItemIndex) == 0){Console(LOG_RED,"[%02d-%03d] Invalid ItemIndex. [%s]",info.ItemIndex/MAX_ITEM_TYPE,info.ItemIndex%MAX_ITEM_TYPE,path);}
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_STACK_INFO info;

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.Level = -1;

					info.MaxStack = lpMemScript->GetAsNumber();

					info.CreateItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(),lpMemScript->GetAsNumber()));

					this->m_ItemStackInfo.push_back(info);

					if(IsValidItem(info.ItemIndex) == 0){Console(LOG_RED,"[%02d-%03d] Invalid ItemIndex. [%s]",info.ItemIndex/MAX_ITEM_TYPE,info.ItemIndex%MAX_ITEM_TYPE,path);}

					if(IsValidItem(info.CreateItemIndex) == 0){Console(LOG_RED,"[%02d-%03d] Invalid ItemIndex. [%s]",info.CreateItemIndex/MAX_ITEM_TYPE,info.CreateItemIndex%MAX_ITEM_TYPE,path);}
				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

int CItemStack::GetItemMaxStack(int index,int level) // OK
{
	for(std::vector<ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.begin(); it != this->m_ItemStackInfo.end(); it++)
	{
		if(it->ItemIndex != index)
		{
			continue;
		}

		if(it->Level != -1 && it->Level != level)
		{
			continue;
		}

		return it->MaxStack;
	}
	
	return 0;
}

int CItemStack::GetCreateItemIndex(int index) // OK
{
	for(std::vector<ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.begin(); it != this->m_ItemStackInfo.end(); it++)
	{
		if(it->ItemIndex != index)
		{
			continue;
		}

		return it->CreateItemIndex;
	}

	return -1;
}
