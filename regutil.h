#pragma once

HRESULT GetRegString(char *szKey, char *szValue, void *szbuffer, DWORD size);
HRESULT GetRegInt(char *szKey, char *szValue, int *pint);
HRESULT GetRegValue(char *szKey, char *szValue, DWORD *ptype, void *pvalue, DWORD size);

HRESULT SetRegValue(char *szKey, char *szValue, DWORD type, void *pvalue, DWORD size);
