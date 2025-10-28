// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class TableOptionsPage final : public InGameUIPage
{
public:
   TableOptionsPage();
   void Open(bool isBackwardAnimation) override;

private:
   const PinTable::TableOption* GetOption(VPX::Properties::PropertyRegistry::PropId id) const;
};

}
