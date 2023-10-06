#include "stdafx.h"
#include "Anaglyph.h"

Anaglyph::Anaglyph()
{
}

void Anaglyph::LoadSetupFromRegistry(const int glassesSet)
{
   // Common settings for all anaglyph sets
   m_brightness = LoadValueWithDefault(regKey[RegName::Player], "Stereo3DBrightness"s, 1.0f);
   m_saturation = LoadValueWithDefault(regKey[RegName::Player], "Stereo3DSaturation"s, 1.f);
   m_leftEyeContrast = LoadValueWithDefault(regKey[RegName::Player], "Stereo3DLeftContrast"s, 1.0f);
   m_rightEyeContrast = LoadValueWithDefault(regKey[RegName::Player], "Stereo3DRightContrast"s, 1.0f);

   // Default (partial) calibration
   static const vec3 defaultColors[] = {
      /* RC */ vec3(0.95f, 0.19f, 0.07f), vec3(0.06f, 0.92f, 0.28f),
      /* GM */ vec3(0.06f, 0.96f, 0.09f), vec3(0.61f, 0.16f, 0.66f),
      /* BA */ vec3(0.05f, 0.16f, 0.96f), vec3(0.61f, 0.66f, 0.09f),
      /* CR */ vec3(0.06f, 0.92f, 0.28f), vec3(0.95f, 0.19f, 0.07f),
      /* MG */ vec3(0.61f, 0.16f, 0.66f), vec3(0.06f, 0.96f, 0.09f),
      /* AB */ vec3(0.61f, 0.66f, 0.09f), vec3(0.05f, 0.16f, 0.96f),
      /* RC */ vec3(0.95f, 0.19f, 0.07f), vec3(0.06f, 0.92f, 0.28f),
      /* RC */ vec3(0.95f, 0.19f, 0.07f), vec3(0.06f, 0.92f, 0.28f),
      /* RC */ vec3(0.95f, 0.19f, 0.07f), vec3(0.06f, 0.92f, 0.28f),
      /* RC */ vec3(0.95f, 0.19f, 0.07f), vec3(0.06f, 0.92f, 0.28f),
   };

   const string prefKey = "Anaglyph"s.append(std::to_string(glassesSet + 1));
   vec3 leftLum, rightLum;
   m_filter = (Filter)LoadValueWithDefault(regKey[RegName::Player], prefKey + "Filter"s, 2);
   m_sRGBDisplay = LoadValueWithDefault(regKey[RegName::Player], prefKey + "sRGB"s, true);
   m_dynDesatLevel = LoadValueWithDefault(regKey[RegName::Player], prefKey + "DynDesat"s, 1.f);
   m_deghostLevel = LoadValueWithDefault(regKey[RegName::Player], prefKey + "Deghost"s, 0.f);
   leftLum.x = LoadValueWithDefault(regKey[RegName::Player], prefKey + "LeftRed"s, -1.f);
   leftLum.y = LoadValueWithDefault(regKey[RegName::Player], prefKey + "LeftGreen"s, -1.f);
   leftLum.z = LoadValueWithDefault(regKey[RegName::Player], prefKey + "LeftBlue"s, -1.f);
   rightLum.x = LoadValueWithDefault(regKey[RegName::Player], prefKey + "RightRed"s, -1.f);
   rightLum.y = LoadValueWithDefault(regKey[RegName::Player], prefKey + "RightGreen"s, -1.f);
   rightLum.z = LoadValueWithDefault(regKey[RegName::Player], prefKey + "RightBlue"s, -1.f);
   if (leftLum.x < 0 || leftLum.y < 0 || leftLum.z || rightLum.x < 0 || rightLum.y < 0 || rightLum.z < 0)
      SetLuminanceCalibration(defaultColors[glassesSet * 2], defaultColors[glassesSet * 2 + 1]);
   else
      SetLuminanceCalibration(leftLum, rightLum);

   Update();
}

void Anaglyph::SetupShader(Shader* shader)
{
   // Main matrices to project from linear rgb to anaglyph
   shader->SetMatrix(m_reversedColorPair ? SHADER_Stereo_RightMat : SHADER_Stereo_LeftMat, &m_rgb2AnaglyphLeft);
   shader->SetMatrix(m_reversedColorPair ? SHADER_Stereo_LeftMat : SHADER_Stereo_RightMat, &m_rgb2AnaglyphRight);
   
   // Used by the dynamic desaturation filter to identify colors that would be seen by only one eye
   shader->SetVector(SHADER_Stereo_LeftLuminance_Gamma, 0.5f * m_rgb2Yl.x, 0.5f * m_rgb2Yl.y, 0.5f * m_rgb2Yl.z, m_sRGBDisplay ? -1.f : m_displayGamma);
   shader->SetVector(SHADER_Stereo_RightLuminance_DynDesat, 0.5f * m_rgb2Yr.x, 0.5f * m_rgb2Yr.y, 0.5f * m_rgb2Yr.z, m_dynDesatLevel);

   // Used by Deghost filter
   shader->SetVector(SHADER_Stereo_DeghostGamma, m_deghostGamma.x, m_deghostGamma.y, m_deghostGamma.z, 0.00f);
   shader->SetMatrix(SHADER_Stereo_DeghostFilter, &m_deghostFilter);

   // Select the shader based on the filter, dynamic desaturation and the gamma mode
   if (m_filter == DEGHOST)
      shader->SetTechnique(SHADER_TECHNIQUE_Stereo_DeghostAnaglyph);
   else if (m_dynDesatLevel > 0.f)
      shader->SetTechnique(m_sRGBDisplay ? SHADER_TECHNIQUE_Stereo_sRGBDynDesatAnaglyph : SHADER_TECHNIQUE_Stereo_GammaDynDesatAnaglyph);
   else
      shader->SetTechnique(m_sRGBDisplay ? SHADER_TECHNIQUE_Stereo_sRGBAnaglyph : SHADER_TECHNIQUE_Stereo_GammaAnaglyph);
}

vec3 Anaglyph::Gamma(const vec3& rgb)
{
   #define sRGB(x) ((x <= 0.0031308f) ? (12.92f * x) : (1.055f * powf(x, 1.0f / 2.4f) - 0.055f))
   if (m_sRGBDisplay)
      return vec3(sRGB(rgb.x), sRGB(rgb.y), sRGB(rgb.z));
   else
      return vec3(powf(rgb.x, 1.f / m_displayGamma), powf(rgb.y, 1.f / m_displayGamma), powf(rgb.z, 1.f / m_displayGamma));
   #undef sRGB
}

vec3 Anaglyph::InvGamma(const vec3& rgb)
{
   #define InvsRGB(x) ((x <= 0.04045f) ? (x * (1.0f / 12.92f)) : (powf(x * (1.0f / 1.055f) + (0.055f / 1.055f), 2.4f)))
   if (m_sRGBDisplay)
      return vec3(InvsRGB(rgb.x), InvsRGB(rgb.y), InvsRGB(rgb.z));
   else
      return vec3(powf(rgb.x, m_displayGamma), powf(rgb.y, m_displayGamma), powf(rgb.z, m_displayGamma));
   #undef InvsRGB
}

void Anaglyph::SetLuminanceCalibration(const vec3& leftLum, const vec3& rightLum)
{
   // Use calibration data to fit the luminance model proposed in https://www.visus.uni-stuttgart.de/en/research/computer-graphics/anaglyph-stereo/anaglyph-stereo-without-ghosting/
   // If user has a calibrated sRGB display, gamma calibration is discarded

   // Evaluate screen gamma from calibration data using Newton's method: https://en.wikipedia.org/wiki/Newton%27s_method
   // The calibration model compare a RGB color to a gray channel to fit a luminance model: Y = aR^g + bG^g + cB^g
   // For a gray value where each channel have the value V, this gives: Y = (a+b+c)V^g
   // This gray luminance matches a reference where a single channel is 1 and others 0: Y = a or b or c
   // Therefore, for each eye, (Vr, Vg, Vb) being the calibration measure, we have Vr^g + Vg^g + Vb^g = 1
   // We solve this for the 2 eyes at once by resolving the sum function (the right way would be to use the generalized jacobian, but...)
   if (!m_sRGBDisplay)
   {
      float gamma = 2.4f;
      for (int i = 0; i < 100; i++)
      {
         float f = powf(leftLum.x, gamma) + powf(leftLum.y, gamma) + powf(leftLum.z, gamma) - 1
                 + powf(rightLum.x, gamma) + powf(rightLum.y, gamma) + powf(rightLum.z, gamma) - 1;
         float fp = powf(leftLum.x, gamma) / logf(leftLum.x) + powf(leftLum.y, gamma) / logf(leftLum.y) + powf(leftLum.z, gamma) / logf(leftLum.z)
                  + powf(rightLum.x, gamma) / logf(rightLum.x) + powf(rightLum.y, gamma) / logf(rightLum.y) + powf(rightLum.z, gamma) / logf(rightLum.z);
         gamma -= f / fp;
      }
      m_displayGamma = gamma;
   }

   // Evaluate relative luminance of each color channel through eye filters (needed for dynamic desaturation and luminance filter)
   // As seen before, for a single calibration, we have: aR^g + bG^g + cB^g = aV^g + bV^g + cV^g
   // This gives a = (a+b+c)Vr^g, b = (a+b+c)Vg^g, c = (a+b+c)Vb^g
   // Therefore the following vectors should have x+y+z = 1, this is enforced to account for calibration errors
   m_rgb2Yl = InvGamma(leftLum);
   m_rgb2Yr = InvGamma(rightLum);
   m_rgb2Yl = m_rgb2Yl / (m_rgb2Yl.x + m_rgb2Yl.y + m_rgb2Yl.z);
   m_rgb2Yr = m_rgb2Yr / (m_rgb2Yr.x + m_rgb2Yr.y + m_rgb2Yr.z);

   // Evaluate filter tints
   vec3 eyeL(m_rgb2Yl.x / 0.2126f, m_rgb2Yl.y / 0.7152f, m_rgb2Yl.z / 0.0722f);
   vec3 eyeR(m_rgb2Yr.x / 0.2126f, m_rgb2Yr.y / 0.7152f, m_rgb2Yr.z / 0.0722f);
   eyeL = eyeL / max(max(eyeL.x, eyeL.y), eyeL.z);
   eyeR = eyeR / max(max(eyeR.x, eyeR.y), eyeR.z);
   m_leftEyeColor = eyeL;
   m_rightEyeColor = eyeR;

   // Identify the bichromatic eye with its color from the luminance calibration: it is the one with the higher second channel
   const float leftSecondHigher   = (eyeL.y > eyeL.x && eyeL.x > eyeL.z) ? eyeL.x : (eyeL.y < eyeL.x && eyeL.x < eyeL.z) ? eyeL.x : 
                                    (eyeL.x > eyeL.y && eyeL.y > eyeL.z) ? eyeL.y : (eyeL.x < eyeL.y && eyeL.y < eyeL.z) ? eyeL.y : 
                                    (eyeL.x > eyeL.z && eyeL.z > eyeL.y) ? eyeL.z : (eyeL.x < eyeL.z && eyeL.z < eyeL.y) ? eyeL.z : -1.f;
   const float rightSecondHigher  = (eyeR.y > eyeR.x && eyeR.x > eyeR.z) ? eyeR.x : (eyeR.y < eyeR.x && eyeR.x < eyeR.z) ? eyeR.x : 
                                    (eyeR.x > eyeR.y && eyeR.y > eyeR.z) ? eyeR.y : (eyeR.x < eyeR.y && eyeR.y < eyeR.z) ? eyeR.y : 
                                    (eyeR.x > eyeR.z && eyeR.z > eyeR.y) ? eyeR.z : (eyeR.x < eyeR.z && eyeR.z < eyeR.y) ? eyeR.z : -1.f;
   m_reversedColorPair = leftSecondHigher > rightSecondHigher; // Monochromatic (red/green/blue) is supposed to be on the left eye
   const vec3 eyeMono = m_reversedColorPair ? eyeR : eyeL;
   m_colorPair = (eyeMono.x > eyeMono.y && eyeMono.x > eyeMono.z) ? RED_CYAN
               : (eyeMono.y > eyeMono.x && eyeMono.y > eyeMono.z) ? GREEN_MAGENTA
                                                                  : BLUE_AMBER;
   Update();
}

void Anaglyph::Update()
{
   switch (m_filter)
   {
   case NONE:
   {
      // Basic anaglyph composition (only for reference, all other filters will always perform better)
      switch (m_colorPair)
      {
      case RED_CYAN:
         m_rgb2AnaglyphLeft = Matrix3D(1.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ 0.f, 1.f, 0.f, 0.f, /**/ 0.f, 0.f, 1.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      case GREEN_MAGENTA:
         m_rgb2AnaglyphLeft = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ 0.f, 1.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(1.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 1.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      case BLUE_AMBER:
         m_rgb2AnaglyphLeft = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 1.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(1.f, 0.f, 0.f, 0.f, /**/ 0.f, 1.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      }
      break;
   }

   case DUBOIS:
   {
      // Compose anaglyph by applying Dubois filter (pre-fitted filters on theoretical display / glasses / viewer set)
      // see https://www.site.uottawa.ca/~edubois/anaglyph/
      switch (m_colorPair)
      {
      case RED_CYAN:
         m_rgb2AnaglyphLeft = Matrix3D(0.437f, 0.449f, 0.164f, 0.f, /**/ -0.062f, -0.062f, -0.024f, 0.f, /**/ -0.048f, -0.050f, -0.017f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(-0.011f, -0.032f, -0.007f, 0.f, /**/ 0.377f, 0.761f, 0.009f, 0.f, /**/ -0.026f, -0.093f, 1.234f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      case GREEN_MAGENTA:
         m_rgb2AnaglyphLeft = Matrix3D(-0.062f, -0.158f, -0.039f, 0.f, /**/ 0.284f, 0.668f, 0.143f, 0.f, /**/ -0.015f, -0.027f, 0.021f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(0.529f, 0.705f, 0.024f, 0.f, /**/ -0.016f, -0.015f, 0.065f, 0.f, /**/ 0.009f, 0.075f, 0.937f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      case BLUE_AMBER:
         m_rgb2AnaglyphLeft = Matrix3D(-0.016f, -0.123f, -0.017f, 0.f, /**/ 0.006f, 0.062f, -0.017f, 0.f, /**/ 0.094f, 0.185f, 0.911f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         m_rgb2AnaglyphRight = Matrix3D(1.062f, -0.205f, 0.299f, 0.f, /**/ -0.026f, 0.908f, 0.068f, 0.f, /**/ -0.038f, -0.173f, 0.022f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         break;
      }
      break;
   }

   case LUMINANCE:
   {
      // Compose anaglyph base on measured luminance of display/filter/user by calibration
      // see https://www.visus.uni-stuttgart.de/en/research/computer-graphics/anaglyph-stereo/anaglyph-stereo-without-ghosting/
      vec3 chromacity;
      switch (m_colorPair)
      {
      case RED_CYAN: chromacity = vec3(0.f, 1.f, -1.f); break;
      case GREEN_MAGENTA: chromacity = vec3(-1.f, 0.f, 1.f); break;
      case BLUE_AMBER: chromacity = vec3(-1.f, 1.f, 0.f); break;
      }
      vec3 rgb2Yl(m_reversedColorPair ? m_rgb2Yr : m_rgb2Yl);
      vec3 rgb2Yr(m_reversedColorPair ? m_rgb2Yl : m_rgb2Yr);
      Matrix3D matYYC2RGB;
      matYYC2RGB.SetIdentity();
      matYYC2RGB.m[0][0] = rgb2Yl.x;
      matYYC2RGB.m[0][1] = rgb2Yl.y;
      matYYC2RGB.m[0][2] = rgb2Yl.z;
      matYYC2RGB.m[1][0] = rgb2Yr.x;
      matYYC2RGB.m[1][1] = rgb2Yr.y;
      matYYC2RGB.m[1][2] = rgb2Yr.z;
      matYYC2RGB.m[2][0] = chromacity.x;
      matYYC2RGB.m[2][1] = chromacity.y;
      matYYC2RGB.m[2][2] = chromacity.z;
      matYYC2RGB.Invert();
      Matrix3D matLeft2YYC, matRight2YYC;
      matLeft2YYC = Matrix3D(rgb2Yl.x, rgb2Yl.y, rgb2Yl.z, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
      matRight2YYC = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ rgb2Yr.x, rgb2Yr.y, rgb2Yr.z, 0.f, /**/ chromacity.x, chromacity.y, chromacity.z, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
      m_rgb2AnaglyphLeft = matYYC2RGB * matLeft2YYC;
      m_rgb2AnaglyphRight = matYYC2RGB * matRight2YYC;
      break;
   }

   case DEGHOST:
   {
      // Compose anaglyph by applying John Einselen's contrast and deghosting method
      // see http://iaian7.com/quartz/AnaglyphCompositing & vectorform.com
      constexpr float contrast = 1.f; 
      if (m_colorPair == RED_CYAN)
      {
         const float a = 0.45f * contrast, b = 0.5f * (1.f - a);
         m_rgb2AnaglyphLeft = Matrix3D(a, b, b, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         const float c = 1.00f * contrast, d = 1.f - c;
         m_rgb2AnaglyphRight = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ d, c, 0.f, 0.f, /**/ d, 0.f, c, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         constexpr float e = 0.06f * 0.1f;
         m_deghostGamma = vec3(1.00f, 1.15f, 1.15f);
         m_deghostFilter = Matrix3D(1.f + e, -0.5f*e, -0.5f*e, 0.f, /**/ -0.25f*e, 1.f + 0.5f*e, -0.25f*e, 0.f, /**/ -0.25f*e, -0.25f*e, 1.f + 0.5f*e, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
      }
      else if (m_colorPair == GREEN_MAGENTA)
      {
         const float a = 1.00f * contrast, b = 0.5f * (1.f - a);
         m_rgb2AnaglyphLeft = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ b, a, b, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         const float c = 0.80f * contrast, d = 1.f - c;
         m_rgb2AnaglyphRight = Matrix3D(c, d, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, d, c, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         constexpr float e = 0.06f * 0.275f;
         m_deghostGamma = vec3(1.15f, 1.05f, 1.15f);
         m_deghostFilter = Matrix3D(1.f + 0.5f*e, -0.25f*e, -0.25f*e, 0.f, /**/ -0.5f*e, 1.f + 0.25f*e, -0.5f*e, 0.f, /**/ -0.25f*e, -0.25f*e, 1.f + 0.5f*e, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
      }
      else if (m_colorPair == BLUE_AMBER)
      {
         const float a = 0.45f * contrast, b = 0.5f * (1.f - a);
         m_rgb2AnaglyphLeft = Matrix3D(0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ b, b, a, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         const float c = 1.00f * contrast, d = 1.f - c;
         m_rgb2AnaglyphRight = Matrix3D(c, 0.f, d, 0.f, /**/ 0.f, c, d, 0.f, /**/ 0.f, 0.f, 0.f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
         constexpr float e = 0.06f * 0.275f;
         m_deghostGamma = vec3(1.05f, 1.10f, 1.00f);
         m_deghostFilter = Matrix3D(1.f + 1.5f*e, -0.75f*e, -0.75f*e, 0.f, /**/ -0.75f*e, 1.f + 1.5f*e, -0.75f*e, 0.f, /**/ -1.5f*e, -1.5f*e, 1.f + 3.f*e, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
      }
      m_deghostFilter.Transpose();
      break;
   }
   }
      
   // Common parameters for all anaglyph composition filters
   const Matrix3D matLeftContrast = Matrix3D::MatrixTranslate(-0.5f, -0.5f, -0.5f) * Matrix3D::MatrixScale(m_reversedColorPair ? m_rightEyeContrast : m_leftEyeContrast) * Matrix3D::MatrixTranslate(0.5f, 0.5f, 0.5f);
   const Matrix3D matRightContrast = Matrix3D::MatrixTranslate(-0.5f, -0.5f, -0.5f) * Matrix3D::MatrixScale(m_reversedColorPair ? m_leftEyeContrast : m_rightEyeContrast) * Matrix3D::MatrixTranslate(0.5f, 0.5f, 0.5f);
   const Matrix3D matGrayscale = Matrix3D(0.212655f, 0.715158f, 0.072187f, 0.f, /**/ 0.212655f, 0.715158f, 0.072187f, 0.f, /**/ 0.212655f, 0.715158f, 0.072187f, 0.f, /**/ 0.f, 0.f, 0.f, 1.f);
   const Matrix3D matSaturation = Matrix3D::MatrixScale(1.f - m_saturation) * matGrayscale + Matrix3D::MatrixScale(m_saturation);
   const Matrix3D matBrightness = Matrix3D::MatrixScale(m_brightness); 
   m_rgb2AnaglyphLeft = matBrightness * matLeftContrast * m_rgb2AnaglyphLeft * matSaturation;
   m_rgb2AnaglyphRight = matBrightness * matRightContrast * m_rgb2AnaglyphRight * matSaturation;
   m_rgb2AnaglyphLeft.Transpose();
   m_rgb2AnaglyphRight.Transpose();

   // Adjust colors before processing in order to avoid needing to clamp after applying anaglyph matrices (since clamping always results in ghosting)
   // This works well but makes the image brighter and less contrasted.
   if (m_deghostLevel > 0.f)
   {
      float minCoef = 0.f, maxCoef = 1.f;
      for (int j = 0; j < 3; j++)
      {
         float rowMin = 0.f, rowMax = 0.f;
         for (int i = 0; i < 3; i++)
         {
            if (m_rgb2AnaglyphLeft.m[i][j] < 0)
               rowMin += m_rgb2AnaglyphLeft.m[i][j];
            else
               rowMax += m_rgb2AnaglyphLeft.m[i][j];
            if (m_rgb2AnaglyphRight.m[i][j] < 0)
               rowMin += m_rgb2AnaglyphRight.m[i][j];
            else
               rowMax += m_rgb2AnaglyphRight.m[i][j];
         }
         minCoef = min(minCoef, rowMin);
         maxCoef = max(maxCoef, rowMax);
      }
      const float contrast = 1.f / (maxCoef - minCoef);
      const float offset = -lerp(0.f, contrast * minCoef, m_deghostLevel);
      const Matrix3D matDeghost = Matrix3D::MatrixTranslate(offset, offset, offset) * Matrix3D::MatrixScale(lerp(1.f, contrast, m_deghostLevel));
      m_rgb2AnaglyphLeft = matDeghost * m_rgb2AnaglyphLeft;
      m_rgb2AnaglyphRight = matDeghost * m_rgb2AnaglyphRight;
   }
}