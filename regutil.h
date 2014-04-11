#pragma once

HRESULT GetRegStringAsFloat(char *szKey, char *szValue, float *pfloat);
HRESULT GetRegString(char *szKey, char *szValue, void *szbuffer, DWORD size);
HRESULT GetRegInt(char *szKey, char *szValue, int *pint);
HRESULT GetRegValue(char *szKey, char *szValue, DWORD *ptype, void *pvalue, DWORD size);

int GetRegIntWithDefault(const char *szKey, const char *szValue, int def);
float GetRegStringAsFloatWithDefault(const char *szKey, const char *szValue, float def);

HRESULT SetRegValue(char *szKey, char *szValue, DWORD type, void *pvalue, DWORD size);
