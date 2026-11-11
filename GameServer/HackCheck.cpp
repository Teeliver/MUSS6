#include "stdafx.h"
#include "HackCheck.h"
#include "ServerInfo.h"
#if !defined(MU_NATIVE_MACOS)
#include "ThemidaSDK.h"
#else
#define VM_START
#define VM_END
#endif

char CustomerName[32];
BYTE EncDecKey1;
BYTE EncDecKey2;

void DecryptData(BYTE* lpMsg,int size) // OK
{
	if(gServerInfo.m_ServerEncDecKey1 != 0 || gServerInfo.m_ServerEncDecKey2 != 0)
	{
		MHPDecryptData(lpMsg,size);
	}

	for(int n=0;n < size;n++)
	{
		lpMsg[n] = (lpMsg[n]^EncDecKey1)-(EncDecKey2*EncDecKey1);
	}
}

void EncryptData(BYTE* lpMsg,int size) // OK
{
	for(int n=0;n < size;n++)
	{
		lpMsg[n] = (lpMsg[n]+(EncDecKey2*EncDecKey1))^EncDecKey1;
	}

	if(gServerInfo.m_ServerEncDecKey1 != 0 || gServerInfo.m_ServerEncDecKey2 != 0)
	{
		MHPEncryptData(lpMsg,size);
	}
}

void MHPDecryptData(BYTE* lpMsg,int size) // OK
{
	for(int n=0;n < size;n++)
	{
		lpMsg[n] = (lpMsg[n]^gServerInfo.m_ServerEncDecKey1)-gServerInfo.m_ServerEncDecKey2;
	}
}

void MHPEncryptData(BYTE* lpMsg,int size) // OK
{
	for(int n=0;n < size;n++)
	{
		lpMsg[n] = (lpMsg[n]+gServerInfo.m_ServerEncDecKey2)^gServerInfo.m_ServerEncDecKey1;
	}
}

void InitHackCheck() // OK
{
	VM_START

	strcpy_s(CustomerName,"MuOnline");

	WORD EncDecKey = 0;

	for(int n=0;n < sizeof(CustomerName);n++)
	{
		EncDecKey += (BYTE)(CustomerName[n]^gServerInfo.m_ServerSerial[(n%sizeof(gServerInfo.m_ServerSerial))]);
		EncDecKey ^= (BYTE)(CustomerName[n]-gServerInfo.m_ServerSerial[(n%sizeof(gServerInfo.m_ServerSerial))]);
	}

	EncDecKey1 = (BYTE)0xAB;
	EncDecKey2 = (BYTE)0xCD;

	EncDecKey1 += LOBYTE(EncDecKey);
	EncDecKey2 += HIBYTE(EncDecKey);

	if(gServerInfo.m_ServerEncDecKey1 != 0 || gServerInfo.m_ServerEncDecKey2 != 0)
	{
		WORD MHPEncDecKey = 0;

		for(int n=0;n < sizeof(CustomerName);n++)
		{
			MHPEncDecKey += CustomerName[n];
		}

		gServerInfo.m_ServerEncDecKey1 += LOBYTE(MHPEncDecKey);
		gServerInfo.m_ServerEncDecKey2 += HIBYTE(MHPEncDecKey);
	}

	VM_END
}
