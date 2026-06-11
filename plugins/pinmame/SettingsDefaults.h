// license:GPLv3+

#pragma once

#include "common.h"

#include <unordered_map>

namespace PinMAME {

// Default values matching VPinMAME settings that were never explicitly set,
// so table scripts relying on VPinMAME defaults behave the same as on
// Windows. Values taken from PinMAME src/windows/config.c (pinmame_opts)
// and src/win32com/VPinMAMEConfig.cpp (vpinmame_opts).
inline bool TryGetSettingDefault(const string& lowerKey, int& value)
{
   static const std::unordered_map<string, int> defaults = {
      // DMD colors and intensities
      { "dmd_red"s, 255 },
      { "dmd_green"s, 88 },
      { "dmd_blue"s, 32 },
      { "dmd_perc0"s, 20 },
      { "dmd_perc33"s, 33 },
      { "dmd_perc66"s, 67 },
      { "dmd_only"s, 0 },
      { "dmd_compact"s, 0 },
      { "dmd_antialias"s, 50 },
      { "dmd_colorize"s, 0 },
      { "dmd_red66"s, 225 },
      { "dmd_green66"s, 15 },
      { "dmd_blue66"s, 193 },
      { "dmd_red33"s, 6 },
      { "dmd_green33"s, 0 },
      { "dmd_blue33"s, 214 },
      { "dmd_red0"s, 0 },
      { "dmd_green0"s, 0 },
      { "dmd_blue0"s, 0 },
      { "dmd_opacity"s, 100 },
      // DMD window
      { "dmd_border"s, 1 },
      { "dmd_title"s, 1 },
      { "dmd_pos_x"s, 0 },
      { "dmd_pos_y"s, 0 },
      { "dmd_width"s, 0 },
      { "dmd_height"s, 0 },
      { "dmd_doublesize"s, 0 },
      { "showpindmd"s, 0 },
      { "showwindmd"s, 1 },
      // Sound
      { "sound"s, 1 },
      { "samples"s, 1 },
      { "resampling_quality"s, 0 },
      { "sound_mode"s, 0 },
      { "force_stereo"s, 0 },
      // Misc
      { "rol"s, 0 },
      { "ignore_rom_crc"s, 0 },
      { "cabinet_mode"s, 0 },
      { "threadpriority"s, 1 },
      { "synclevel"s, 0 },
      { "fastframes"s, -1 },
   };
   const auto it = defaults.find(lowerKey);
   if (it == defaults.end())
      return false;
   value = it->second;
   return true;
}

}
