// license:GPLv3+

#include "core/stdafx.h"
#include "Anaglyph.h"

Anaglyph::Anaglyph()
{
}

void Anaglyph::LoadSetupFromRegistry(const int glassesSet)
{
   const PinTable* table = g_pplayer->m_ptable;

   // Common settings for all anaglyph sets
   m_brightness = table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.0f);
   m_saturation = table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DSaturation"s, 1.0f);
   m_leftEyeContrast = table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DLeftContrast"s, 1.0f);
   m_rightEyeContrast = table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DRightContrast"s, 1.0f);

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
   m_filter = (Filter)table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Filter", 2);
   m_sRGBDisplay = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "sRGB", true);
   m_dynDesatLevel = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "DynDesat", 1.f);
   m_deghostLevel = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Deghost", 0.f);
   leftLum.x = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "LeftRed", -1.f);
   leftLum.y = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "LeftGreen", -1.f);
   leftLum.z = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "LeftBlue", -1.f);
   rightLum.x = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "RightRed", -1.f);
   rightLum.y = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "RightGreen", -1.f);
   rightLum.z = table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "RightBlue", -1.f);
   if (leftLum.x < 0.f || leftLum.y < 0.f || leftLum.z < 0.f || rightLum.x < 0.f || rightLum.y < 0.f || rightLum.z < 0.f)
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
   constexpr float scale = 0.25f * 0.5f; // 0.5 is because we sum the contribution of the 2 eyes, 0.25 is magic adjusted from real play
   shader->SetVector(SHADER_Stereo_LeftLuminance_Gamma, scale * m_rgb2Yl.x, scale * m_rgb2Yl.y, scale * m_rgb2Yl.z, m_sRGBDisplay ? -1.f : m_displayGamma);
   shader->SetVector(SHADER_Stereo_RightLuminance_DynDesat, scale * m_rgb2Yr.x, scale * m_rgb2Yr.y, scale * m_rgb2Yr.z, m_dynDesatLevel);

   // Used by Deghost filter
   shader->SetVector(SHADER_Stereo_DeghostGamma, m_deghostGamma.x, m_deghostGamma.y, m_deghostGamma.z, 0.0f);
   shader->SetMatrix(SHADER_Stereo_DeghostFilter, &m_deghostFilter);

   // Select the shader based on the filter, dynamic desaturation and the gamma mode
   if (m_filter == DEGHOST)
      shader->SetTechnique(SHADER_TECHNIQUE_Stereo_DeghostAnaglyph);
   else if (m_dynDesatLevel > 0.f)
      shader->SetTechnique(m_sRGBDisplay ? SHADER_TECHNIQUE_Stereo_sRGBDynDesatAnaglyph : SHADER_TECHNIQUE_Stereo_GammaDynDesatAnaglyph);
   else
      shader->SetTechnique(m_sRGBDisplay ? SHADER_TECHNIQUE_Stereo_sRGBAnaglyph : SHADER_TECHNIQUE_Stereo_GammaAnaglyph);
}

vec3 Anaglyph::Gamma(const vec3& rgb) const
{
   #define sRGB(x) (((x) <= 0.0031308f) ? (12.92f * (x)) : (1.055f * powf(x, (float)(1.0 / 2.4)) - 0.055f))
   if (m_sRGBDisplay)
      return vec3(sRGB(rgb.x), sRGB(rgb.y), sRGB(rgb.z));
   else
      return vec3(powf(rgb.x, 1.f / m_displayGamma), powf(rgb.y, 1.f / m_displayGamma), powf(rgb.z, 1.f / m_displayGamma));
   #undef sRGB
}

vec3 Anaglyph::InvGamma(const vec3& rgb) const
{
   if (m_sRGBDisplay)
      return vec3(InvsRGB(rgb.x), InvsRGB(rgb.y), InvsRGB(rgb.z));
   else
      return vec3(powf(rgb.x, m_displayGamma), powf(rgb.y, m_displayGamma), powf(rgb.z, m_displayGamma));
}

vec3 Anaglyph::LinearRGBtoXYZ(const vec3& linearRGB) const
{
   vec3 xyz;
   xyz.x = 0.4124564f * linearRGB.x + 0.3575761f * linearRGB.y + 0.1804375f * linearRGB.z;
   xyz.y = 0.2126729f * linearRGB.x + 0.7151522f * linearRGB.y + 0.0721750f * linearRGB.z;
   xyz.z = 0.0193339f * linearRGB.x + 0.1191920f * linearRGB.y + 0.9503041f * linearRGB.z;
   return xyz;
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
         float f = powf(leftLum.x, gamma) + powf(leftLum.y, gamma) + powf(leftLum.z, gamma) - 1.f
                 + powf(rightLum.x, gamma) + powf(rightLum.y, gamma) + powf(rightLum.z, gamma) - 1.f;
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

   Update();
}

void Anaglyph::SetPhotoCalibration(const Matrix3& display, const Matrix3& leftFilter, const Matrix3& rightFilter)
{
   // Use a complete calibration made of matrices transforming an input linear RGB color to its perceived CIE 
   // XYZ color thourhg the display/filter pairs or directly through the display. These matrices can be obtained 
   // using dedicated hardware or simply using a digital camera, taking a photo of RGB stripes (directly and 
   // through the filters) and getting the linear RGB from this photo. If using a digital camera, the camera 
   // processing will modify the result leading to an incorrect calibration. Therefore, a single shot for all 
   // 3 filters should be made for the value to be consistent together (still modified by the camera DSP for 
   // exposure, color balance,... but at least they can be used together).

   // This process is somewhat difficult and highly depends on its understanding and the digital camera settings.
   // So far, it is not used in the application, but only kept for future reference (or to be used to prepare better
   // filters since the ones in Dubois paper were made with old CRT display with a fairly different emission spectrum
   // from nowadays LCD or OLED displays). Another point to look at if using this method is that it is made to 
   // work with a sRGB calibrated display but modern HDR displays can have a fairly different gamma curve.

   #if 0
   // Matrix that transform a linear RGB color to its XYZ projection by the display
   /*vec3 displayRed = LinearRGBtoXYZ(vec3(0.8330f, 0.0054f, 0.0284f)); // Personal Iiyama LCD screen
   vec3 displayGreen = LinearRGBtoXYZ(vec3(0.0024f, 0.7637f, 0.1603f));
   vec3 displayBlue = LinearRGBtoXYZ(vec3(0.0103f, 0.0002f, 0.7630f)); //*/
   vec3 displayRed = LinearRGBtoXYZ(vec3(0.9471f, 0.0673f, 0.0731f)); // Personal Iiyama LCD screen v2
   vec3 displayGreen = LinearRGBtoXYZ(vec3(0.1083f, 0.9408f, 0.1663f));
   vec3 displayBlue = LinearRGBtoXYZ(vec3(0.0383f, 0.0158f, 0.8950f)); //*/
   /*vec3 displayRed = vec3(0.4641f, 0.2597f, 0.03357f); // Eric Dubois values (Sony Trinitron CRT display)
   vec3 displayGreen = vec3(0.3055f, 0.6592f, 0.1421f);
   vec3 displayBlue = vec3(0.1808f, 0.0811f, 0.9109f); //*/
   Matrix3 display = Matrix3(
      displayRed.x, displayGreen.x, displayBlue.x, 
      displayRed.y, displayGreen.y, displayBlue.y, 
      displayRed.z, displayGreen.z, displayBlue.z
   );
   // Matrix that transform a linear RGB color to its XYZ projection by the display through the left filter
   /*vec3 leftRed = LinearRGBtoXYZ(vec3(0.8617, 0.0127, 0.0367)); // Aviator Anachrome
   vec3 leftGreen = LinearRGBtoXYZ(vec3(0.2366, 0.0167, 0.0025));
   vec3 leftBlue = LinearRGBtoXYZ(vec3(0.0084, 0.0008, 0.0074)); //*/
   vec3 leftRed = LinearRGBtoXYZ(vec3(0.8041, 0.0090, 0.0181)); // Aviator Anachrome v2
   vec3 leftGreen = LinearRGBtoXYZ(vec3(0.0755, 0.0084, 0.0034));
   vec3 leftBlue = LinearRGBtoXYZ(vec3(0.0293, 0.0027, 0.0067)); //*/
   /* vec3 leftRed = LinearRGBtoXYZ(vec3(0.8561f, 0.0026f, 0.0500f)); // Trioviz Inficolor (not anaglyph)
   vec3 leftGreen = LinearRGBtoXYZ(vec3(0.1827f, 0.1562f, 0.1148f));
   vec3 leftBlue = LinearRGBtoXYZ(vec3(0.0097f, 0.0002f, 0.7355f)); //*/
   /*vec3 leftRed = vec3(0.3185f, 0.1501f, 0.0007f); // Eric Dubois values (Roscolux Red/Cyan filter)
   vec3 leftGreen = vec3(0.0769f, 0.0767f, 0.0020f);
   vec3 leftBlue = vec3(0.0109f, 0.0056f, 0.0156f); //*/
   Matrix3 leftFilter = Matrix3(
      leftRed.x, leftGreen.x, leftBlue.x,
      leftRed.y, leftGreen.y, leftBlue.y,
      leftRed.z, leftGreen.z, leftBlue.z
   );
   // Matrix that transform a linear RGB color to its XYZ projection by the display through the right filter
   /*vec3 rightRed = LinearRGBtoXYZ(vec3(0.4626, 0.0198, 0.0528)); // Aviator Anachrome
   vec3 rightGreen = LinearRGBtoXYZ(vec3(0.0003, 0.7942, 0.3183));
   vec3 rightBlue = LinearRGBtoXYZ(vec3(0.0019, 0.0100, 0.6646)); //*/
   vec3 rightRed = LinearRGBtoXYZ(vec3(0.2353, 0.0348, 0.0462)); // Aviator Anachrome v2
   vec3 rightGreen = LinearRGBtoXYZ(vec3(0.0033, 0.5900, 0.1430));
   vec3 rightBlue = LinearRGBtoXYZ(vec3(0.0069, 0.0045, 0.4385)); //*/
   /*vec3 rightRed = LinearRGBtoXYZ(vec3(0.5470f, 0.0229f, 0.0210f)); // Trioviz Inficolor (not truly anaglyph since both eyes see most of the colors)
   vec3 rightGreen = LinearRGBtoXYZ(vec3(0.0005f, 0.8109f, 0.2163f));
   vec3 rightBlue = LinearRGBtoXYZ(vec3(0.0011f, 0.0031f, 0.3776f)); //*/
   /*vec3 rightRed = vec3(0.0174f, 0.0184f, 0.0286f); // Eric Dubois values (Roscolux Red/Cyan filter)
   vec3 rightGreen = vec3(0.0484f, 0.1807f, 0.0991f);
   vec3 rightBlue = vec3(0.1402f, 0.0458f, 0.7662f);//*/
   Matrix3 rightFilter = Matrix3(
      rightRed.x, rightGreen.x, rightBlue.x, 
      rightRed.y, rightGreen.y, rightBlue.y, 
      rightRed.z, rightGreen.z, rightBlue.z
   );
   #endif

   // For the perceived luminance filter approach, we only use the Y coordinate (luminance) of the display through filter transmission matrices
   m_rgb2Yl = vec3(leftFilter._21, leftFilter._22, leftFilter._23);
   m_rgb2Yr = vec3(rightFilter._21, rightFilter._22, rightFilter._23);

   // For Dubois filter, we perform the proposed projection in the paper from our calibration data (matrices have the same name as in the paper)
   float dMat[6 * 6] = {}; // D matrix is the 6x6 display transmission made from [[C, 0][0, C]]
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         dMat[j + i * 6] = display.m_d[i][j];
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         dMat[(j + 3) + (i + 3) * 6] = display.m_d[i][j];
   float rMat[3 * 6]; // R matrix made from left and right transmission matrices
   memcpy(rMat, &leftFilter._11, 3 * 3 * sizeof(float));
   memcpy(&rMat[3 * 3], &rightFilter._11, 3 * 3 * sizeof(float));
   Matrix3 bMat;
   bMat.SetIdentity(0.f); // B = transpose(R) x R
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
         for (int k = 0; k < 6; k++)
            bMat.m_d[i][j] += rMat[i + k * 3] * rMat[j + k * 3];
   bMat.Invert(); // B = inverse(B)
   float b2Mat[3 * 6] = {}; // B2 = B x transpose(R)
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 6; j++)
         for (int k = 0; k < 3; k++)
            b2Mat[j + i * 6] += bMat.m_d[i][k] * rMat[k + j * 3];
   float b3Mat[3 * 6] = {}; // B3 = B2 x D
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 6; j++)
         for (int k = 0; k < 6; k++)
            b3Mat[j + i * 6] += b2Mat[k + i * 6] * dMat[j + k * 6];
   float rowNorm[3] = {}; // Sum of each row for matrix normalization
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 6; j++)
         rowNorm[i] += b3Mat[j + i * 6];
   for (int i = 0; i < 3; i++) // B3 = normalized(B3)
      for (int j = 0; j < 6; j++)
         b3Mat[j + i * 6] /= rowNorm[i];
   // Derive left/right matrices from the resulting [3x6] matrix
   m_rgb2AnaglyphLeft = m_rgb2AnaglyphRight = Matrix3D::MatrixIdentity();
   for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
      {
         m_rgb2AnaglyphLeft.m[i][j] = b3Mat[j + i * 6];
         m_rgb2AnaglyphRight.m[i][j] = b3Mat[(j + 3) + i * 6];
      }

   // This will overwrite the Dubois matrix and needs to be reworked if using full calibration
   Update();
}

void Anaglyph::Update()
{
   // Enforce luminance model property to account for calibration error (Y(white) = 1)
   m_rgb2Yl = m_rgb2Yl / (m_rgb2Yl.x + m_rgb2Yl.y + m_rgb2Yl.z);
   m_rgb2Yr = m_rgb2Yr / (m_rgb2Yr.x + m_rgb2Yr.y + m_rgb2Yr.z);

   // Compute the anaglyph property of the filter (ability to split the image between the eyes)
   m_anaglyphRatio = vec3(fabsf(m_rgb2Yl.x - m_rgb2Yr.x) / (m_rgb2Yl.x + m_rgb2Yr.x), 
                          fabsf(m_rgb2Yl.y - m_rgb2Yr.y) / (m_rgb2Yl.y + m_rgb2Yr.y), 
                          fabsf(m_rgb2Yl.z - m_rgb2Yr.z) / (m_rgb2Yl.z + m_rgb2Yr.z));

   // Evaluate filter tints (for UI display)
   m_leftEyeColor = vec3(m_rgb2Yl.x / 0.2126f, m_rgb2Yl.y / 0.7152f, m_rgb2Yl.z / 0.0722f);
   m_rightEyeColor = vec3(m_rgb2Yr.x / 0.2126f, m_rgb2Yr.y / 0.7152f, m_rgb2Yr.z / 0.0722f);
   m_leftEyeColor = m_leftEyeColor / max(max(m_leftEyeColor.x, m_leftEyeColor.y), m_leftEyeColor.z);
   m_rightEyeColor = m_rightEyeColor / max(max(m_rightEyeColor.x, m_rightEyeColor.y), m_rightEyeColor.z);

   // Identify the bichromatic eye with its color from the luminance calibration.
   //vec3 eyeL(m_rgb2Yl), eyeR(m_rgb2Yr); // Both will work. I'm not sure which one is the correct approach here
   vec3 eyeL(m_leftEyeColor), eyeR(m_rightEyeColor);
   #define vecChannel(v, c) (c == 0 ? v.x : c == 1 ? v.y : v.z)
   float maxLeft = 0.f, maxRight = 0.f;
   int mainLeft = -1, mainRight = -1;
   for (int i = 0; i < 3; i++)
   {
      if (vecChannel(eyeL, i) > maxLeft) { maxLeft = vecChannel(eyeL, i); mainLeft = i; }
      if (vecChannel(eyeR, i) > maxRight) { maxRight = vecChannel(eyeR, i); mainRight = i; }
   }
   if (mainLeft == mainRight)
   {
      // Non anaglyph glasses with the same main channel for the 2 eyes
      PLOGI << "Invalid anaglyph calibration: both eyes have the same main color channel";
      m_reversedColorPair = false;
      m_colorPair = RED_CYAN;
   }
   else
   {
      // The bichromatic is the one with the higher luminance value of the channel which is not the main of any of the eyes
      int lesser = ((1 << mainLeft) | (1 << mainRight)) ^ 0x7;
      lesser = (int) log2(lesser & -lesser); // Return the position of the first bit which is the unused channel
      m_reversedColorPair = vecChannel(eyeL, lesser) > vecChannel(eyeR, lesser);
      m_colorPair = (AnaglyphPair)(m_reversedColorPair ? mainRight : mainLeft);
   }
   #undef vecChannel

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
      Matrix3D matYYC2RGB = Matrix3D::MatrixIdentity();
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
