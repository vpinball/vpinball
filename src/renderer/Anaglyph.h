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

   void LoadSetupFromRegistry(const Settings& settings, const int set);
   void SetLuminanceCalibration(const vec3& leftLum, const vec3& rightLum);
   void SetPhotoCalibration(const Matrix3& display, const Matrix3& leftFilter, const Matrix3& rightFilter);
   void SetupShader(Shader* shader) const;

   // Computes the anaglyph color for a single pixel on the CPU
   vec3 ComputeColor(const vec3& leftCol, const vec3& rightCol, bool isLinearInput = false) const;

   float GetDisplayGamma() const { return m_displayGamma; }
   AnaglyphPair GetColorPair() const { return m_colorPair; }
   bool IsReversedColorPair() const { return m_reversedColorPair; }
   vec3 GetLeftEyeGlassFilter(const bool linear) const { return linear ? m_leftEyeGlassFilter : Gamma(m_leftEyeGlassFilter); }
   vec3 GetRightEyeGlassFilter(const bool linear) const { return linear ? m_rightEyeGlassFilter : Gamma(m_rightEyeGlassFilter); }

private:
   void Update();
   vec3 Gamma(const vec3& rgb) const;
   vec3 InvGamma(const vec3& rgb) const;
   static vec3 LinearRGBtoXYZ(const vec3& linearRGB);

   // User settings
   bool m_sRGBDisplay = true;
   float m_brightness = 1.f;
   float m_saturation = 1.f;
   float m_leftEyeContrast = 1.f;
   float m_rightEyeContrast = 1.f;
   float m_deghostLevel = 0.f;
   float m_dynDesatLevel = 0.f;
   Filter m_filter = Filter::NONE;

   // Calibration data
   vec3 m_rgb2Yl; // Left eye normalized luminance transfer function in linear RGB
   vec3 m_rgb2Yr; // Right eye normalized luminance transfer function in linear RGB
   float m_displayGamma = 2.4f;

   // Data derived from calibration data
   AnaglyphPair m_colorPair = AnaglyphPair::RED_CYAN;
   bool m_reversedColorPair = false;
   vec3 m_leftEyeGlassFilter = vec3(1.f, 0.f, 0.f);
   vec3 m_rightEyeGlassFilter = vec3(0.f, 1.f, 1.f);

   // Render information
   vec3 m_deghostGamma;
   Matrix3D m_deghostFilter;
   Matrix3D m_rgb2AnaglyphLeft, m_rgb2AnaglyphRight;
};
