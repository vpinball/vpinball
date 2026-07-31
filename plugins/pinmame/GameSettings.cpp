// license:GPLv3+

#include "GameSettings.h"

namespace PinMAME
{

GameSettings::GameSettings() { }

GameSettings::~GameSettings() { }

void GameSettings::Clear() { m_values.clear(); }

int GameSettings::GetValue(const string& key) const
{
   const string k = string_to_lower(key);
   const auto it = m_values.find(k);
   if (it != m_values.end())
      return it->second;
   int v;
   if (TryGetSettingDefault(k, v))
      return v;
   LOGW(std::format("Unknown game setting '{}', returning 0", key));
   return 0;
}

void GameSettings::SetValue(const string& key, int v)
{
   const string k = string_to_lower(key);
   m_values[k] = v;
}

}
