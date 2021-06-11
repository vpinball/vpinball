#pragma once

HRESULT LoadValue(const std::string &szKey, const std::string &zValue, std::string &buffer);
HRESULT LoadValue(const std::string &szKey, const std::string &szValue, void* const szbuffer, const DWORD size);

HRESULT LoadValue(const std::string &szKey, const std::string &szValue, float &pfloat);
float   LoadValueFloatWithDefault(const char *szKey, const char *szValue, const float def); 

HRESULT LoadValue(const std::string &szKey, const std::string &szValue, int &pint);
HRESULT LoadValue(const std::string &szKey, const std::string &szValue, unsigned int &pint);
int     LoadValueIntWithDefault(const std::string &szKey, const std::string &szValue, const int def);

bool    LoadValueBoolWithDefault(const std::string &szKey, const std::string &szValue, const bool def);


HRESULT SaveValue(const std::string &szKey, const std::string &szValue, const char *val);
HRESULT SaveValue(const std::string &szKey, const std::string &szValue, const std::string &val);
HRESULT SaveValueFloat(const std::string &szKey, const std::string &szValue, const float val);
HRESULT SaveValueInt(const std::string &szKey, const std::string &szValue, const int val);
HRESULT SaveValueBool(const std::string &szKey, const std::string &szValue, const bool val);

HRESULT DeleteValue(const std::string &szKey, const std::string &szValue);
