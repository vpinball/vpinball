// license:GPLv3+

#pragma once


// Multiple Document Interface child window that contains a PinTable view
class PinTableMDI final : public CMDIChild
{
public:
   PinTableMDI(VPinball *vpinball);
   ~PinTableMDI()
   #ifndef __STANDALONE__
   override
   #endif
   ;
   CComObject<PinTable> *GetTable() { return m_table; }
   bool CanClose() const;

protected:
   void PreCreate(CREATESTRUCT &cs) override;
   int OnCreate(CREATESTRUCT &cs) override;
   void OnClose() override;
   LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam) override;
   BOOL OnEraseBkgnd(CDC &dc) override;

private:
   CComObject<PinTable> *m_table;
   VPinball *m_vpinball;
};
