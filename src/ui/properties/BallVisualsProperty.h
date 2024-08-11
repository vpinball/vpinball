// license:GPLv3+

#pragma once

class BallVisualsProperty : public BasePropertyDialog
{
public:
   BallVisualsProperty(const VectorProtected<ISelect> *pvsel);
   virtual ~BallVisualsProperty() { }

   void UpdateVisuals(const int dispid = -1);
   void UpdateProperties(const int dispid);

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
