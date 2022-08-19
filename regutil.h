#pragma once

void SaveXMLregistry(const string &path);
void InitXMLregistry(const string &path);
void ClearXMLregistry(); // clears self-allocated strings


HRESULT LoadValue(const string &szKey, const string &zValue, string &buffer);
HRESULT LoadValue(const string &szKey, const string &szValue, void* const szbuffer, const DWORD size);

HRESULT LoadValue(const string &szKey, const string &szValue, float &pfloat);
float   LoadValueFloatWithDefault(const string &szKey, const string &szValue, const float def); 

HRESULT LoadValue(const string &szKey, const string &szValue, int &pint);
HRESULT LoadValue(const string &szKey, const string &szValue, unsigned int &pint);
int     LoadValueIntWithDefault(const string &szKey, const string &szValue, const int def);

bool    LoadValueBoolWithDefault(const string &szKey, const string &szValue, const bool def);


HRESULT SaveValue(const string &szKey, const string &szValue, const char *val);
HRESULT SaveValue(const string &szKey, const string &szValue, const string &val);
HRESULT SaveValueFloat(const string &szKey, const string &szValue, const float val);
HRESULT SaveValueInt(const string &szKey, const string &szValue, const int val);
HRESULT SaveValueBool(const string &szKey, const string &szValue, const bool val);

HRESULT DeleteValue(const string &szKey, const string &szValue);
HRESULT DeleteSubKey(const string &szKey);
