// license:GPLv3+

#pragma once

class BallVisualsProperty final : public BasePropertyDialog
{
public:
   BallVisualsProperty(const VectorProtected<ISelect> *pvsel);
   ~BallVisualsProperty() override { }

   void UpdateVisuals(const int dispid = -1) override;
   void UpdateProperties(const int dispid) override;

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   ComboBox m_imageCombo;
   ComboBox m_decalImageCombo;
   EditBox m_bulbIntensityScaleEdit;
   EditBox m_pfReflectionStrengthEdit;
   CColorDialog m_colorDialog;
   ColorButton m_colorButton;
   HWND m_hUseTableSettings = NULL;
   HWND m_hForceReflectionCheck = NULL;
   HWND m_hDecalModeCheck = NULL;
   HWND m_hSphericalCheck = NULL;
   EditBox m_posXEdit;
   EditBox m_posYEdit;
   EditBox m_posZEdit;
   EditBox m_radiusEdit;
};
