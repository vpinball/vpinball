#pragma once

HRESULT LoadValueString(const std::string& szKey, const std::string& szValue, void* const szbuffer, const DWORD size);

HRESULT LoadValueFloat(const std::string &szKey, const std::string &szValue, float *pfloat);
float   LoadValueFloatWithDefault(const char *szKey, const char *szValue, const float def); 

HRESULT LoadValueInt(const std::string &szKey, const std::string &szValue, int *pint);
int     LoadValueIntWithDefault(const std::string &szKey, const std::string &szValue, const int def);

bool    LoadValueBoolWithDefault(const std::string &szKey, const std::string &szValue, const bool def);


HRESULT SaveValueString(const std::string &szKey, const std::string &szValue, const char *val);
HRESULT SaveValueString(const std::string &szKey, const std::string &szValue, const string& val);
HRESULT SaveValueFloat(const std::string &szKey, const std::string &szValue, const float val);
HRESULT SaveValueInt(const std::string &szKey, const std::string &szValue, const int val);
HRESULT SaveValueBool(const std::string &szKey, const std::string &szValue, const bool val);

HRESULT DeleteValue(const std::string &szKey, const std::string &szValue);
