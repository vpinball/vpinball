// license:GPLv3+

#include <algorithm>
#include <cmath>

#include "B2SProEffects.h"

namespace B2S::ProEffects {

static inline uint8_t ClampByte(const double value) { return static_cast<uint8_t>(std::max(0, std::min(255, static_cast<int>(std::lround(value))))); }
static inline double Lerp(const double a, const double b, double amount)
{
   amount = std::max(0.0, std::min(1.0, amount));
   return a + (b - a) * amount;
}
static inline double SmoothStep(const double value)
{
   const double t = std::max(0.0, std::min(1.0, value));
   return t * t * (3.0 - 2.0 * t);
}

// Bilinear resample of an RGB(A) source into a straight alpha RGBA buffer
static std::vector<uint8_t> Resample(const uint8_t* source, const int sw, const int sh, const int bpp, const int width, const int height)
{
   std::vector<uint8_t> out(static_cast<size_t>(width) * height * 4);
   const double scaleX = static_cast<double>(sw) / width, scaleY = static_cast<double>(sh) / height;
   for (int y = 0; y < height; y++)
   {
      const double sy = (y + 0.5) * scaleY - 0.5;
      const int y0 = static_cast<int>(std::floor(sy));
      const double fy = sy - y0;
      for (int x = 0; x < width; x++)
      {
         const double sx = (x + 0.5) * scaleX - 0.5;
         const int x0 = static_cast<int>(std::floor(sx));
         const double fx = sx - x0;
         double acc[4] = { 0.0, 0.0, 0.0, 0.0 };
         for (int j = 0; j < 2; j++)
            for (int i = 0; i < 2; i++)
            {
               const int xi = std::clamp(x0 + i, 0, sw - 1), yi = std::clamp(y0 + j, 0, sh - 1);
               const double w = (i == 0 ? 1.0 - fx : fx) * (j == 0 ? 1.0 - fy : fy);
               const uint8_t* const p = source + (static_cast<size_t>(yi) * sw + xi) * bpp;
               acc[0] += w * p[0];
               acc[1] += w * p[1];
               acc[2] += w * p[2];
               acc[3] += w * (bpp == 4 ? p[3] : 255);
            }
         uint8_t* const d = out.data() + (static_cast<size_t>(y) * width + x) * 4;
         for (int c = 0; c < 4; c++)
            d[c] = ClampByte(acc[c]);
      }
   }
   return out;
}

static void TemperatureFactors(int kelvin, double& rf, double& gf, double& bf)
{
   kelvin = std::max(2000, std::min(6500, kelvin));
   if (kelvin <= 2700)
   {
      const double t = (kelvin - 2000.0) / 700.0;
      rf = Lerp(1.22, 1.14, t);
      gf = Lerp(0.92, 0.96, t);
      bf = Lerp(0.58, 0.72, t);
   }
   else if (kelvin <= 3000)
   {
      const double t = (kelvin - 2700.0) / 300.0;
      rf = Lerp(1.14, 1.1, t);
      gf = Lerp(0.96, 0.98, t);
      bf = Lerp(0.72, 0.8, t);
   }
   else if (kelvin <= 4000)
   {
      const double t = (kelvin - 3000.0) / 1000.0;
      rf = Lerp(1.1, 1.0, t);
      gf = Lerp(0.98, 1.0, t);
      bf = Lerp(0.8, 1.0, t);
   }
   else if (kelvin <= 5000)
   {
      const double t = (kelvin - 4000.0) / 1000.0;
      rf = Lerp(1.0, 0.96, t);
      gf = Lerp(1.0, 1.01, t);
      bf = Lerp(1.0, 1.1, t);
   }
   else
   {
      const double t = (kelvin - 5000.0) / 1500.0;
      rf = Lerp(0.96, 0.9, t);
      gf = Lerp(1.01, 1.0, t);
      bf = Lerp(1.1, 1.22, t);
   }
}

void ApplyTemperatureGrade(uint8_t* rgba, const int width, const int height, const int kelvin)
{
   if (rgba == nullptr || std::max(2000, std::min(6500, kelvin)) == 4000)
      return;
   double rf, gf, bf;
   TemperatureFactors(kelvin, rf, gf, bf);
   for (size_t i = 0; i < static_cast<size_t>(width) * height; i++)
   {
      uint8_t* const p = rgba + i * 4;
      if (p[3] == 0)
         continue;
      const double r = p[0], g = p[1], b = p[2];
      const double oldLuma = r * 0.2126 + g * 0.7152 + b * 0.0722;
      double nr = r * rf, ng = g * gf, nb = b * bf;
      const double newLuma = nr * 0.2126 + ng * 0.7152 + nb * 0.0722;
      if (newLuma > 0.001)
      {
         const double scale = std::max(0.82, std::min(1.18, oldLuma / newLuma));
         nr *= scale;
         ng *= scale;
         nb *= scale;
      }
      const double strength = 0.55 + 0.3 * (oldLuma / 255.0);
      p[0] = ClampByte(r + (nr - r) * strength);
      p[1] = ClampByte(g + (ng - g) * strength);
      p[2] = ClampByte(b + (nb - b) * strength);
   }
}

std::vector<uint8_t> RenderReel(const uint8_t* source, const int sourceWidth, const int sourceHeight, const int sourceBpp, const int width, const int height, int brightness, int temperature, int depth)
{
   if (source == nullptr || sourceWidth <= 0 || sourceHeight <= 0 || width <= 0 || height <= 0)
      return {};
   brightness = std::max(0, std::min(400, brightness));
   temperature = std::max(2000, std::min(6500, temperature));
   depth = std::max(0, std::min(200, depth));
   std::vector<uint8_t> input = Resample(source, sourceWidth, sourceHeight, sourceBpp, width, height);
   std::vector<uint8_t> out(input.size());
   const double depthAmount = depth / 100.0;
   const double lightAmount = brightness / 100.0;
   for (int y = 0; y < height; y++)
   {
      const double normalY = height <= 1 ? 0.0 : 2.0 * y / (height - 1.0) - 1.0;
      const double roundness = std::sqrt(std::max(0.0, 1.0 - normalY * normalY));
      const double drumShade = std::max(0.18, 1.0 - depthAmount * 0.5 * (1.0 - roundness));
      for (int x = 0; x < width; x++)
      {
         const uint8_t* const p = input.data() + (static_cast<size_t>(y) * width + x) * 4;
         uint8_t* const d = out.data() + (static_cast<size_t>(y) * width + x) * 4;
         const int red = p[0], green = p[1], blue = p[2], alpha = p[3];
         const double luminance = (0.2126 * red + 0.7152 * green + 0.0722 * blue) / 255.0;
         const double material = std::pow(std::max(0.0, std::min(1.0, luminance)), 1.35);
         const double normalX = width <= 1 ? 0.0 : 2.0 * x / (width - 1.0) - 1.0;
         const double bulbFalloff = 0.68 + 0.32 * std::max(0.0, 1.0 - std::sqrt(normalX * normalX * 0.38 + normalY * normalY * 0.72));
         const double addedLight = material * bulbFalloff * lightAmount * 0.54;
         d[0] = ClampByte(red * drumShade * 0.67 + 255.0 * addedLight);
         d[1] = ClampByte(green * drumShade * 0.67 + 255.0 * addedLight);
         d[2] = ClampByte(blue * drumShade * 0.67 + 255.0 * addedLight);
         d[3] = static_cast<uint8_t>(alpha);
      }
   }
   // Drum sides: vertical black lines of decreasing alpha on both edges
   const int sideWidth = std::min(std::max(1, width / 3), 12 + static_cast<int>(std::lround(8.0 * depthAmount)));
   for (int i = 0; i < sideWidth; i++)
   {
      const int alpha = std::max(0, std::min(255, static_cast<int>(std::lround(105.0 * depthAmount * (1.0 - static_cast<double>(i) / sideWidth)))));
      if (alpha <= 0)
         continue;
      const double a = alpha / 255.0;
      for (const int x : { i, width - 1 - i })
      {
         if (x < 0 || x >= width)
            continue;
         for (int y = 0; y < height; y++)
         {
            uint8_t* const d = out.data() + (static_cast<size_t>(y) * width + x) * 4;
            // Source-over of black at alpha a onto the pixel (straight alpha)
            const double da = d[3] / 255.0;
            const double outA = a + da * (1.0 - a);
            if (outA <= 0.0)
               continue;
            for (int c = 0; c < 3; c++)
               d[c] = ClampByte((d[c] * da * (1.0 - a)) / outA);
            d[3] = ClampByte(outA * 255.0);
         }
      }
   }
   ApplyTemperatureGrade(out.data(), width, height, temperature);
   return out;
}

// Source-over blend of a straight alpha colour onto a straight alpha RGBA pixel
static inline void BlendPixel(uint8_t* d, const double r, const double g, const double b, const double a)
{
   if (a <= 0.0)
      return;
   const double da = d[3] / 255.0;
   const double outA = a + da * (1.0 - a);
   if (outA <= 0.0)
      return;
   d[0] = ClampByte((r * a + d[0] * da * (1.0 - a)) / outA);
   d[1] = ClampByte((g * a + d[1] * da * (1.0 - a)) / outA);
   d[2] = ClampByte((b * a + d[2] * da * (1.0 - a)) / outA);
   d[3] = ClampByte(outA * 255.0);
}

// Thick anti-aliased line, pen centred on the segment
static void DrawLine(std::vector<uint8_t>& img, const int width, const int height, const double x1, const double y1, const double x2, const double y2, const double thickness, const double r, const double g, const double b,
   const double alpha)
{
   const double half = thickness * 0.5;
   const int left = std::max(0, static_cast<int>(std::floor(std::min(x1, x2) - half - 1)));
   const int right = std::min(width - 1, static_cast<int>(std::ceil(std::max(x1, x2) + half + 1)));
   const int top = std::max(0, static_cast<int>(std::floor(std::min(y1, y2) - half - 1)));
   const int bottom = std::min(height - 1, static_cast<int>(std::ceil(std::max(y1, y2) + half + 1)));
   const double dx = x2 - x1, dy = y2 - y1;
   const double len2 = dx * dx + dy * dy;
   for (int y = top; y <= bottom; y++)
      for (int x = left; x <= right; x++)
      {
         const double px = x + 0.5, py = y + 0.5;
         double t = len2 > 0.0 ? ((px - x1) * dx + (py - y1) * dy) / len2 : 0.0;
         t = std::max(0.0, std::min(1.0, t));
         const double cx = x1 + dx * t, cy = y1 + dy * t;
         const double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
         const double coverage = std::max(0.0, std::min(1.0, half + 0.5 - dist));
         if (coverage > 0.0)
            BlendPixel(img.data() + (static_cast<size_t>(y) * width + x) * 4, r, g, b, alpha * coverage);
      }
}

std::vector<uint8_t> ReelWindowOverlay(const int width, const int height, int depth, int glassReflection)
{
   if (width <= 2 || height <= 2)
      return {};
   std::vector<uint8_t> out(static_cast<size_t>(width) * height * 4, 0);
   depth = std::max(0, std::min(200, depth));
   glassReflection = std::max(0, std::min(200, glassReflection));
   const double depthAmount = depth / 100.0;
   int insetWidth = 18 + static_cast<int>(std::lround(7.0 * depthAmount));
   insetWidth = std::min(insetWidth, std::max(1, std::min(width, height) / 2 - 1));
   // Inset shadow: concentric 1 px rectangles of decreasing alpha
   for (int i = 0; i < insetWidth; i++)
   {
      const int alpha = std::max(0, std::min(255, static_cast<int>(std::lround(150.0 * depthAmount * (1.0 - static_cast<double>(i) / insetWidth)))));
      if (alpha <= 0)
         continue;
      const double a = alpha / 255.0;
      const int x0 = i, y0 = i, x1 = std::max(i, width - 1 - i), y1 = std::max(i, height - 1 - i);
      for (int x = x0; x <= x1; x++)
      {
         BlendPixel(out.data() + (static_cast<size_t>(y0) * width + x) * 4, 0, 0, 0, a);
         if (y1 != y0)
            BlendPixel(out.data() + (static_cast<size_t>(y1) * width + x) * 4, 0, 0, 0, a);
      }
      for (int y = y0 + 1; y < y1; y++)
      {
         BlendPixel(out.data() + (static_cast<size_t>(y) * width + x0) * 4, 0, 0, 0, a);
         if (x1 != x0)
            BlendPixel(out.data() + (static_cast<size_t>(y) * width + x1) * 4, 0, 0, 0, a);
      }
   }
   const int reflectionAlpha = std::max(0, std::min(255, static_cast<int>(std::lround(150.0 * glassReflection / 100.0))));
   if (reflectionAlpha <= 0)
      return out;
   // Vertical shine gradient over the top third
   const int shineX = 16, shineY = 8, shineW = std::max(2, width - 32), shineH = std::max(16, height / 3);
   for (int y = shineY; y < std::min(height, shineY + shineH); y++)
   {
      const double t = static_cast<double>(y - shineY) / shineH;
      const double a = (reflectionAlpha / 255.0) * (1.0 - t);
      for (int x = shineX; x < std::min(width, shineX + shineW); x++)
         BlendPixel(out.data() + (static_cast<size_t>(y) * width + x) * 4, 235, 248, 255, a);
   }
   DrawLine(out, width, height, 35.0, 19.0, width - 85.0, 7.0, 2.5, 255, 255, 255, reflectionAlpha / 255.0);
   DrawLine(out, width, height, width / 3.0, 10.0, width / 2.0, height / 3.0, 5.0, 210, 238, 255, (reflectionAlpha / 2) / 255.0);
   return out;
}

// Box blur of a double map (radius r), separable, matching BlurDoubleMap
static std::vector<double> BlurDoubleMap(const std::vector<double>& values, const int width, const int height, const int radius)
{
   std::vector<double> horizontal(values.size()), output(values.size());
   for (int y = 0; y < height; y++)
   {
      const int row = y * width;
      double sum = 0.0;
      int left = 0, right = std::min(width - 1, radius);
      for (int x = left; x <= right; x++)
         sum += values[row + x];
      for (int x = 0; x < width; x++)
      {
         horizontal[row + x] = sum / (right - left + 1);
         const int removeX = x - radius, addX = x + radius + 1;
         if (removeX >= 0)
         {
            sum -= values[row + removeX];
            left++;
         }
         if (addX < width)
         {
            sum += values[row + addX];
            right++;
         }
      }
   }
   for (int x = 0; x < width; x++)
   {
      double sum = 0.0;
      int top = 0, bottom = std::min(height - 1, radius);
      for (int y = top; y <= bottom; y++)
         sum += horizontal[static_cast<size_t>(y) * width + x];
      for (int y = 0; y < height; y++)
      {
         output[static_cast<size_t>(y) * width + x] = sum / (bottom - top + 1);
         const int removeY = y - radius, addY = y + radius + 1;
         if (removeY >= 0)
         {
            sum -= horizontal[static_cast<size_t>(removeY) * width + x];
            top++;
         }
         if (addY < height)
         {
            sum += horizontal[static_cast<size_t>(addY) * width + x];
            bottom++;
         }
      }
   }
   return output;
}

std::vector<uint8_t> RenderDynamicLight(const uint8_t* source, const int sourceBpp, const uint8_t* field, const int width, const int height, const DynamicLightParams& params)
{
   if (source == nullptr || field == nullptr || width <= 0 || height <= 0)
      return {};
   const size_t count = static_cast<size_t>(width) * height;
   // Transmission map: luminance detail against its local average
   std::vector<double> luminance(count);
   for (size_t i = 0; i < count; i++)
   {
      const uint8_t* const p = source + i * sourceBpp;
      luminance[i] = std::max(0.0, std::min(1.0, (p[0] * 0.299 + p[1] * 0.587 + p[2] * 0.114) / 255.0));
   }
   const std::vector<double> localAverage = BlurDoubleMap(luminance, width, height, 4);
   const double contrast = std::max(0.0, std::min(3.0, params.transmissionContrast / 100.0));
   const double detailStrength = 0.45 + contrast * 1.15;
   const double midpointStrength = 0.72 + contrast * 0.28;
   std::vector<double> transmission(count);
   for (size_t i = 0; i < count; i++)
      transmission[i] = std::max(0.025, std::min(1.0, std::pow(std::max(0.0, luminance[i]), 0.82) * midpointStrength + (luminance[i] - localAverage[i]) * detailStrength));

   std::vector<uint8_t> out(count * 4, 0);
   const double normalizedIntensity = std::max(0.0, std::min(16.0, params.exposure / 100.0));
   const double exposureStrength = normalizedIntensity * 1.18 + std::pow(std::max(0.0, normalizedIntensity - 3.0), 1.28) * 0.58;
   const double protect = std::max(0.0, std::min(1.0, params.highlightProtection / 100.0));
   const double shadowLift = std::max(0.0, std::min(1.0, params.darkAreaLift / 100.0));
   const double sat = std::max(0.0, std::min(2.0, params.saturation / 100.0));
   const double lightR = params.colorR * 255.0, lightG = params.colorG * 255.0, lightB = params.colorB * 255.0;
   for (size_t i = 0; i < count; i++)
   {
      const double f = field[i] / 255.0;
      if (f <= 0.0)
         continue;
      const uint8_t* const p = source + i * sourceBpp;
      const double r = p[0], g = p[1], b = p[2];
      const double sourceAlpha = (sourceBpp == 4 ? p[3] : 255) / 255.0;
      const double lum = (r * 0.299 + g * 0.587 + b * 0.114) / 255.0;
      const double blackProtection = SmoothStep((std::max(r, std::max(g, b)) - 18.0) / 30.0);
      if (blackProtection <= 0.0)
         continue;
      // Front light reflected by the artwork: scale the pigment by the incident light
      const double response = 0.72 + shadowLift * (1.0 - lum) * 0.28;
      const double detailResponse = 0.8 + transmission[i] * 0.4;
      double localEnergy = exposureStrength * f * response * detailResponse;
      localEnergy *= 1.0 - protect * lum * 0.8;
      const double gain = 1.0 + localEnergy * 0.38;
      double rr = r * (1.0 + (gain - 1.0) * lightR / 255.0);
      double gg = g * (1.0 + (gain - 1.0) * lightG / 255.0);
      double bb = b * (1.0 + (gain - 1.0) * lightB / 255.0);
      // Above 800 % a highlight only white burst on already bright detail
      const double burstMix = SmoothStep((normalizedIntensity - 8.0) / 8.0) * f * SmoothStep((lum - 0.55) / 0.45);
      rr += (255.0 - rr) * burstMix;
      gg += (255.0 - gg) * burstMix;
      bb += (255.0 - bb) * burstMix;
      const double litLum = rr * 0.299 + gg * 0.587 + bb * 0.114;
      rr = litLum + (rr - litLum) * sat;
      gg = litLum + (gg - litLum) * sat;
      bb = litLum + (bb - litLum) * sat;
      if (params.flasherStyle == 2)
      {
         rr = lightR;
         gg = lightG;
         bb = lightB;
      }
      uint8_t* const d = out.data() + i * 4;
      d[0] = ClampByte(rr);
      d[1] = ClampByte(gg);
      d[2] = ClampByte(bb);
      d[3] = ClampByte(f * sourceAlpha * blackProtection * 255.0);
   }
   ApplyTemperatureGrade(out.data(), width, height, params.temperature);
   return out;
}

}
