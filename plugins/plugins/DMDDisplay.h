// license:GPLv3+

#pragma once

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "plugins/VPXPlugin.h"        // VPXDisplayRenderStyle, VPXPluginAPI
#include "plugins/ControllerPlugin.h" // CTLPI_DISPLAY_HARDWARE_*

// Map a controller display hardware hint (CTLPI_DISPLAY_HARDWARE_*) to the matching DMD render style.
// Shared so every DMD consumer (B2S, B2S legacy, ...) selects the style identically.
inline VPXDisplayRenderStyle DMDStyleFromHardware(uint32_t hardware)
{
   switch (hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK)
   {
   case CTLPI_DISPLAY_HARDWARE_RED_LED:     return VPXDisplayRenderStyle::VPXDMDStyle_RedLED;
   case CTLPI_DISPLAY_HARDWARE_RGB_LED:     return VPXDisplayRenderStyle::VPXDMDStyle_GenLED;
   case CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY: return VPXDisplayRenderStyle::VPXDMDStyle_CRT;
   case CTLPI_DISPLAY_HARDWARE_NEON_PLASMA:
   default:                                 return VPXDisplayRenderStyle::VPXDMDStyle_Plasma;
   }
}

// sRGB to linear, for glass ambient values that are authored in sRGB but passed to the shader in linear.
inline float DMDsRGBToLinear(float x) { return (x <= 0.04045f) ? (x * (1.f / 12.92f)) : powf(x * (1.f / 1.055f) + (0.055f / 1.055f), 2.4f); }

// Lazily loads, caches and frees the DMD glass overlay texture (the dirty-glass cover shipped with the
// ScoreView plugin). Hold one as a member and call Get() each frame; the texture is loaded once.
class DMDGlassTexture final
{
public:
   DMDGlassTexture() = default;
   ~DMDGlassTexture() { if (m_tex && m_vpxApi) m_vpxApi->DeleteTexture(m_tex); }
   DMDGlassTexture(const DMDGlassTexture&) = delete;
   DMDGlassTexture& operator=(const DMDGlassTexture&) = delete;

   VPXTexture Get(VPXPluginAPI* vpxApi)
   {
      if (m_loaded)
         return m_tex;
      m_loaded = true;
      m_vpxApi = vpxApi;
      VPXInfo vpxInfo {};
      vpxApi->GetVpxInfo(&vpxInfo);
      const std::filesystem::path path = std::filesystem::path(vpxInfo.path) / "plugins" / "scoreview" / "layouts" / "Glass-Dirty-1.webp";
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file.is_open())
         return nullptr;
      const std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      std::vector<uint8_t> buffer(static_cast<size_t>(size));
      if (file.read(reinterpret_cast<char*>(buffer.data()), size))
         m_tex = vpxApi->CreateTexture(buffer.data(), static_cast<int>(size));
      return m_tex;
   }

private:
   VPXTexture m_tex = nullptr;
   bool m_loaded = false;
   VPXPluginAPI* m_vpxApi = nullptr;
};
