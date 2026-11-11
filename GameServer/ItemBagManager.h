// ItemBagManager.h: interface for the CItemBagManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemBag.h"
#include "ItemBagEx.h"
#include "User.h"

enum eItemBagSpecialValue
{
	ITEM_BAG_NONE = 0,
	ITEM_BAG_BLOOD_CASTLE = 1,
	ITEM_BAG_CHAOS_CASTLE = 2,
	ITEM_BAG_SENIOR_MIX = 3,
	ITEM_BAG_CRYWOLF = 4,
	ITEM_BAG_LEO_THE_HELPER = 5,
	ITEM_BAG_CHAOS_CARD_MIX = 6,
	ITEM_BAG_ILLUSION_TEMPLE = 7,
	ITEM_BAG_CHERRY_BLOSSOM_MIX = 8,
	ITEM_BAG_SANTA_CLAUS = 9,
	ITEM_BAG_LUCKY_COIN = 10,
	ITEM_BAG_DOUBLE_GOER = 11,
	ITEM_BAG_IMPERIAL_GUARDIAN = 12,
	ITEM_BAG_DEVIL_SQUARE = 13,
	ITEM_BAG_CUSTOM_ARENA = 14,
	ITEM_BAG_CHAOS_MIX = 15,
	ITEM_BAG_MU_RUMMY = 16,
	ITEM_BAG_CHAOS_CASTLE_FINAL = 17,
	ITEM_BAG_TROPHIES_MIX = 18,
	ITEM_BAG_ACHERON_GUARDIAN = 19,
	ITEM_BAG_ARCA_WAR = 20,
};

struct ITEM_BAG_MANAGER_ITEM_INFO
{
	int ItemIndex;
	int ItemLevel;
	char BagPath[50];
	CItemBag ItemBag;
};

struct ITEM_BAG_MANAGER_MONSTER_INFO
{
	int MonsterClass;
	char BagPath[50];
	CItemBag ItemBag;
};

struct ITEM_BAG_MANAGER_SPECIAL_INFO
{
	int SpecialValue;
	int SpecialLevel;
	int SpecialState;
	char BagPath[50];
	CItemBag ItemBag;
};

class CItemBagManager
{
public:
	CItemBagManager();
	virtual ~CItemBagManager();
	void Load(char* path);
	void LoadEventItemBag();
	bool GetItemBySpecialValue(int SpecialValue,int SpecialLevel,int SpecialState,LPOBJ lpObj,CItem* lpItem);
	bool DropItemByItemIndex(int ItemIndex,int ItemLevel,LPOBJ lpObj,int map,int x,int y);
	bool DropItemByMonsterClass(int MonsterClass,LPOBJ lpObj,int map,int x,int y);
	bool DropItemBySpecialValue(int SpecialValue,int SpecialLevel,int SpecialState,LPOBJ lpObj,int map,int x,int y);
private:
	std::vector<ITEM_BAG_MANAGER_ITEM_INFO> m_ItemBagManagerItemInfo;
	std::map<int,ITEM_BAG_MANAGER_MONSTER_INFO> m_ItemBagManagerMonsterInfo;
	std::vector<ITEM_BAG_MANAGER_SPECIAL_INFO> m_ItemBagManagerSpecialInfo;
};

extern CItemBagManager gItemBagManager;