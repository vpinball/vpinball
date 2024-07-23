#pragma once

#include "../b2s_i.h"

#include "B2SBaseBox.h"

class B2SPictureBox : public B2SBaseBox
{
public:
   B2SPictureBox();

   ePictureBoxType GetPictureBoxType() const { return m_pictureBoxType; }
   void SetPictureBoxType(ePictureBoxType pictureBoxType) { m_pictureBoxType = pictureBoxType; }
   string GetGroupName() const { return m_szGroupName; }
   void SetGroupName(string szGroupName) { m_szGroupName = szGroupName; }
   int GetIntensity() const { return m_intensity; }
   void SetIntensity(int intensity) { m_intensity = intensity; }
   int GetInitialState() const { return m_initialState; }
   void SetInitialState(int initialstate) { m_initialState = initialstate; }
   eDualMode GetDualMode() const { return m_dualMode; }
   void SetDualMode(eDualMode dualMode) { m_dualMode = dualMode; }
   int GetZOrder() const { return m_zorder; }
   void SetZOrder(int zorder) { m_zorder = zorder; }
   bool IsImageSnippit() const { return m_imageSnippit; }
   void SetImageSnippit(bool imageSnippit) { m_imageSnippit = imageSnippit; }
   eSnippitRotationStopBehaviour GetSnippitRotationStopBehaviour() const { return m_snippitRotationStopBehaviour; }
   void SetSnippitRotationStopBehaviour(eSnippitRotationStopBehaviour snippitRotationStopBehaviour) { m_snippitRotationStopBehaviour = snippitRotationStopBehaviour; }
   void SetVisible(bool visible) override { if (IsVisible() != visible) { m_setThruAnimation = visible; B2SBaseBox::SetVisible(visible); } };
   bool IsSetThruAnimation() const { return m_setThruAnimation; }
   SDL_Surface* GetOffImage() const { return m_pOffImage; }
   void SetOffImage(SDL_Surface* pOffImage) { m_pOffImage = pOffImage; }

private:
   ePictureBoxType m_pictureBoxType;
   string m_szGroupName;
   int m_intensity;
   int m_initialState;
   eDualMode m_dualMode;
   int m_zorder;
   bool m_imageSnippit;
   eSnippitRotationStopBehaviour m_snippitRotationStopBehaviour;
   bool m_setThruAnimation;
   SDL_Surface* m_pOffImage;
};
