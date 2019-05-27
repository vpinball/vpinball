#pragma once

HRESULT LoadValueString(const char * const szKey, const char * const szValue, void * const szbuffer, const DWORD size);

HRESULT LoadValueFloat(const char *szKey, const char *szValue, float *pfloat);
float   LoadValueFloatWithDefault(const char *szKey, const char *szValue, const float def); 

HRESULT LoadValueInt(const char *szKey, const char *szValue, int *pint);
int     LoadValueIntWithDefault(const char *szKey, const char *szValue, const int def);

bool    LoadValueBoolWithDefault(const char *szKey, const char *szValue, const bool def);


HRESULT SaveValueString(const char *szKey, const char *szValue, const char *val);
HRESULT SaveValueFloat(const char *szKey, const char *szValue, const float val);
HRESULT SaveValueInt(const char *szKey, const char *szValue, const int val);
HRESULT SaveValueBool(const char *szKey, const char *szValue, const bool val);
