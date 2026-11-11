// HidManager.cpp: implementation of the CHidManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HidManager.h"
#include "BlackList.h"
#include "Log.h"
#include "Message.h"
#include "ServerInfo.h"

CHidManager gHidManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHidManager::CHidManager() // OK
{

}

CHidManager::~CHidManager() // OK
{

}

bool CHidManager::CheckHardwareId(char* HardwareId) // OK
{
	std::map<std::string,HARDWARE_ID_INFO>::iterator it = this->m_HardwareIdInfo.find(std::string(HardwareId));

	if(it == this->m_HardwareIdInfo.end())
	{
		return ((gServerInfo.m_MaxConnectionPerHID==0)?0:1);
	}
	else
	{
		return ((it->second.HardwareIdCount>=gServerInfo.m_MaxConnectionPerHID)?0:1);
	}
}

void CHidManager::InsertHardwareId(char* HardwareId) // OK
{
	HARDWARE_ID_INFO info;

	strcpy_s(info.HardwareId,HardwareId);

	info.HardwareIdCount = 1;

	std::map<std::string,HARDWARE_ID_INFO>::iterator it = this->m_HardwareIdInfo.find(std::string(HardwareId));

	if(it == this->m_HardwareIdInfo.end())
	{
		this->m_HardwareIdInfo.insert(std::pair<std::string,HARDWARE_ID_INFO>(std::string(HardwareId),info));
	}
	else
	{
		it->second.HardwareIdCount++;
	}
}

void CHidManager::RemoveHardwareId(char* HardwareId) // OK
{
	std::map<std::string,HARDWARE_ID_INFO>::iterator it = this->m_HardwareIdInfo.find(std::string(HardwareId));

	if(it != this->m_HardwareIdInfo.end())
	{
		if((--it->second.HardwareIdCount) == 0)
		{
			this->m_HardwareIdInfo.erase(it);
		}
	}
}

void CHidManager::CGHardwareIdRecv(PMSG_HARDWAREID_INFO_RECV* lpMsg,int aIndex) // OK
{
	char HardwareId[45] = {0};

	memcpy(HardwareId,lpMsg->HardwareId,sizeof(lpMsg->HardwareId));

	if(HardwareId[8] != '-' && HardwareId[17] != '-' && HardwareId[26] != '-' && HardwareId[35] != '-')
	{
		gObjDel(aIndex);
		return;
	}

	if(strlen(HardwareId) < 0 || strlen(HardwareId) > 44 || strlen(HardwareId) != 44)
	{
		gObjDel(aIndex);
		return;
	}
	
	if(gBlackList.CheckHardwareId(HardwareId) == 0)
	{
		gObjDel(aIndex);
		return;
	}

	if(this->CheckHardwareId(HardwareId) == 0)
	{
		gObjDel(aIndex);
		return;
	}

	gObj[aIndex].ClientVerify = 1;

	strcpy_s(gObj[aIndex].HardwareId,HardwareId);

	gLog.Output(LOG_CONNECT,"[ObjectManager][%d] Verified client [%s][%s]",aIndex,gObj[aIndex].IpAddr,gObj[aIndex].HardwareId);

	this->InsertHardwareId(lpMsg->HardwareId);
}