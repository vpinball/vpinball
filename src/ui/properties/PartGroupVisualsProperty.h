// license:GPLv3+

#pragma once

class PartGroupVisualsProperty final : public BasePropertyDialog
{
public:
    PartGroupVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~PartGroupVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   void UpdateVisibilityMask(PartGroup* const partGroup, PartGroupData::VisibilityMask mask, bool checked);

   ComboBox m_referenceSpace;
   CButton  m_visibilityPlayfield;
   CButton  m_visibilityMixedReality;
   CButton  m_visibilityVirtualReality;
};
