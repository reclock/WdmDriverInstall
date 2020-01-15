#include "stdafx.h"
#include "CDriverInstall.h"

#pragma comment(lib,"newdev.lib")
#pragma comment(lib,"setupapi.lib")

CDriverInstall::CDriverInstall()
{
	g_wHardware = 0;
	g_wVender = 0;
	ZeroMemory(g_strHID, MAX_LABEL_LEN + 1);
}


CDriverInstall::~CDriverInstall()
{
}


BOOL CDriverInstall::GetINFData(FILE * pFile)
{
	WORD wLoop;

	if (!g_wVender || !g_wHardware)
		InitialGlobalVar();
	if (GetSectionData(pFile, "[Manufacturer]", TRUE) == FALSE)
		return 0;

	for (wLoop = 0; wLoop < g_wVender; wLoop++)
	{
		CHAR szVender[64] = { 0 };
		//UnicodeToAnsi(g_strVender[wLoop], wcslen(g_strVender[wLoop]), szVender, 64);
		strcpy_s(szVender, g_strVender[wLoop]);

		GetSectionData(pFile, szVender, FALSE);
	}
	if (g_wHardware != 0)
	{
		if (IsInstalled() == TRUE)//如果已经安装
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;
}


//读取INF节数据
BOOL CDriverInstall::GetSectionData(FILE * pFile, const char * szKey, const char bIsVender)
{
	char szData[128] = { 0 };

	if (bIsVender)
		strcpy_s(szData, szKey);
	else
		sprintf_s(szData, _countof(szData), "[%s]", szKey);

	if (FindSectionName(pFile, szData) == FALSE)
		return FALSE;

	RtlZeroMemory(szData, sizeof(char) * 128);
	while (!feof(pFile))
	{
		char *str = NULL;
		fgets(szData, 127, pFile);
		szData[strlen(szData) - 1] = 0;
		StrLTrim(szData);
		StrRTrim(szData);
		if (!*szData)
			continue;
		if (szData[0] == ';')
			continue;

		if (strchr(szData, '['))
		{
			StrLTrim(szData);
			if (szData[0] != ';')
				return 1;
			else
				continue;
		}

		if (bIsVender)
			str = strchr(szData, '=');
		else
			str = strchr(szData, ',');

		if (*str)
		{
			char szTmp[128] = { 0 };
			WORD pos = (WORD)(str - szData + 1);

			StrRight(szData, (short)(strlen(szData) - pos));
			StrLTrim(szData);
			StrRTrim(szData);
			FindComma(szData);
			if (bIsVender)
			{
				//std::string ss(strchr(szData, '.'));
				//pos = (WORD)(strlen(szData) - ss.length() + 1);
				//StrRight(ss, (short)(strlen(ss) - pos));
				//AnsiToUnicode(szData, strlen(szData), g_strVender[g_wVender++], 64);

				std::string ss(szData);
				std::string s2;
				while (1)
				{

					std::string substr = ss.substr(0, ss.find('.'));

					if (substr.compare(s2) == 0)
						break;
					if (substr.length() > 0)
						//AnsiToUnicode(substr.c_str(), substr.length(), g_strVender[g_wVender++], 64);
						strcpy_s(g_strVender[g_wVender++], substr.c_str());
					else
						break;
					ss = ss.substr(ss.find('.') + 1);
					s2 = substr;
				}
			}
			else
			{
				//AnsiToUnicode(szData, strlen(szData), g_strHardware[g_wHardware++], 64);
				_tcscpy_s(g_strHardware[g_wHardware++], 64, szData);
			}
		}/* end if */
	}
	return TRUE;
}


//读取INF节名称
BOOL CDriverInstall::FindSectionName(FILE * pFile, const char * szKey)
{
	char szData[256] = { 0 };

	if (!pFile)
		return FALSE;

	//将文件内部的位置指针重新指向一个流（数据流/文件）的开头
	rewind(pFile);
	//循环读取文件内容
	while (!feof(pFile))
	{
		//读取一行
		fgets(szData, 255, pFile);
		//去除前后空格
		StrLTrim(szData);
		StrRTrim(szData);

		if (strcmp(szKey, szData) == 0)
			return TRUE;
	}
	return FALSE;
}



//判断是否已安装
BOOL CDriverInstall::IsInstalled()
{
	HDEVINFO hDecInfo = 0L;
	SP_DEVINFO_DATA spDevInfoData = { 0L };
	WORD wIdx;
	BOOL bIsFound;

	hDecInfo = SetupDiGetClassDevs(0L, 0, 0, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDecInfo == INVALID_HANDLE_VALUE)
	{
		//ShowErrMsg(GetLastError(), L"SetUpDiGetClassDevs");
		return FALSE;
	}

	spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	wIdx = 0;
	bIsFound = 0;
	while (++wIdx)
	{
		if (SetupDiEnumDeviceInfo(hDecInfo, wIdx, &spDevInfoData))
		{
			LPSTR ptr;
			LPBYTE pBuffer = NULL;
			DWORD dwData = 0L;
			DWORD dwRetVal;
			DWORD dwBufSize = 0;

			while (TRUE)
			{
				dwRetVal = SetupDiGetDeviceRegistryProperty(
					hDecInfo,
					&spDevInfoData,
					SPDRP_HARDWAREID,
					&dwData,
					(PBYTE)pBuffer,
					dwBufSize,
					&dwBufSize
				);
				if (!dwRetVal)
					dwRetVal = GetLastError();
				else
					break;
				if (dwRetVal == ERROR_INVALID_DATA)
					break;
				else if (dwRetVal == ERROR_INSUFFICIENT_BUFFER)
				{
					if (pBuffer)
						LocalFree(pBuffer);
					pBuffer = (LPBYTE)LocalAlloc(LPTR, dwBufSize);
				}
				else
				{
					//ShowErrMsg(dwRetVal, L"SetupDiGetDeviceRegistryProperty");
					SetupDiDestroyDeviceInfoList(hDecInfo);
					return FALSE;
				}
			}

			if (dwRetVal == ERROR_INVALID_DATA)
				continue;
			for (ptr = (LPSTR)pBuffer; *ptr && (ptr < (LPSTR)&pBuffer[dwBufSize]); ptr += _tcslen(ptr) + sizeof(CHAR))
			{
				WORD wLoop;
				for (wLoop = 0; wLoop < g_wHardware; wLoop++)
				{
					if (!_tcscmp(g_strHardware[wLoop], ptr))
					{
						bIsFound = TRUE;
						break;
					}
				}
			}
			if (pBuffer)
				LocalFree(pBuffer);
			if (bIsFound)
				break;
		}
	}
	SetupDiDestroyDeviceInfoList(hDecInfo);
	return bIsFound;
}


//初始化变量
VOID CDriverInstall::InitialGlobalVar()
{
	WORD wLoop;

	g_wVender = g_wHardware = 0;
	for (wLoop = 0; wLoop < 20; wLoop++)
	{
		RtlZeroMemory(g_strVender[wLoop], sizeof(TCHAR) * 64);
		RtlZeroMemory(g_strHardware[wLoop], sizeof(TCHAR) * 64);
	}
}

//过滤字符
void CDriverInstall::FindComma(LPSTR szData)
{
	WORD wLen = (WORD)strlen(szData);
	WORD wIdx;
	WORD wLoop;
	char szTmp[128] = { 0 };

	for (wIdx = 0, wLoop = 0; wLoop < wLen; wLoop++)
	{
		if (szData[wLoop] == ',')
			szData[wLoop] = '.';
		else if (szData[wLoop] == ' ')
			continue;
		szTmp[wIdx++] = szData[wLoop];
	}
	memcpy(szData, szTmp, wIdx * sizeof(char));
	szData[wIdx] = 0;
}


//去除左边空格
VOID CDriverInstall::StrLTrim(LPSTR szData)
{
	LPSTR ptr = szData;
	//判断是否为空格
	while (isspace(*ptr))
		ptr++;

	if (strcmp(ptr, szData))
	{
		WORD wLen = (WORD)(strlen(szData) - (ptr - szData));
		memmove(szData, ptr, (wLen + 1) * sizeof(char));
	}
}



//去除右边空格
VOID CDriverInstall::StrRTrim(LPSTR szData)
{
	LPSTR ptr = szData;
	LPSTR pTmp = NULL;

	//debug模式下 使用isspace判断中文 需要设置编码
#if defined(WIN32) && defined(_DEBUG)
	char* locale = setlocale(LC_ALL, ".OCP");
#endif 

	while (*ptr != 0)
	{
		//判断是否为空格
		if (isspace(*ptr))
		{
			if (!pTmp)
				pTmp = ptr;
		}
		else
			pTmp = NULL;
		ptr++;
	}

	if (pTmp)
	{
		*pTmp = 0;
		memmove(szData, szData, strlen(szData) - strlen(pTmp));
	}
}



//从右侧截取字符
VOID CDriverInstall::StrRight(LPSTR szData, WORD wCount)
{
	WORD wLen = (WORD)strlen(szData) - wCount;

	if (wCount > 0x7FFF)//负数
		wCount = 0;
	if (wCount >= (WORD)strlen(szData))
		return;

	memmove(szData, szData + wLen, wCount * sizeof(char));
	szData[wCount] = 0;
}



//GUID 转字符串
VOID CDriverInstall::ConvertGUIDToString(const GUID guid, LPSTR pData)
{
	CHAR szData[30] = { 0 };
	CHAR szTmp[3] = { 0 };
	WORD wLoop;

	sprintf_s(pData, _countof(szData), "%04X-%02X-%02X-", guid.Data1, guid.Data2, guid.Data3);
	for (wLoop = 0; wLoop < 8; wLoop++)
	{
		if (wLoop == 2)
			strcat_s(szData, "-");
		sprintf_s(szTmp, _countof(szTmp), "%02X", guid.Data4[wLoop]);
		strcat_s(szData, szTmp);
	}

	memcpy(pData + strlen(pData), szData, strlen(szData));
}


//预备安装
BOOL CDriverInstall::StartInstallWDMDriver(LPCTSTR theInfName)
{
	HDEVINFO hDevInfo = 0L;
	GUID guid = { 0L };
	SP_DEVINSTALL_PARAMS spDevInst = { 0L };
	TCHAR strClass[32] = { 0L };

	//取得此驱动的GUID值
	if (!SetupDiGetINFClass(theInfName, &guid, strClass, 32, 0))
	{
		//ShowErrMsg(GetLastError(), _T("SetupDiGetINFClass"));
		return FALSE;
	}

	//得到设备信息结构的句柄
	hDevInfo = SetupDiGetClassDevs(&guid, 0L, 0L, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
	if (!hDevInfo)
	{
		//ShowErrMsg(GetLastError(), _T("SetupDiGetClassDevs"));
		return FALSE;
	}


	spDevInst.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
	//获得指定设备的安装信息
	if (!SetupDiGetDeviceInstallParams(hDevInfo, 0L, &spDevInst))
	{
		//ShowErrMsg(GetLastError(), _T("SetupDiGetDeviceInstallParams"));
		return FALSE;
	}

	spDevInst.Flags = DI_ENUMSINGLEINF;
	spDevInst.FlagsEx = DI_FLAGSEX_ALLOWEXCLUDEDDRVS;
	_tcscpy_s(spDevInst.DriverPath, _countof(spDevInst.DriverPath), theInfName);

	//为设备信息集或者是一个实际的设备信息单元设置或清除类安装参数
	if (!SetupDiSetDeviceInstallParams(hDevInfo, 0, &spDevInst))
	{
		//ShowErrMsg(GetLastError(), _T("SetupDiSetDeviceInstallParams"));
		return FALSE;
	}

	//获取这个设备的驱动程序信息列表
	if (!SetupDiBuildDriverInfoList(hDevInfo, 0, SPDIT_CLASSDRIVER))
	{
		//ShowErrMsg(GetLastError(), _T("SetupDiDeviceInstallParams"));
		return FALSE;
	}

	//销毁一个设备信息集合
	SetupDiDestroyDeviceInfoList(hDevInfo);

	//进入安装设备驱动函数
	return InstallClassDriver(theInfName);
}


//真正安装驱动
BOOL CDriverInstall::InstallClassDriver(LPCTSTR theINFName)
{
	GUID guid = { 0 };
	SP_DEVINFO_DATA spDevData = { 0 };
	HDEVINFO hDevInfo = 0L;
	TCHAR className[32] = { 0 };
	CHAR HID[32] = { 0 };
	LPSTR pHID = NULL;
	WORD wLoop;
	BOOL bRebootRequired;
	HMODULE newdevMod = NULL;

	//取得此驱动的GUID值
	if (!SetupDiGetINFClass(theINFName, &guid, className, 32, 0))
	{
		return FALSE;
	}

	//创建设备信息块列表
	hDevInfo = SetupDiCreateDeviceInfoList(&guid, 0);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	spDevData.cbSize = sizeof(SP_DEVINFO_DATA);
	//创建设备信息块
	if (!SetupDiCreateDeviceInfo(hDevInfo, className, &guid, 0L, 0L, DICD_GENERATE_ID, &spDevData))
	{
		//销毁一个设备信息集合
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return FALSE;
	}

	for (wLoop = 0; wLoop < g_wHardware; wLoop++)
	{
		if (pHID)
			LocalFree(pHID);

		pHID = (LPSTR)LocalAlloc(LPTR, _tcslen(g_strHardware[wLoop]) * 2 * sizeof(TCHAR));
		if (!pHID)
		{
			//销毁一个设备信息集合
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return FALSE;
		}

		_tcscpy_s(pHID, _tcslen(g_strHardware[wLoop]) * 2, g_strHardware[wLoop]);
		//UnicodeToAnsi(pHID, _tcslen(pHID), HID, 32);
		//设定硬件ID
		if (!SetupDiSetDeviceRegistryProperty(hDevInfo, &spDevData, SPDRP_HARDWAREID, (PBYTE)pHID,
			(DWORD)(_tcslen(g_strHardware[wLoop]) * 2 * sizeof(CHAR))))
		{
			//销毁一个设备信息集合
			SetupDiDestroyDeviceInfoList(hDevInfo);
			LocalFree(pHID);
			return FALSE;
		}
		//调用相应的类程序来注册设备
		if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfo, &spDevData))
		{
			//ShowErrMsg(GetLastError(), _T("SetupDiCallClassInstaller"));
			//销毁一个设备信息集合
			SetupDiDestroyDeviceInfoList(hDevInfo);
			LocalFree(pHID);
			return FALSE;
		}

		bRebootRequired = FALSE;
		//安装更新和硬件ID相匹配的驱动程序
		if (!UpdateDriverForPlugAndPlayDevices(0L, g_strHardware[wLoop], theINFName,
			INSTALLFLAG_FORCE, &bRebootRequired))
		{
			DWORD dwErrorCode = GetLastError();
			//调用相应的类程序来移除设备
			if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &spDevData))
				//ShowErrMsg(GetLastError(), _T("SetupDiCallClassInstaller(Remove)"));
			//ShowErrMsg((WORD)dwErrorCode, _T("UpdateDriverForPlugAndPlayDevices"));
			//销毁一个设备信息集合
			SetupDiDestroyDeviceInfoList(hDevInfo);
			LocalFree(pHID);
			return FALSE;
		}
		LocalFree(pHID);
		pHID = NULL;
	}
	//销毁一个设备信息集合
	SetupDiDestroyDeviceInfoList(hDevInfo);
	_tprintf(_T("Install Successed\n"));
	return TRUE;
}
//卸载驱动
VOID CDriverInstall::UninstallWdmDriver(LPCTSTR theHardWare)
{
	SP_DEVINFO_DATA spDevInfoData = { 0 };
	HDEVINFO hDevInfo = 0L;
	WORD wIdx, wCount = 0;
	hDevInfo = SetupDiGetClassDevs(0, 0, 0, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return;
	}
	wIdx = 0;
	while (TRUE)
	{
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiEnumDeviceInfo(hDevInfo, wIdx, &spDevInfoData))
		{
			char buffer[2048] = { 0 };
			if(SetupDiGetDeviceRegistryProperty(hDevInfo,&spDevInfoData,SPDRP_HARDWAREID,
				0,(PBYTE)buffer,2048,0));
			{
				if (!_tcscmp(theHardWare, (LPTSTR)buffer))
				{
					if (!SetupDiRemoveDevice(hDevInfo, &spDevInfoData))
					{

					}
					wCount++;
				}
			}
		}
		else
			break;
		wIdx++;
	}
	if (wCount != 0)
	{
		printf("uninstall success\n");
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	InitialGlobalVar();
	return;
}


//打印错误
//void CDriverInstall::ShowErrMsg(DWORD Count, LPCTSTR szMsg)
//{
//	printf("%d\n%s", &Count, szMsg);
//}
