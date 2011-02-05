#include "StdAfx.h"

HRESULT GetRegString(char *szKey, char *szValue, void *szbuffer, DWORD size)
	{
	DWORD type;
	HRESULT hr;

	hr = GetRegValue(szKey, szValue, &type, szbuffer, size);

	if (type != REG_SZ)
		{
		return E_FAIL;
		}

	return hr;
	}

HRESULT GetRegStringAsFloat(char *szKey, char *szValue, float *pfloat)
	{
	DWORD type;
	char szbuffer[16];
	const HRESULT hr = GetRegValue(szKey, szValue, &type, &szbuffer[0], 16);

	if (type != REG_SZ)
		{
		return E_FAIL;
		}

	const size_t len = strlen(szbuffer);
	if (len == 0)
		return E_FAIL;

	//todo: convert decimal character in string to match regional setting of current machine
	if (szbuffer[0] == '-')
	{
		if (len < 2)
			return E_FAIL;
		*pfloat = (float)atof(&szbuffer[1]);
		*pfloat *= -1.0f;
	}
	else
		*pfloat = (float)atof(&szbuffer[0]);
	
	return hr;
	}

HRESULT GetRegInt(char *szKey, char *szValue, int *pint)
	{
	DWORD type;
	HRESULT hr;

	hr = GetRegValue(szKey, szValue, &type, (void *)pint, 4);

	if (type != REG_DWORD)
		{
		return E_FAIL;
		}

	return hr;
	}

HRESULT GetRegValue(char *szKey, char *szValue, DWORD *ptype, void *pvalue, DWORD size)
	{
	DWORD RetVal;

	HKEY hk;

	char szPath[1024];

	lstrcpy(szPath, "Software\\Visual Pinball\\");
	lstrcat(szPath, szKey);

	RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);
	
	if(RetVal == ERROR_SUCCESS)
			{
			DWORD type;

			RetVal = RegQueryValueEx(hk, szValue, NULL, &type, (BYTE *)pvalue, &size);
			
			*ptype = type;

			RegCloseKey(hk);
			}
			
	return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;	
	}

HRESULT SetRegValue(char *szKey, char *szValue, DWORD type, void *pvalue, DWORD size)
	{
	DWORD RetVal;

	HKEY hk;

	char szPath[1024];

	lstrcpy(szPath, "Software\\Visual Pinball\\"); 

	lstrcat(szPath, szKey);

	//RetVal = RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_ALL_ACCESS, &hk);
	RetVal = RegCreateKeyEx(HKEY_CURRENT_USER, szPath, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, NULL);
	
	if(RetVal == ERROR_SUCCESS)
			{
			RetVal = RegSetValueEx(hk, szValue, 0, type, (BYTE *)pvalue, size);

			RegCloseKey(hk);
			}
			
	return (RetVal == ERROR_SUCCESS) ? S_OK : E_FAIL;	
	}
