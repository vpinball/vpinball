// license:GPLv3+

#include "Settings.h"

namespace PinMAME {

Settings::Settings()
{

}

Settings::~Settings()
{

}

int Settings::GetValue(const string& key) const
{
   const string k = string_to_lower(key);
   const auto it = m_values.find(k);
   if (it != m_values.end())
      return it->second;
   int v;
   if (TryGetSettingDefault(k, v))
      return v;
   LOGW(std::format("Unknown setting '{}', returning 0", key));
   return 0;
}

void Settings::SetValue(const string& key, int v)
{
   const string k = string_to_lower(key);
   m_values[k] = v;
}

}
