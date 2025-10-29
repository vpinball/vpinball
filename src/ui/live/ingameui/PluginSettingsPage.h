// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class PluginHomePage final : public InGameUIPage
{
public:
   PluginHomePage();
   void Open(bool isBackwardAnimation) override;
};

class PluginSettingsPage final : public InGameUIPage
{
public:
   PluginSettingsPage(const string& pluginId);
   void Open(bool isBackwardAnimation) override;

   void BuildPage();

private:
   const string m_pluginId;
};

}
