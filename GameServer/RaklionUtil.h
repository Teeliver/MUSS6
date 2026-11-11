// RaklionUtil.h: interface for the CRaklionUtil class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

#pragma pack(push,1)
struct PMSG_RAKLION_STATE_SEND
{
	PSBMSG_HEAD header; // C1:D1:11
	BYTE state;
	BYTE DetailState;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PMSG_RAKLION_STATE_CHANGE_SEND
{
	PSBMSG_HEAD header; // C1:D1:12
	BYTE state;
	BYTE DetailState;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PMSG_RAKLION_SUCCESS_SEND
{
	PSBMSG_HEAD header; // C1:D1:13
	BYTE result;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PMSG_RAKLION_AREA_ATTACK_SEND
{
	PSBMSG_HEAD header; // C1:D1:14
	BYTE MonsterClass[2];
	BYTE type;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PMSG_RAKLION_OBJECT_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:D1:15
	BYTE MonsterCount;
	BYTE UserCount;
};
#pragma pack(pop)

//**********************************************//
//**********************************************//
//**********************************************//

class CRaklionUtil
{
public:
	CRaklionUtil();
	virtual ~CRaklionUtil();
	void NotifyRaklionState(int aIndex,int state,int DetailState);
	void NotifyRaklionChangeState(int state,int DetailState);
	void NotifyRaklionResult(int result);
	void NotifyRaklionAreaAttack(int aIndex,int bIndex,int type);
	void NotifyRaklionObjectCount(int MonsterCount,int UserCount);
	void SendDataRaklionMapUser(BYTE* lpMsg,int size);
	void SendDataRaklionBossMapUser(BYTE* lpMsg,int size);
};

extern CRaklionUtil gRaklionUtil;
