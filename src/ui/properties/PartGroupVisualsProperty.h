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
   void UpdatePlayerModeVisibilityMask(PartGroup* const partGroup, PartGroupData::PlayerModeVisibilityMask mask, bool checked);
   void UpdateViewVisibilityMask(PartGroup* const partGroup, PartGroupData::ViewVisibilityMask mask, bool checked);

   CComboBox m_referenceSpace;
   CButton m_visibilityPlayfield;
   CButton m_visibilityDesktop;
   CButton m_visibilityFSS;
   CButton m_visibilityCabinet;
   CButton m_visibilityMixedReality;
   CButton m_visibilityVirtualReality;
};
