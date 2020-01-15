#pragma once

#include <iostream>
#include <windows.h>
#include <newdev.h>
#include <SetupAPI.h>
#include <locale.h>
#include <tchar.h>
#include <string.h>
#include <iostream>


class CDriverInstall
{
public:
	CDriverInstall();
	~CDriverInstall();

public:
	BOOL GetINFData(FILE *pFile);
	BOOL GetSectionData(FILE* pFile, const char* szKey, const char bIsVender);
	BOOL FindSectionName(FILE *pFile, const char *szKey);
	BOOL IsInstalled();
	VOID InitialGlobalVar();
	void FindComma(LPSTR szData);
	VOID StrLTrim(LPSTR szData);
	VOID StrRTrim(LPSTR szData);
	VOID StrRight(LPSTR szData, WORD wCount);
	VOID ConvertGUIDToString(const GUID guid, LPSTR pData);
	BOOL StartInstallWDMDriver(LPCTSTR theInfName);
	BOOL InstallClassDriver(LPCTSTR theINFName);
	VOID UninstallWdmDriver(LPCTSTR theHardWare);

public:
	WORD g_wVender;
	WORD g_wHardware;
	CHAR g_strVender[20][64];
	CHAR g_strHardware[20][64];
	CHAR g_strHID[MAX_PATH];

	
};

