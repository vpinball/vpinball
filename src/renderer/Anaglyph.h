// license:GPLv3+

#pragma once

#include "Shader.h"

class Anaglyph final
{
public:
   Anaglyph();

   enum Filter
   {
      NONE,
      DUBOIS,
      LUMINANCE,
      DEGHOST
   };

   enum AnaglyphPair
   {
      RED_CYAN,
      GREEN_MAGENTA,
      BLUE_AMBER
   };

   void LoadSetupFromRegistry(const int set);
   void SetLuminanceCalibration(const vec3& leftLum, const vec3& rightLum);
   void SetPhotoCalibration(const Matrix3& display, const Matrix3& leftFilter, const Matrix3& rightFilter);
   void SetupShader(Shader* shader);

   float GetDisplayGamma() const { return m_displayGamma; }
   AnaglyphPair GetColorPair() const { return m_colorPair; }
   bool IsReversedColorPair() const { return m_reversedColorPair; }
   vec3 GetLeftEyeColor(const bool linear) const { return linear ? m_leftEyeColor : Gamma(m_leftEyeColor); }
   vec3 GetRightEyeColor(const bool linear) const { return linear ? m_rightEyeColor : Gamma(m_rightEyeColor); }
   vec3 GetAnaglyphRatio() const { return m_anaglyphRatio; }

private:
   void Update();
   vec3 Gamma(const vec3& rgb) const;
   vec3 InvGamma(const vec3& rgb) const;
   vec3 LinearRGBtoXYZ(const vec3& linearRGB) const;

private:
   // User settings
   bool m_sRGBDisplay = true;
   float m_brightness = 1.f;
   float m_saturation = 1.f;
   float m_leftEyeContrast = 1.f;
   float m_rightEyeContrast = 1.f;
   float m_deghostLevel = 0.f;
   float m_dynDesatLevel = 0.f;
   Filter m_filter = Filter::NONE;

   // Data computed from calibration data
   vec3 m_rgb2Yl, m_rgb2Yr, m_anaglyphRatio;
   float m_displayGamma = 2.4f;
   AnaglyphPair m_colorPair = AnaglyphPair::RED_CYAN;
   bool m_reversedColorPair = false;
   vec3 m_leftEyeColor = vec3(1.f, 0.f, 0.f);
   vec3 m_rightEyeColor = vec3(0.f, 1.f, 1.f);

   // Render information
   vec3 m_deghostGamma;
   Matrix3D m_deghostFilter;
   Matrix3D m_rgb2AnaglyphLeft, m_rgb2AnaglyphRight;
};
