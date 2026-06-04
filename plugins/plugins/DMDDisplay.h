// license:GPLv3+

#pragma once

#include <cstdint>

#include "plugins/VPXPlugin.h"        // VPXDisplayRenderStyle
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
