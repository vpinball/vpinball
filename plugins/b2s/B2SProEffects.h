// license:GPLv3+

#pragma once

#include <cstdint>
#include <vector>

// B2S Pro image kernels (ports of B2S Pro's Reel3DEffect and DynamicArtworkLighting, Ken Wildman),
// working on straight alpha RGBA8 buffers. CPU work, meant to be cached: a reel digit renders once
// per (image, backlight) state, the window overlay once per display.
namespace B2S::ProEffects {

// Realistic 3D EM reel digit: the source artwork resampled to width x height, shaded as a drum
// (depth 0..200), backlit (brightness 0..400 %), colour graded by temperature (2000..6500 K)
std::vector<uint8_t> RenderReel(const uint8_t* source, int sourceWidth, int sourceHeight, int sourceBpp, int width, int height, int brightness, int temperature, int depth);

// Glass window overlay spanning a whole reel display: inset shadow (depth) and glass reflection (0..200)
std::vector<uint8_t> ReelWindowOverlay(int width, int height, int depth, int glassReflection);

// Light temperature grade of a lit frame (2000..6500 K, 4000 = neutral), luminance preserving
void ApplyTemperatureGrade(uint8_t* rgba, int width, int height, int kelvin);

// Dynamic artwork lighting: the authored alpha light field applied to a snippit frame (same size),
// returning the lit overlay to composite over the artwork (alpha = field x artwork alpha x black protection)
struct DynamicLightParams
{
   int exposure = 100; // 0..1600 (%)
   float colorR = 1.f, colorG = 1.f, colorB = 1.f;
   int flasherStyle = 1;
   int saturation = 100; // 0..200
   int highlightProtection = 0; // 0..100
   int darkAreaLift = 0; // 0..100
   int transmissionContrast = 140; // 0..300
   int temperature = 4000; // 2000..6500
};
std::vector<uint8_t> RenderDynamicLight(const uint8_t* source, int sourceBpp, const uint8_t* field, int width, int height, const DynamicLightParams& params);

}
