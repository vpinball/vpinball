// license:GPLv3+

#include "core/stdafx.h"
#include "Anaglyph.h"

Anaglyph::Anaglyph()
{
}

void Anaglyph::LoadSetupFromRegistry(const Settings& settings, const int glassesSet)
{
   // Common settings for all anaglyph sets
   m_brightness = settings.GetPlayer_Stereo3DBrightness();
   m_saturation = settings.GetPlayer_Stereo3DSaturation();
   m_leftEyeContrast = settings.GetPlayer_Stereo3DLeftContrast();
   m_rightEyeContrast = settings.GetPlayer_Stereo3DRightContrast();

   const int set = clamp(glassesSet, 0, 9);
   m_filter = (Filter)settings.GetPlayer_AnaglyphFilter(set);
   m_sRGBDisplay = settings.GetPlayer_AnaglyphsRGB(set);
   m_dynDesatLevel = settings.GetPlayer_AnaglyphDynDesat(set);
   m_deghostLevel = settings.GetPlayer_AnaglyphDeghost(set);
   // calibrated sRGB luminance transfer (for each primary, pereceived luminance level of the display emitted light, by the eye of the player through the glass filter)
   const vec3 leftLumSRGB { settings.GetPlayer_AnaglyphLeftRed(set), settings.GetPlayer_AnaglyphLeftGreen(set), settings.GetPlayer_AnaglyphLeftBlue(set) };
   const vec3 rightLumSRGB { settings.GetPlayer_AnaglyphRightRed(set), settings.GetPlayer_AnaglyphRightGreen(set), settings.GetPlayer_AnaglyphRightBlue(set) };
   SetLuminanceCalibration(leftLumSRGB, rightLumSRGB);
}

void Anaglyph::SetupShader(Shader* shader) const
{
   // Main matrices to project from linear rgb to anaglyph
   shader->SetMatrix(SHADER_Stereo_LeftMat, &m_rgb2AnaglyphLeft);
   shader->SetMatrix(SHADER_Stereo_RightMat, &m_rgb2AnaglyphRight);
   
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
   if (m_sRGBDisplay)
      return vec3(sRGB(rgb.x), sRGB(rgb.y), sRGB(rgb.z));
   else
      return vec3(powf(rgb.x, 1.f / m_displayGamma), powf(rgb.y, 1.f / m_displayGamma), powf(rgb.z, 1.f / m_displayGamma));
}

vec3 Anaglyph::InvGamma(const vec3& rgb) const
{
   if (m_sRGBDisplay)
      return vec3(InvsRGB(rgb.x), InvsRGB(rgb.y), InvsRGB(rgb.z));
   else
      return vec3(powf(rgb.x, m_displayGamma), powf(rgb.y, m_displayGamma), powf(rgb.z, m_displayGamma));
}

#define powd(a, b) pow(static_cast<double>(a), static_cast<double>(b))
#define logd(a)    log(static_cast<double>(a))

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
      double gamma = 2.4;
      for (int i = 0; i < 100; i++)
      {
         const double f = powd(leftLum.x, gamma)  + powd(leftLum.y, gamma)  + powd(leftLum.z, gamma)  - 1.
                        + powd(rightLum.x, gamma) + powd(rightLum.y, gamma) + powd(rightLum.z, gamma) - 1.;
         const double fp = powd(leftLum.x, gamma)  / logd(leftLum.x)  + powd(leftLum.y, gamma)  / logd(leftLum.y)  + powd(leftLum.z, gamma)  / logd(leftLum.z)
                         + powd(rightLum.x, gamma) / logd(rightLum.x) + powd(rightLum.y, gamma) / logd(rightLum.y) + powd(rightLum.z, gamma) / logd(rightLum.z);
         gamma -= f / fp;
      }
      m_displayGamma = (float)gamma;
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
   // XYZ color through the display/filter pairs or directly through the display. These matrices can be obtained 
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
   memcpy(rMat, leftFilter.m_d, sizeof(leftFilter.m_d));
   memcpy(&rMat[3 * 3], rightFilter.m_d, sizeof(rightFilter.m_d));
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


static Vertex3Ds EvaluateGlassFilter(const Matrix3& M, const float eps = 1e-6f)
{
   Matrix3 A = M;

   // Gaussian elimination
   for (int col = 0; col < 3; ++col)
   {
      // Partial pivoting: find the row with the largest element in the current column
      int max_row = col;
      for (int i = col + 1; i < 3; ++i)
         if (std::abs(A.m_d[i][col]) > std::abs(A.m_d[max_row][col]))
            max_row = i;

      // Swap rows
      for (int j = 0; j < 3; ++j)
         std::swap(A.m_d[col][j], A.m_d[max_row][j]);

      // If the pivot is zero, skip to the next column
      if (std::abs(A.m_d[col][col]) <= eps)
         continue;

      // Eliminate the current column in all rows below
      for (int i = col + 1; i < 3; ++i)
      {
         float factor = A.m_d[i][col] / A.m_d[col][col];
         for (int j = col; j < 3; ++j)
            A.m_d[i][j] -= factor * A.m_d[col][j];
      }
   }

   // Evaluate rank (number of free variables, that is to say null rows)
   {
      int rank = 0;
      float minZero = FLT_MAX;
      for (int row = 0; row < 3; ++row)
      {
         float maxRow = FLT_MIN;
         for (int col = 0; col < 3; ++col)
            maxRow = max(maxRow, std::abs(A.m_d[row][col]));
         if (maxRow > eps)
            rank++;
         minZero = min(minZero, maxRow);
      }
      if (rank == 3)
         // Only the { 0.0f, 0.0f, 0.0f } trivial solution exists but we know that this is not our solution (glass are not full opaque...)
         // We guess that this is a calibration precision, issue and restart with an higher precision limit guaranteed to give a suitable rank
         return EvaluateGlassFilter(A, minZero);
   }

   // Find a free variable and set it to 1
   int free_var = -1;
   Vertex3Ds filter = { 0.0f, 0.0f, 0.0f };
   {
      float minZero = FLT_MAX;
      for (int col = 0; col < 3; ++col)
      {
         float maxCol = FLT_MIN;
         for (int row = 0; row < 3; ++row)
            maxCol = max(maxCol, std::abs(A.m_d[row][col]));
         if (maxCol < minZero)
         {
            minZero = maxCol;
            free_var = col;
         }
      }
      switch (free_var)
      {
      case 0: filter.x = 1.0f; break;
      case 1: filter.y = 1.0f; break;
      case 2: filter.z = 1.0f; break;
      }
   }

   // Solve for the other variables
   for (int row = 2; row >= 0; --row)
   {
      float sum = 0.0f;
      int pivot_col = -1;
      for (int col = 0; col < 3; ++col)
      {
         if (pivot_col == -1 && col != free_var && std ::abs(A.m_d[row][col]) > eps)
            pivot_col = col;
         else
         {
            switch (col)
            {
            case 0: sum += A.m_d[row][col] * filter.x; break;
            case 1: sum += A.m_d[row][col] * filter.y; break;
            case 2: sum += A.m_d[row][col] * filter.z; break;
            }
         }
      }
      if (pivot_col != -1)
      {
         switch (pivot_col)
         {
         case 0: filter.x = -sum / A.m_d[row][pivot_col]; break;
         case 1: filter.y = -sum / A.m_d[row][pivot_col]; break;
         case 2: filter.z = -sum / A.m_d[row][pivot_col]; break;
         }
      }
   }

   filter.NormalizeSafe();
   if (filter.x < 0.0f || filter.y < 0.0f || filter.z < 0.0f)
   {
      // We have a calibration issue as we are searching for a glass filter so only 0..1 values are valids, clamp and renormalize
      filter.x = max(filter.x, 0.0f);
      filter.y = max(filter.y, 0.0f);
      filter.z = max(filter.z, 0.0f);
      filter.NormalizeSafe();
   }

   return filter;
}


void Anaglyph::Update()
{
   // Calibration consist in finding the point where the white seen through the glass filter has the same luminance as the primary 
   // also seen through the glass filter. The corresponding equation is (linear RGB colorspace):
   //   Cr.Lr.Gr + Cr.Lg.Gg + Cr.Lb.Gb = Lr.Gr
   //   Cg.Lr.Gr + Cg.Lg.Gg + Cg.Lb.Gb = Lr.Gg
   //   Cb.Lr.Gr + Cb.Lg.Gg + Cb.Lb.Gb = Lr.Gb
   // With:
   // . C the Calibration level per primary
   // . L the Luminance factor per primary
   // . G the Glass filter factor per primary
   // The rank of this equation is under 3 otherwise the only solution would be an opaque black filter (G = { 0, 0, 0 })
   // These equations also implies that Cr + Cg + Cb = 1
   
   // Enforce Cr + Cg + Cb = 1 as this may not be true due to calibration imprecision
   m_rgb2Yl = m_rgb2Yl / (m_rgb2Yl.x + m_rgb2Yl.y + m_rgb2Yl.z);
   m_rgb2Yr = m_rgb2Yr / (m_rgb2Yr.x + m_rgb2Yr.y + m_rgb2Yr.z);

   // Solve the equations for left and right eyes
   vec3 lum { 0.2126f, 0.7152f, 0.0722f };
   Matrix3 leftMat;
   leftMat._11 = lum.x * (m_rgb2Yl.x - 1.f);
   leftMat._12 = lum.y * m_rgb2Yl.x;
   leftMat._13 = lum.z * m_rgb2Yl.x;
   leftMat._21 = lum.x * m_rgb2Yl.y;
   leftMat._22 = lum.y * (m_rgb2Yl.y - 1.f);
   leftMat._23 = lum.z * m_rgb2Yl.y;
   leftMat._31 = lum.x * m_rgb2Yl.z;
   leftMat._32 = lum.y * m_rgb2Yl.z;
   leftMat._33 = lum.z * (m_rgb2Yl.z - 1.f);
   m_leftEyeGlassFilter = EvaluateGlassFilter(leftMat);

   Matrix3 rightMat;
   rightMat._11 = lum.x * (m_rgb2Yr.x - 1.f);
   rightMat._12 = lum.y * m_rgb2Yr.x;
   rightMat._13 = lum.z * m_rgb2Yr.x;
   rightMat._21 = lum.x * m_rgb2Yr.y;
   rightMat._22 = lum.y * (m_rgb2Yr.y - 1.f);
   rightMat._23 = lum.z * m_rgb2Yr.y;
   rightMat._31 = lum.x * m_rgb2Yr.z;
   rightMat._32 = lum.y * m_rgb2Yr.z;
   rightMat._33 = lum.z * (m_rgb2Yr.z - 1.f);
   m_rightEyeGlassFilter = EvaluateGlassFilter(rightMat);

   // Identify the color pair from the luminance calibration
   const vec3 leftHSV = VPX::Colors::SRGBToHSV(m_leftEyeGlassFilter);
   const vec3 rightHSV = VPX::Colors::SRGBToHSV(m_rightEyeGlassFilter);
   float bestPairMatchDist = FLT_MAX;
   int bestPairMatch = -1;
   const auto angularDistance = [](float a, float b)
   {
      float diff = fabsf(fmodf(a, 360.0f) - fmodf(b, 360.0f));
      return std::min(diff, 360.0f - diff);
   };
   for (int i = 0; i < 6; i++)
   {
      float dist = powf(angularDistance(leftHSV.x,i * 60.f), 2.f) + powf(angularDistance(rightHSV.x, i * 60.f + 180.f), 2.f);
      if (dist < bestPairMatchDist)
      {
         bestPairMatchDist = dist;
         bestPairMatch = i;
      }
   }
   switch (bestPairMatch)
   {
   case 0:
      m_colorPair = RED_CYAN;
      m_reversedColorPair = false;
      break;
   case 1:
      m_colorPair = BLUE_AMBER;
      m_reversedColorPair = true;
      break;
   case 2:
      m_colorPair = GREEN_MAGENTA;
      m_reversedColorPair = false;
      break;
   case 3:
      m_colorPair = RED_CYAN;
      m_reversedColorPair = true;
      break;
   case 4:
      m_colorPair = BLUE_AMBER;
      m_reversedColorPair = false;
      break;
   case 5:
      m_colorPair = GREEN_MAGENTA;
      m_reversedColorPair = true;
      break;
   }

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
      if (m_reversedColorPair)
         std::swap(m_rgb2AnaglyphLeft, m_rgb2AnaglyphRight);
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
      if (m_reversedColorPair)
         std::swap(m_rgb2AnaglyphLeft, m_rgb2AnaglyphRight);
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
      vec3 rgb2Yl(m_rgb2Yl);
      vec3 rgb2Yr(m_rgb2Yr);
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
      const Matrix3D matLeft2YYC( // Left image to left luminance, right luminance, chromacity
         rgb2Yl.x, rgb2Yl.y, rgb2Yl.z, 0.f, //
         0.f, 0.f, 0.f, 0.f, //
         0.f, 0.f, 0.f, 0.f, //
         0.f, 0.f, 0.f, 1.f);
      const Matrix3D matRight2YYC( // Right image to left luminance, right luminance, chromacity
         0.f, 0.f, 0.f, 0.f, //
         rgb2Yr.x, rgb2Yr.y, rgb2Yr.z, 0.f, //
         chromacity.x, chromacity.y, chromacity.z, 0.f, //
         0.f, 0.f, 0.f, 1.f);
      m_rgb2AnaglyphLeft = matYYC2RGB * matLeft2YYC;
      m_rgb2AnaglyphRight = matYYC2RGB * matRight2YYC;
      break;
   }

   case DEGHOST:
   {
      // Compose anaglyph by applying John Einselen's contrast and deghosting method
      // see http://iaian7.com/quartz/AnaglyphCompositing & vectorform.com
      constexpr double contrast = 1.;
      if (m_colorPair == RED_CYAN)
      {
         constexpr double a = 0.45 * contrast, b = 0.5 * (1. - a);
         m_rgb2AnaglyphLeft = Matrix3D { //
            (float)a, (float)b, (float)b, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double c = 1.00 * contrast, d = 1. - c;
         m_rgb2AnaglyphRight = Matrix3D { //
            0.f, 0.f, 0.f, 0.f, //
            (float)d, (float)c, 0.f, 0.f, //
            (float)d, 0.f, (float)c, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double e = 0.06 * 0.1;
         m_deghostGamma = vec3 { 1.00f, 1.15f, 1.15f };
         m_deghostFilter = Matrix3D { //
            (float)(1. + e), (float)(-0.5 * e), (float)(-0.5 * e), 0.f, //
            (float)(-0.25 * e), (float)(1. + 0.5 * e), (float)(-0.25 * e), 0.f, //
            (float)(-0.25 * e), (float)(-0.25 * e), (float)(1. + 0.5 * e), 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
      }
      else if (m_colorPair == GREEN_MAGENTA)
      {
         constexpr double a = 1.00 * contrast, b = 0.5 * (1. - a);
         m_rgb2AnaglyphLeft = Matrix3D { //
            0.f, 0.f, 0.f, 0.f, //
            (float)b, (float)a, (float)b, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double c = 0.80 * contrast, d = 1. - c;
         m_rgb2AnaglyphRight = Matrix3D { //
            (float)c, (float)d, 0.f, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            0.f, (float)d, (float)c, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double e = 0.06 * 0.275;
         m_deghostGamma = vec3 { 1.15f, 1.05f, 1.15f };
         m_deghostFilter = Matrix3D { //
            (float)(1. + 0.5 * e), (float)(-0.25 * e), (float)(-0.25 * e), 0.f, //
            (float)(-0.5 * e), (float)(1. + 0.25 * e), (float)(-0.5 * e), 0.f, //
            (float)(-0.25 * e), (float)(-0.25 * e), (float)(1. + 0.5 * e), 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
      }
      else if (m_colorPair == BLUE_AMBER)
      {
         constexpr double a = 0.45 * contrast, b = 0.5 * (1. - a);
         m_rgb2AnaglyphLeft = Matrix3D { //
            0.f, 0.f, 0.f, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            (float)b, (float)b, (float)a, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double c = 1.00 * contrast, d = 1. - c;
         m_rgb2AnaglyphRight = Matrix3D { //
            (float)c, 0.f, (float)d, 0.f, //
            0.f, (float)c, (float)d, 0.f, //
            0.f, 0.f, 0.f, 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
         constexpr double e = 0.06 * 0.275;
         m_deghostGamma = vec3 { 1.05f, 1.10f, 1.00f };
         m_deghostFilter = Matrix3D { //
            (float)(1. + 1.5 * e), (float)(-0.75 * e), (float)(-0.75 * e), 0.f, //
            (float)(-0.75 * e), (float)(1. + 1.5 * e), (float)(-0.75 * e), 0.f, //
            (float)(-1.5 * e), (float)(-1.5 * e), (float)(1. + 3. * e), 0.f, //
            0.f, 0.f, 0.f, 1.f
         };
      }
      m_deghostFilter.Transpose();
      if (m_reversedColorPair)
         std::swap(m_rgb2AnaglyphLeft, m_rgb2AnaglyphRight);
      break;
   }
   }

   // Common parameters for all anaglyph composition filters
   const Matrix3D matLeftContrast = Matrix3D::MatrixTranslate(-0.5f, -0.5f, -0.5f) * Matrix3D::MatrixScale(m_leftEyeContrast) * Matrix3D::MatrixTranslate(0.5f, 0.5f, 0.5f);
   const Matrix3D matRightContrast = Matrix3D::MatrixTranslate(-0.5f, -0.5f, -0.5f) * Matrix3D::MatrixScale(m_rightEyeContrast) * Matrix3D::MatrixTranslate(0.5f, 0.5f, 0.5f);
   static constexpr Matrix3D matGrayscale { //
      0.212655f, 0.715158f, 0.072187f, 0.f, //
      0.212655f, 0.715158f, 0.072187f, 0.f, //
      0.212655f, 0.715158f, 0.072187f, 0.f, //
      0.f, 0.f, 0.f, 1.f};
   Matrix3D matSaturation = Matrix3D::MatrixScale(1.f - m_saturation) * matGrayscale + Matrix3D::MatrixScale(m_saturation);
   const Matrix3D matBrightness = Matrix3D::MatrixScale(m_brightness); 
   m_rgb2AnaglyphLeft = matBrightness * matLeftContrast * m_rgb2AnaglyphLeft * matSaturation;
   m_rgb2AnaglyphRight = matBrightness * matRightContrast * m_rgb2AnaglyphRight * matSaturation;

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

// This must be kept in sync with the GPU shader implementation
vec3 Anaglyph::ComputeColor(const vec3& leftCol, const vec3& rightCol, bool isLinearInput) const
{
   vec3 lCol = leftCol;
   vec3 rCol = rightCol;

   // 1. If not Deghost and not linear, convert to linear space (InvGamma)
   if (m_filter != DEGHOST && !isLinearInput)
   {
      lCol = InvGamma(lCol);
      rCol = InvGamma(rCol);
   }

   // 2. Apply Dynamic Desaturation if active and not Deghost
   if (m_filter != DEGHOST && m_dynDesatLevel > 0.f)
   {
      const float scale = 0.25f * 0.5f;
      vec3 lLumGamma = m_rgb2Yl * scale;
      vec3 rLumDynDesat = m_rgb2Yr * scale;
      
      const float left2LeftLum = lCol.x * lLumGamma.x + lCol.y * lLumGamma.y + lCol.z * lLumGamma.z;
      const float left2RightLum = lCol.x * rLumDynDesat.x + lCol.y * rLumDynDesat.y + lCol.z * rLumDynDesat.z;
      const float right2LeftLum = rCol.x * lLumGamma.x + rCol.y * lLumGamma.y + rCol.z * lLumGamma.z;
      const float right2RightLum = rCol.x * rLumDynDesat.x + rCol.y * rLumDynDesat.y + rCol.z * rLumDynDesat.z;
      
      const float leftLum = left2LeftLum + left2RightLum;
      const float rightLum = right2LeftLum + right2RightLum;
      
      const float leftDesat = m_dynDesatLevel * fabsf((left2LeftLum - left2RightLum) / (leftLum + 0.0001f));
      const float rightDesat = m_dynDesatLevel * fabsf((right2LeftLum - right2RightLum) / (rightLum + 0.0001f));
      
      lCol = lCol * (1.0f - leftDesat) + vec3(leftLum, leftLum, leftLum) * leftDesat;
      rCol = rCol * (1.0f - rightDesat) + vec3(rightLum, rightLum, rightLum) * rightDesat;
   }

   // 3. Setup linear projection matrices based on reversed pair state
   const Matrix3D& lMat = m_rgb2AnaglyphLeft;
   const Matrix3D& rMat = m_rgb2AnaglyphRight;

   // 4. Multiply color by matrices (LinearAnaglyph)
   vec3 color = vec3(
       lCol.x * lMat.m[0][0] + lCol.y * lMat.m[1][0] + lCol.z * lMat.m[2][0] + lMat.m[3][0] +
       rCol.x * rMat.m[0][0] + rCol.y * rMat.m[1][0] + rCol.z * rMat.m[2][0] + rMat.m[3][0],
       
       lCol.x * lMat.m[0][1] + lCol.y * lMat.m[1][1] + lCol.z * lMat.m[2][1] + lMat.m[3][1] +
       rCol.x * rMat.m[0][1] + rCol.y * rMat.m[1][1] + rCol.z * rMat.m[2][1] + rMat.m[3][1],
       
       lCol.x * lMat.m[0][2] + lCol.y * lMat.m[1][2] + lCol.z * lMat.m[2][2] + lMat.m[3][2] +
       rCol.x * rMat.m[0][2] + rCol.y * rMat.m[1][2] + rCol.z * rMat.m[2][2] + rMat.m[3][2]
   );

   // 5. Apply Deghost Filter or Revert Gamma
   if (m_filter == DEGHOST)
   {
      vec3 gammaColor(
          powf(max(color.x, 0.f), m_deghostGamma.x),
          powf(max(color.y, 0.f), m_deghostGamma.y),
          powf(max(color.z, 0.f), m_deghostGamma.z)
      );
      
      color = vec3(
          gammaColor.x * m_deghostFilter.m[0][0] + gammaColor.y * m_deghostFilter.m[1][0] + gammaColor.z * m_deghostFilter.m[2][0],
          gammaColor.x * m_deghostFilter.m[0][1] + gammaColor.y * m_deghostFilter.m[1][1] + gammaColor.z * m_deghostFilter.m[2][1],
          gammaColor.x * m_deghostFilter.m[0][2] + gammaColor.y * m_deghostFilter.m[1][2] + gammaColor.z * m_deghostFilter.m[2][2]
      );
   }
   else if (!isLinearInput)
   {
      color = Gamma(color);
   }

   return color;
}
