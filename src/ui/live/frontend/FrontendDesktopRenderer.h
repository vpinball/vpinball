// license:GPLv3+

#pragma once

#include "ui/live/ingameui/InGameUIDesktopRenderer.h"

namespace VPX::Frontend
{

// The mini frontend's UI chrome: the in-game UI desktop renderer over the table
// catalog base view (the wheel). The top bar mirrors the desktop chrome's
// arrangement, with GAMES standing in for GAME — it closes the UI back to the
// catalog (the base view) as GAME closes it back to play. TABLE lists the
// actions on the visible catalog entry (Play, Edit, media management, ...); SETTINGS,
// PLUGINS, ABOUT and QUIT are the
// desktop renderer's, fed by the frontend's navigation root page.
class FrontendDesktopRenderer final : public InGameUI::InGameUIDesktopRenderer
{
public:
   explicit FrontendDesktopRenderer(InGameUI::InGameUI &owner)
      : InGameUIDesktopRenderer(owner)
   {
   }

private:
   void BuildTopTabs(vector<InGameUI::DesktopTopTab> &tabs, const string &rootPath, const std::function<string(const string &)> &firstPathOf) override;
   string NavGroup(const string &path) const override;
   string NavRootGroup(const string &rootPath) const override;
   const char *NavRootRailLabel(const string &group) const override;
   const char *NavRootPageTitle() const override;
};

}
