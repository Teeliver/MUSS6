// BlackList.cpp: implementation of the CBlackList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BlackList.h"
#include "MemScript.h"
#include "Util.h"

CBlackList gBlackList;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlackList::CBlackList() // OK
{

}

CBlackList::~CBlackList() // OK
{
	this->m_BlackListIpInfo.clear();

}

void CBlackList::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_BlackListIpInfo.clear();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

#if defined(MU_NATIVE_MACOS)
			// GetToken above already selected this row. Do not skip an IP token.
			if (strcmp("end",lpMemScript->GetString()) == 0)
#else
			if (strcmp("end",lpMemScript->GetAsString()) == 0)
#endif
			{
				break;
			}

			BLACKLIST_IP_INFO info;
			
			strcpy_s(info.IpAddress,lpMemScript->GetString());

			this->m_BlackListIpInfo.insert(std::pair<std::string,BLACKLIST_IP_INFO>(std::string(info.IpAddress),info));
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CBlackList::CheckIpAddress(char* IpAddress) // OK
{
	std::map<std::string,BLACKLIST_IP_INFO>::iterator it = this->m_BlackListIpInfo.find(std::string(IpAddress));

	if (it == this->m_BlackListIpInfo.end())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
