// license:GPLv3+

#pragma once

#include <string>
#include <vector>
#include <unordered_dense.h>

#include "ControllerPlugin.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Warning: This is a not yet stable, pre-alpha syntax which will be validated over time
//////////////////////////////////////////////////////////////////////////////////////////

//
// General URI scheme for accessing shared states exposed through ControllerPlugin API
// 
// This class needs 2 third party dependencies:
// - unordered_dense.h from https://github.com/martinus/unordered_dense
// - simple-uri-parser.f from https://github.com/jholloc/simple-uri-parser
// 
// This class handles multithreading:
// - the class instance must be created on the MsgAPI thread
// - getters may be called from any thread, they are synchronized against changes (happening on the MsgAPI thread)
// 
// The syntax is: scheme://authority/path?query
// 
// - 'ctrl' scheme allows to access states exposed through the generic controller API (see ControllerPlugin.h)
//   Authority must be either the id of a given plugin or 'default' in which case a default source is selected
//   . '/display' path allows to access rectangular displays (DMD and video displays)
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'override=xx' specify how variants are selected ('all' is the default): [Unimplemented]
//       . 'override=no' disable variants
//       . 'override=color' only select variants with improved coloring
//       . 'override=scale' only select variants with improved resolution
//       . 'override=all' select highest quality variant (color, resolution, ...)
//     - 'x=xx', 'y=yy' and 'channel=zz' specify the coordinates of the pixel to gather a channel between 'r', [Unimplemented]
//       'g', 'b' or 'lum' (relative luminance which is the default)
//   . '/seg' path allows to access alphanumeric segment displays
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'sub=xx' is used to select the xx-th element of the display (0-based)
//   . '/state' path allows to access game states
//     - 'group=xx' where xx is the device group defined by the plugin
//     - 'io=xx' where xx is the device mapping id (user friendly number, defined by the plugin, unique inside the device group)
//     - 'name=xx' where xx is the user friendly name of the state
//
//   examples:
//   - ctrl://default/display                  => Default DMD or display
//   - ctrl://flexdmd/display                  => FlexDMD first DMD
//   - ctrl://pinmame/display?x=0&y=0          => Relative luminance of the top left dot of PinMAME's first display [Unimplemented]
//   - ctrl://pinmame/display?override=no      => Untouched version of PinMAME first display (no colorization or upscaling) [Unimplemented]
//   - ctrl://pinmame/state?group=1&io=11      => Element #11 of PinMAME state group #1 (solenoid #11)
//   - ctrl://pinmame/state?group=2&io=1       => Element #1 of PinMAME state group #2 (first GI string)
//   - ctrl://pinmame/seg?id=1                 => Alphanumeric segment display #1
//   - ctrl://pinmame/seg?id=1&sub=0           => Alphanumeric first element (block of segments forming a number/character) of segment display #1
//

namespace PinballPlugin
{

class ResURIResolver final
{
public:
   ResURIResolver(const MsgPluginAPI &msgAPI, unsigned int endpointId, bool trackDisplays, bool trackSegDisplays, bool trackStates);
   ~ResURIResolver();

   float GetFloatState(const std::string &link);
   
   struct DisplayState
   {
      const DisplaySrcId *source = nullptr;
      DisplayFrame state;
   };
   DisplayState GetDisplayState(const std::string &link);
   // Same, for elements that render dots (DMDs): resolves as above, then reports no frame for a CRT or LCD source.
   // Anything that can legitimately show a video screen (a flasher in Display mode) keeps using GetDisplayState()
   DisplayState GetDmdDisplayState(const std::string &link);
   std::string DumpDisplaySources() const;
   static const DisplaySrcId *GetDefaultDisplaySource(const std::vector<DisplaySrcId> &sources);
   
   struct SegDisplayState
   {
      const SegSrcId *source;
      SegDisplayFrame state;
   };
   SegDisplayState GetSegDisplayState(const std::string &link);

private:
   const MsgPluginAPI& m_msgAPI;
   const unsigned int m_endpointId;

   mutable std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<StateSrcId>> m_stateSources;
   using floatCacheLambda = std::function<float(const std::string &)>;
   ankerl::unordered_dense::map<std::string, floatCacheLambda> m_floatCache;

   mutable std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<SegSrcId>> m_segSources;
   using segCacheLambda = std::function<SegDisplayState(const std::string &)>;
   ankerl::unordered_dense::map<std::string, segCacheLambda> m_segCache;

   mutable std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<DisplaySrcId>> m_displaySources;
   using displayCacheLambda = std::function<DisplayState(const std::string &)>;
   ankerl::unordered_dense::map<std::string, displayCacheLambda> m_displayCache;

   static std::string trim_string(const std::string &str);
   static bool try_parse_int(const std::string &str, int &value);
};

};
