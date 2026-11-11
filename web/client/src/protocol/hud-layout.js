// Generated from actual UPDATE603/EXTRA1 server structs; do not hand-edit offsets.
export const hudLayout={
  "PMSG_PSHOP_ITEM_LIST_SEND": {
    "size": 55,
    "result": 5,
    "index": 6,
    "name": 8,
    "text": 18,
    "count": 54
  },
  "PMSG_PSHOP_ITEM_LIST": {
    "size": 20,
    "slot": 0,
    "ItemInfo": 1,
    "value": 16
  },
  "PMSG_PSHOP_BUY_ITEM_SEND": {
    "size": 20,
    "result": 4,
    "index": 5,
    "ItemInfo": 7,
    "slot": 19
  },
  "PMSG_VIEWPORT_SEND": {
    "size": 5,
    "count": 4
  },
  "PMSG_VIEWPORT_DESTROY_ITEM_SEND": {
    "size": 5,
    "count": 4
  },
  "PMSG_VIEWPORT_ITEM": {
    "size": 16,
    "index": 0,
    "x": 2,
    "y": 3,
    "ItemInfo": 4
  },
  "PMSG_ITEM_GET_RECV": {
    "size": 5,
    "index": 3
  },
  "PMSG_ITEM_GET_SEND": {
    "size": 20,
    "result": 3,
    "ItemInfo": 4,
    "ViewIndex": 16
  },
  "PMSG_ITEM_DROP_RECV": {
    "size": 6,
    "x": 3,
    "y": 4,
    "slot": 5
  },
  "PMSG_ITEM_DROP_SEND": {
    "size": 5,
    "result": 3,
    "slot": 4
  },
  "PMSG_CHARACTER_INFO_SEND": {
    "size": 136,
    "X": 4,
    "Y": 5,
    "Map": 6,
    "Dir": 7,
    "Experience": 8,
    "NextExperience": 16,
    "LevelUpPoint": 24,
    "Strength": 26,
    "Dexterity": 28,
    "Vitality": 30,
    "Energy": 32,
    "Life": 34,
    "MaxLife": 36,
    "Mana": 38,
    "MaxMana": 40,
    "Shield": 42,
    "MaxShield": 44,
    "BP": 46,
    "MaxBP": 48,
    "Money": 52,
    "PKLevel": 56,
    "CtlCode": 57,
    "FruitAddPoint": 58,
    "MaxFruitAddPoint": 60,
    "Leadership": 62,
    "FruitSubPoint": 64,
    "MaxFruitSubPoint": 66,
    "ExtInventory": 68,
    "ViewReset": 72,
    "ViewPoint": 76,
    "ViewCurHP": 80,
    "ViewMaxHP": 84,
    "ViewCurMP": 88,
    "ViewMaxMP": 92,
    "ViewCurBP": 96,
    "ViewMaxBP": 100,
    "ViewCurSD": 104,
    "ViewMaxSD": 108,
    "ViewStrength": 112,
    "ViewDexterity": 116,
    "ViewVitality": 120,
    "ViewEnergy": 124,
    "ViewLeadership": 128,
    "Class": 132
  },
  "PMSG_NEW_CHARACTER_INFO_SEND": {
    "size": 112,
    "Level": 4,
    "LevelUpPoint": 6,
    "Experience": 8,
    "NextExperience": 12,
    "Strength": 16,
    "Dexterity": 18,
    "Vitality": 20,
    "Energy": 22,
    "Leadership": 24,
    "Life": 26,
    "MaxLife": 28,
    "Mana": 30,
    "MaxMana": 32,
    "BP": 34,
    "MaxBP": 36,
    "Shield": 38,
    "MaxShield": 40,
    "FruitAddPoint": 42,
    "MaxFruitAddPoint": 44,
    "FruitSubPoint": 46,
    "MaxFruitSubPoint": 48,
    "ViewReset": 52,
    "ViewPoint": 56,
    "ViewCurHP": 60,
    "ViewMaxHP": 64,
    "ViewCurMP": 68,
    "ViewMaxMP": 72,
    "ViewCurBP": 76,
    "ViewMaxBP": 80,
    "ViewCurSD": 84,
    "ViewMaxSD": 88,
    "ViewStrength": 92,
    "ViewDexterity": 96,
    "ViewVitality": 100,
    "ViewEnergy": 104,
    "ViewLeadership": 108
  },
  "PMSG_NEW_CHARACTER_CALC_SEND": {
    "size": 172,
    "ViewCurHP": 4,
    "ViewMaxHP": 8,
    "ViewCurMP": 12,
    "ViewMaxMP": 16,
    "ViewCurBP": 20,
    "ViewMaxBP": 24,
    "ViewCurSD": 28,
    "ViewMaxSD": 32,
    "ViewAddStrength": 36,
    "ViewAddDexterity": 40,
    "ViewAddVitality": 44,
    "ViewAddEnergy": 48,
    "ViewAddLeadership": 52,
    "ViewPhysiDamageMin": 56,
    "ViewPhysiDamageMax": 60,
    "ViewMagicDamageMin": 64,
    "ViewMagicDamageMax": 68,
    "ViewCurseDamageMin": 72,
    "ViewCurseDamageMax": 76,
    "ViewMulPhysiDamage": 80,
    "ViewDivPhysiDamage": 84,
    "ViewMulMagicDamage": 88,
    "ViewDivMagicDamage": 92,
    "ViewMulCurseDamage": 96,
    "ViewDivCurseDamage": 100,
    "ViewMagicDamageRate": 104,
    "ViewCurseDamageRate": 108,
    "ViewPhysiSpeed": 112,
    "ViewMagicSpeed": 116,
    "ViewAttackSuccessRate": 120,
    "ViewAttackSuccessRatePvP": 124,
    "ViewDefense": 128,
    "ViewDefenseSuccessRate": 132,
    "ViewDefenseSuccessRatePvP": 136,
    "ViewDamageMultiplier": 140,
    "ViewRFDamageMultiplierA": 144,
    "ViewRFDamageMultiplierB": 148,
    "ViewRFDamageMultiplierC": 152,
    "ViewDarkSpiritAttackDamageMin": 156,
    "ViewDarkSpiritAttackDamageMax": 160,
    "ViewDarkSpiritAttackSpeed": 164,
    "ViewDarkSpiritAttackSuccessRate": 168
  },
  "PMSG_LEVEL_UP_SEND": {
    "size": 52,
    "Level": 4,
    "LevelUpPoint": 6,
    "MaxLife": 8,
    "MaxMana": 10,
    "MaxShield": 12,
    "MaxBP": 14,
    "FruitAddPoint": 16,
    "MaxFruitAddPoint": 18,
    "FruitSubPoint": 20,
    "MaxFruitSubPoint": 22,
    "ViewPoint": 24,
    "ViewMaxHP": 28,
    "ViewMaxMP": 32,
    "ViewMaxBP": 36,
    "ViewMaxSD": 40,
    "ViewExperience": 44,
    "ViewNextExperience": 48
  },
  "PMSG_LEVEL_UP_POINT_SEND": {
    "size": 52,
    "result": 4,
    "MaxLifeAndMana": 6,
    "MaxShield": 8,
    "MaxBP": 10,
    "ViewPoint": 12,
    "ViewMaxHP": 16,
    "ViewMaxMP": 20,
    "ViewMaxBP": 24,
    "ViewMaxSD": 28,
    "ViewStrength": 32,
    "ViewDexterity": 36,
    "ViewVitality": 40,
    "ViewEnergy": 44,
    "ViewLeadership": 48
  },
  "PMSG_LIFE_SEND": {
    "size": 20,
    "type": 3,
    "life": 4,
    "flag": 6,
    "shield": 7,
    "ViewHP": 12,
    "ViewSD": 16
  },
  "PMSG_MANA_SEND": {
    "size": 16,
    "type": 3,
    "mana": 4,
    "bp": 6,
    "ViewMP": 8,
    "ViewBP": 12
  },
  "PMSG_CHARACTER_REGEN_SEND": {
    "size": 44,
    "X": 4,
    "Y": 5,
    "Map": 6,
    "Dir": 7,
    "Life": 8,
    "Mana": 10,
    "Shield": 12,
    "BP": 14,
    "Experience": 16,
    "Money": 24,
    "ViewCurHP": 28,
    "ViewCurMP": 32,
    "ViewCurBP": 36,
    "ViewCurSD": 40
  },
  "PMSG_MONSTER_DAMAGE_SEND": {
    "size": 24,
    "damage": 4,
    "ShieldDamage": 6,
    "ViewCurHP": 8,
    "ViewCurSD": 12,
    "ViewDamageHP": 16,
    "ViewDamageSD": 20
  },
  "PMSG_ITEM_MOVE_RECV": {
    "size": 19,
    "SourceFlag": 3,
    "SourceSlot": 4,
    "ItemInfo": 5,
    "TargetFlag": 17,
    "TargetSlot": 18
  },
  "PMSG_ITEM_MOVE_SEND": {
    "size": 17,
    "result": 3,
    "slot": 4,
    "ItemInfo": 5
  },
  "PMSG_ITEM_REPAIR_RECV": {
    "size": 5,
    "slot": 3,
    "type": 4
  },
  "PMSG_ITEM_REPAIR_SEND": {
    "size": 8,
    "money": 4
  },
  "PMSG_PSHOP_SET_ITEM_PRICE_RECV": {
    "size": 9,
    "slot": 4,
    "price": 5
  },
  "PMSG_PSHOP_SET_ITEM_PRICE_SEND": {
    "size": 6,
    "result": 4,
    "slot": 5
  },
  "PMSG_PSHOP_OPEN_RECV": {
    "size": 40,
    "text": 4
  },
  "PMSG_PSHOP_OPEN_SEND": {
    "size": 5,
    "result": 4
  },
  "PMSG_PSHOP_CLOSE_SEND": {
    "size": 7,
    "result": 4,
    "index": 5
  },
  "PMSG_PSHOP_SELL_ITEM_SEND": {
    "size": 15,
    "slot": 4,
    "name": 5
  },
  "PMSG_NPC_TALK_RECV": {
    "size": 5,
    "index": 3
  },
  "PMSG_NPC_TALK_SEND": {
    "size": 11,
    "result": 3,
    "level": 4
  },
  "PMSG_SHOP_ITEM_LIST_SEND": {
    "size": 6,
    "type": 4,
    "count": 5
  },
  "PMSG_SHOP_ITEM_LIST": {
    "size": 13,
    "slot": 0,
    "ItemInfo": 1
  },
  "PMSG_SHOP_PRICE_LIST_SEND": {
    "size": 7,
    "count": 5,
    "type": 6
  },
  "PMSG_SHOP_PRICE": {
    "size": 20,
    "ItemIndex": 0,
    "ItemLevel": 4,
    "ItemDur": 8,
    "ItemNewOption": 12,
    "ItemPrice": 16
  },
  "PMSG_ITEM_BUY_RECV": {
    "size": 4,
    "slot": 3
  },
  "PMSG_ITEM_BUY_SEND": {
    "size": 16,
    "result": 3,
    "ItemInfo": 4
  },
  "PMSG_ITEM_SELL_RECV": {
    "size": 4,
    "slot": 3
  },
  "PMSG_ITEM_SELL_SEND": {
    "size": 8,
    "result": 3,
    "money": 4
  },
  "PMSG_TAX_INFO_SEND": {
    "size": 6,
    "type": 4,
    "rate": 5
  },
  "PMSG_WAREHOUSE_MONEY_RECV": {
    "size": 8,
    "type": 3,
    "money": 4
  },
  "PMSG_WAREHOUSE_MONEY_SEND": {
    "size": 12,
    "result": 3,
    "WarehouseMoney": 4,
    "InventoryMoney": 8
  },
  "PMSG_WAREHOUSE_PASSWORD_RECV": {
    "size": 16,
    "type": 3,
    "password": 4,
    "PersonalCode": 6
  },
  "PMSG_WAREHOUSE_STATE_SEND": {
    "size": 4,
    "state": 3
  },
  "PMSG_CHAOS_MIX_RECV": {
    "size": 5,
    "type": 3,
    "info": 4
  },
  "PMSG_CHAOS_MIX_RATE_RECV": {
    "size": 8,
    "type": 4
  },
  "PMSG_CHAOS_MIX_RATE_SEND": {
    "size": 16,
    "rate": 4,
    "money": 8,
    "tax": 12
  },
  "PMSG_CHAOS_MIX_SEND": {
    "size": 16,
    "result": 3,
    "ItemInfo": 4
  },
  "PMSG_JEWEL_MIX_RECV": {
    "size": 6,
    "type": 4,
    "level": 5
  },
  "PMSG_JEWEL_UNMIX_RECV": {
    "size": 7,
    "type": 4,
    "level": 5,
    "slot": 6
  },
  "PMSG_JEWEL_MIX_RESULT_SEND": {
    "size": 5,
    "result": 4
  },
  "PMSG_QUEST_WORLD_NPC_TALK_SEND": {
    "size": 12,
    "MonsterClass": 4,
    "contribution": 8
  },
  "QUEST_WORLD_OBJECTIVE": {
    "size": 26,
    "type": 0,
    "index": 4,
    "value": 6,
    "count": 10,
    "ItemInfo": 14
  },
  "QUEST_WORLD_REWARD": {
    "size": 22,
    "type": 0,
    "index": 4,
    "value": 6,
    "ItemInfo": 10
  },
  "PMSG_QUEST_WORLD_LIST_SEND": {
    "size": 8,
    "MonsterClass": 4,
    "count": 6,
    "result": 7
  },
  "PMSG_QUEST_WORLD_LIST": {
    "size": 4,
    "QuestIndex": 0,
    "QuestGroup": 2
  },
  "PMSG_QUEST_WORLD_DIALOG_RECV": {
    "size": 9,
    "QuestIndex": 4,
    "QuestGroup": 6,
    "QuestOption": 8
  },
  "PMSG_QUEST_WORLD_ACCEPT_RECV": {
    "size": 9,
    "QuestIndex": 4,
    "QuestGroup": 6,
    "QuestOption": 8
  },
  "PMSG_QUEST_WORLD_DETAIL_SEND": {
    "size": 251,
    "QuestIndex": 4,
    "QuestGroup": 6,
    "ObjectiveCount": 8,
    "RewardCount": 9,
    "result": 10
  },
  "PMSG_QUEST_WORLD_FINISH_SEND": {
    "size": 9,
    "QuestIndex": 4,
    "QuestGroup": 6,
    "QuestResult": 8
  },
  "PMSG_QUEST_WORLD_CANCEL_SEND": {
    "size": 9,
    "QuestIndex": 4,
    "QuestGroup": 6,
    "QuestResult": 8
  },
  "PMSG_GENS_SYSTEM_INSERT_SEND": {
    "size": 6,
    "result": 4,
    "GensFamily": 5
  },
  "PMSG_GENS_SYSTEM_DELETE_SEND": {
    "size": 5,
    "result": 4
  },
  "PMSG_GENS_SYSTEM_REWARD_SEND": {
    "size": 5,
    "result": 4
  },
  "PMSG_GENS_SYSTEM_MEMBER_SEND": {
    "size": 24,
    "GensFamily": 4,
    "GensRank": 8,
    "GensSymbol": 12,
    "GensContribution": 16,
    "GensNextContribution": 20
  },
  "PMSG_GUILD_CREATE_RECV": {
    "size": 44,
    "type": 3,
    "GuildName": 4,
    "Mark": 12
  },
  "PMSG_GUILD_CREATE_RESULT_SEND": {
    "size": 5,
    "result": 3,
    "type": 4
  },
  "PMSG_DUEL_WATCH_RECV": {
    "size": 5,
    "number": 4
  },
  "PMSG_DUEL_BUTTON_RECV": {
    "size": 5,
    "number": 4
  },
  "PMSG_DUEL_SCORE_SEND": {
    "size": 10,
    "index1": 4,
    "index2": 6,
    "score": 8
  },
  "PMSG_DUEL_INTERFACE_SEND": {
    "size": 12,
    "index1": 4,
    "index2": 6,
    "life1": 8,
    "life2": 9,
    "shield1": 10,
    "shield2": 11
  },
  "PMSG_DUEL_NPC_INFO_SEND": {
    "size": 4
  },
  "PMSG_DUEL_NPC_INFO": {
    "size": 22,
    "name1": 0,
    "name2": 10,
    "state": 20,
    "flag": 21
  },
  "PMSG_DUEL_GLADIATOR_ADD_SEND": {
    "size": 30,
    "result": 4,
    "number": 5,
    "name1": 6,
    "name2": 16,
    "index1": 26,
    "index2": 28
  },
  "PMSG_DUEL_SPECTATOR_ADD_SEND": {
    "size": 14,
    "name": 4
  },
  "PMSG_DUEL_GLADIATOR_DEL_SEND": {
    "size": 5,
    "result": 4
  },
  "PMSG_DUEL_SPECTATOR_DEL_SEND": {
    "size": 14,
    "name": 4
  },
  "PMSG_DUEL_SPECTATOR_LIST_SEND": {
    "size": 5,
    "count": 4
  },
  "PMSG_DUEL_SPECTATOR_LIST": {
    "size": 10,
    "name": 0
  },
  "PMSG_DUEL_WIN_SEND": {
    "size": 24,
    "WinnerName": 4,
    "LoserName": 14
  },
  "PMSG_DUEL_ROUND_START_SEND": {
    "size": 5,
    "flag": 4
  }
};
