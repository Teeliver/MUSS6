// ItemBagManager.cpp: implementation of the CItemBagManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemBagManager.h"
#include "ItemManager.h"
#include "Log.h"
#include "MemScript.h"
#include "Path.h"
#include "Util.h"

CItemBagManager gItemBagManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemBagManager::CItemBagManager() // OK
{
	this->m_ItemBagManagerItemInfo.clear();

	this->m_ItemBagManagerMonsterInfo.clear();

	this->m_ItemBagManagerSpecialInfo.clear();
}

CItemBagManager::~CItemBagManager() // OK
{

}

void CItemBagManager::Load(char* path) // OK
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

	this->m_ItemBagManagerItemInfo.clear();

	this->m_ItemBagManagerMonsterInfo.clear();

	this->m_ItemBagManagerSpecialInfo.clear();

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

					ITEM_BAG_MANAGER_ITEM_INFO info;

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.ItemLevel = lpMemScript->GetAsNumber();

					strcpy_s(info.BagPath,lpMemScript->GetAsString());

					this->m_ItemBagManagerItemInfo.push_back(info);

					if(IsValidItem(info.ItemIndex) == 0){Console(LOG_RED,"[%02d-%03d] Invalid ItemIndex. [%s]",info.ItemIndex/MAX_ITEM_TYPE,info.ItemIndex%MAX_ITEM_TYPE,path);}
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_MANAGER_MONSTER_INFO info;

					info.MonsterClass = lpMemScript->GetNumber();

					strcpy_s(info.BagPath,lpMemScript->GetAsString());

					this->m_ItemBagManagerMonsterInfo.insert(std::pair<int,ITEM_BAG_MANAGER_MONSTER_INFO>(info.MonsterClass,info));

					if(IsValidMonster(info.MonsterClass) == 0){Console(LOG_RED,"[%d] Invalid MonsterIndex. [%s]",info.MonsterClass,path);}
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_MANAGER_SPECIAL_INFO info;

					info.SpecialValue = lpMemScript->GetNumber();

					info.SpecialLevel = lpMemScript->GetAsNumber();

					info.SpecialState = lpMemScript->GetAsNumber();

					strcpy_s(info.BagPath,lpMemScript->GetAsString());

					this->m_ItemBagManagerSpecialInfo.push_back(info);
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

void CItemBagManager::LoadEventItemBag() // OK
{
	for(std::vector<ITEM_BAG_MANAGER_ITEM_INFO>::iterator it = this->m_ItemBagManagerItemInfo.begin(); it != this->m_ItemBagManagerItemInfo.end(); it++)
	{
		char path[MAX_PATH];

		wsprintf(path,"EventItemBag\\%s.txt",it->BagPath);

		it->ItemBag.Load(gPath.GetFullPath(path));
	}

	for(std::map<int,ITEM_BAG_MANAGER_MONSTER_INFO>::iterator it = this->m_ItemBagManagerMonsterInfo.begin(); it != this->m_ItemBagManagerMonsterInfo.end(); it++)
	{
		char path[MAX_PATH];

		wsprintf(path,"EventItemBag\\%s.txt",it->second.BagPath);

		it->second.ItemBag.Load(gPath.GetFullPath(path));
	}

	for(std::vector<ITEM_BAG_MANAGER_SPECIAL_INFO>::iterator it = this->m_ItemBagManagerSpecialInfo.begin(); it != this->m_ItemBagManagerSpecialInfo.end(); it++)
	{
		char path[MAX_PATH];

		wsprintf(path,"EventItemBag\\%s.txt",it->BagPath);

		it->ItemBag.Load(gPath.GetFullPath(path));
	}
}

bool CItemBagManager::GetItemBySpecialValue(int SpecialValue,int SpecialLevel,int SpecialState,LPOBJ lpObj,CItem* lpItem) // OK
{
	for(std::vector<ITEM_BAG_MANAGER_SPECIAL_INFO>::iterator it = this->m_ItemBagManagerSpecialInfo.begin(); it != this->m_ItemBagManagerSpecialInfo.end(); it++)
	{
		if(it->SpecialValue == SpecialValue && (it->SpecialLevel == -1 || it->SpecialLevel == SpecialLevel) && (it->SpecialState == -1 || it->SpecialState == SpecialState))
		{
			return it->ItemBag.GetItem(lpObj,lpItem);
		}
	}

	return 0;
}

bool CItemBagManager::DropItemByItemIndex(int ItemIndex,int ItemLevel,LPOBJ lpObj,int map,int x,int y) // OK
{
	for(std::vector<ITEM_BAG_MANAGER_ITEM_INFO>::iterator it = this->m_ItemBagManagerItemInfo.begin(); it != this->m_ItemBagManagerItemInfo.end(); it++)
	{
		if(it->ItemIndex == ItemIndex && (it->ItemLevel == -1 || it->ItemLevel == ItemLevel))
		{
			return it->ItemBag.DropItem(lpObj,map,x,y);
		}
	}

	return 0;
}

bool CItemBagManager::DropItemByMonsterClass(int MonsterClass,LPOBJ lpObj,int map,int x,int y) // OK
{
	std::map<int,ITEM_BAG_MANAGER_MONSTER_INFO>::iterator it = this->m_ItemBagManagerMonsterInfo.find(MonsterClass);

	if(it != this->m_ItemBagManagerMonsterInfo.end())
	{
		return it->second.ItemBag.DropItem(lpObj, map, x, y);
	}
	
	return 0;
}

bool CItemBagManager::DropItemBySpecialValue(int SpecialValue,int SpecialLevel,int SpecialState,LPOBJ lpObj,int map,int x,int y) // OK
{
	for(std::vector<ITEM_BAG_MANAGER_SPECIAL_INFO>::iterator it = this->m_ItemBagManagerSpecialInfo.begin(); it != this->m_ItemBagManagerSpecialInfo.end(); it++)
	{
		if(it->SpecialValue == SpecialValue && (it->SpecialLevel == -1 || it->SpecialLevel == SpecialLevel) && (it->SpecialState == -1 || it->SpecialState == SpecialState))
		{
			return it->ItemBag.DropItem(lpObj,map,x,y);
		}
	}

	return 0;
}