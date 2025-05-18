// license:GPLv3+

#pragma once

#include "unordered_dense.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Warning: This is a not yet stable, pre-alpha syntax which will be validated over time
//////////////////////////////////////////////////////////////////////////////////////////

//
// General URI scheme for accessing shared states
// 
// The syntax is: scheme://authority/path?query
// 
// - 'ctrl' scheme allows to access states exposed through the generic controller API (see ControllerPlugin.h)
//   Authority must be either the id of a given plugin or 'default' in which case a default source is selected
//   . '/display' path allows to access rectangular displays (DMD and video displays)
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'override=xx' specify how variants are selected ('all' is the default):
//       . 'override=no' disable variants
//       . 'override=color' only select variants with improved coloring
//       . 'override=scale' only select variants with improved resolution
//       . 'override=all' select highest quality variant (color, resolution, ...)
//     - 'x=xx', 'y=yy' and 'channel=zz' specify the coordinates of the pixel to gather a channel between 'r',
//       'g', 'b' or 'lum' (relative luminance which is the default)
//   . '/seg' path allows to access alphanumeric segment displays
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'sub=xx' is used to select the xx-th element of the display (0-based)
//   . '/device' path allows to access controlled device states
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'grp=xx' where xx is the device group defined by the plugin
//     - 'io=xx' where xx is the device mapping id (user friendly number, defined by the plugin, unique inside the device group)
//   . '/input' path allows to access input states
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'grp=xx' where xx is the input group defined by the plugin
//     - 'io=xx' where xx is the input mapping id (user friendly number, defined by the plugin, unique inside the input group)
//
//   examples:
//   - ctrl://default/display                  => Default DMD or display
//   - ctrl://pinmame/seg?id=1                 => Alphanumeric segment display #1
//   - ctrl://pinmame/seg?id=1&sub=0           => Alphanumeric first element (block of segments forming a number/character) of segment display #1
//   - ctrl://flexdmd/display                  => FlexDMD first DMD
//   - ctrl://pinmame/input?grp=1&io=11        => Input #11 of PinMAME input group #1 (switch matrix 1.1)
//   - ctrl://pinmame/input?grp=2&io=1         => Input #1 of PinMAME input group #2 (first dip switch)
//   - ctrl://pinmame/display?x=0&y=0          => Relative luminance of the top left dot of PinMAME's first display
//   - ctrl://pinmame/display?override=no      => Untouched version of PinMAME first display (no colorization or upscaling)
//


class ResURIResolver final
{
public:
   ResURIResolver(const MsgPluginAPI& msgAPI, unsigned int endpointId, bool trackDisplays, bool trackSegDisplays, bool trackInputs, bool trackDevices);
   ~ResURIResolver();

   float GetFloatState(const string &link);
   
   struct DisplayState
   {
      DisplaySrcId *source = nullptr;
      DisplayFrame state;
   };
   DisplayState GetDisplayState(const string &link);
   
   struct SegDisplayState
   {
      const SegSrcId *source;
      SegDisplayFrame state;
   };
   SegDisplayState GetSegDisplayState(const string &link);

private:
   const MsgPluginAPI& m_msgAPI;
   const unsigned int m_endpointId;

   unsigned int GetPluginEndpoint(const string &pluginId) const;

   const unsigned int m_getDevSrcMsgId, m_onDevChangedMsgId;
   static void OnDevSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   std::vector<DevSrcId> m_deviceSources;

   const unsigned int m_getInputSrcMsgId, m_onInputChangedMsgId;
   static void OnInputSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   std::vector<InputSrcId> m_inputSources;

   typedef std::function<float(const string &)> floatCacheLambda;
   ankerl::unordered_dense::map<string, floatCacheLambda> m_floatCache;

   const unsigned int m_getSegSrcMsgId, m_onSegChangedMsgId;
   static void OnSegSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   vector<SegSrcId> m_segSources;

   typedef std::function<SegDisplayState(const string &)> segCacheLambda;
   ankerl::unordered_dense::map<string, segCacheLambda> m_segCache;

   const unsigned int m_getDisplaySrcMsgId, m_onDisplayChangedMsgId;
   static void OnDisplaySrcChanged(const unsigned int msgId, void *userData, void *msgData);
   vector<DisplaySrcId> m_displaySources;

   typedef std::function<DisplayState(const string &)> displayCacheLambda;
   ankerl::unordered_dense::map<string, displayCacheLambda> m_displayCache;
};
