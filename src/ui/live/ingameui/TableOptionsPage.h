// license:GPLv3+

#pragma once


namespace VPX::InGameUI
{

class TableOptionsPage : public InGameUIPage
{
public:
   TableOptionsPage();

   void Open() override;

private:
   int GetOptionIndex(const string& id) const;
};

};