// HidManager.h: interface for the CHidManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_HARDWAREID_INFO_RECV
{
	PSBMSG_HEAD header; // C1:F3:09
	char HardwareId[45];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct HARDWARE_ID_INFO
{
	char HardwareId[45];
	WORD HardwareIdCount;
};

class CHidManager
{
public:
	CHidManager();
	virtual ~CHidManager();
	bool CheckHardwareId(char* HardwareId);
	void InsertHardwareId(char* HardwareId);
	void RemoveHardwareId(char* HardwareId);
	void CGHardwareIdRecv(PMSG_HARDWAREID_INFO_RECV* lpMsg,int aIndex);
private:
	std::map<std::string,HARDWARE_ID_INFO> m_HardwareIdInfo;
};

extern CHidManager gHidManager;
