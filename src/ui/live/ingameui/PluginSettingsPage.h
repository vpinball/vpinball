// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class PluginHomePage final : public InGameUIPage
{
public:
   PluginHomePage();

private:
   void BuildPage() override;
};

class PluginSettingsPage final : public InGameUIPage
{
public:
   PluginSettingsPage(const string& pluginId);

   void BuildPage() override;

private:
   const string m_pluginId;
};

}
