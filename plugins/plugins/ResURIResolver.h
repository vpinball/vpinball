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
//   . '/state' path allows to access game states
//     - 'id=xx' specify id of the resource (defaults to 0), unsupported when using 'default' instead of a plugin
//     - 'grp=xx' where xx is the device group defined by the plugin
//     - 'io=xx' where xx is the device mapping id (user friendly number, defined by the plugin, unique inside the device group)
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
   ResURIResolver(const MsgPluginAPI &msgAPI, unsigned int endpointId, bool trackDisplays, bool trackSegDisplays, bool trackStates);
   ~ResURIResolver();

   float GetFloatState(const std::string &link);
   
   struct DisplayState
   {
      const DisplaySrcId *source = nullptr;
      DisplayFrame state;
   };
   DisplayState GetDisplayState(const std::string &link);

   // Skip-unchanged gate for consumers that pull a DisplayState once per
   // rendered frame and copy it into a texture. Sources only advance
   // DisplayFrame.frameId when the frame content actually changed, so a
   // consumer that remembers the id it last uploaded can skip the copy (and
   // the GPU upload behind it) instead of re-uploading a static image at
   // render rate.
   //
   // One gate per consumer-owned target texture. Never share a gate between
   // two consumers, even when they feed the same texture: the first puller's
   // upload would suppress the second one's, and whichever window pulls later
   // goes stale while everything else looks healthy.
   //
   // frameId is compared for INEQUALITY, not ordering: id sequences restart
   // with their source (new table, ROM restart, source swap) and can wrap, so
   // "different from the id THIS gate last uploaded" is the only safe trigger.
   //
   // The target texture pointer and the source identity/geometry are part of
   // the key so a cached id can never suppress the first upload into a texture
   // object this gate has not fed yet (still null, recreated, reallocated), an
   // upload from a different source that happens to reuse an id, or an upload
   // after the source changed shape under an unchanged id. Callers must call
   // MarkUploaded AFTER the upload, with the texture pointer as it is AFTER
   // the call — the upload itself may reallocate the texture.
   //
   // Skipping is safe against GPU-side texture lifetime: the last uploaded
   // frame remains in the consumer's CPU-side texture, which is what the
   // renderer's texture cache rebuilds from if its GPU copy is evicted or the
   // device is reset — so a skipped upload can never leave the display blank
   // or frozen, only identical to the frame already shown.
   struct DisplayUploadGate
   {
      const void *texture = nullptr; // consumer's target texture as of the last upload
      uint64_t srcId = 0;
      unsigned int frameId = 0;
      unsigned int width = 0;
      unsigned int height = 0;
      unsigned int frameFormat = 0;

      bool NeedsUpload(const DisplayState &display, const void *targetTexture) const
      {
         return targetTexture == nullptr || targetTexture != texture
            || display.source->id.id != srcId || display.state.frameId != frameId
            || display.source->width != width || display.source->height != height
            || display.source->frameFormat != frameFormat;
      }
      void MarkUploaded(const DisplayState &display, const void *targetTexture)
      {
         texture = targetTexture;
         srcId = display.source->id.id;
         frameId = display.state.frameId;
         width = display.source->width;
         height = display.source->height;
         frameFormat = display.source->frameFormat;
      }
   };
   void SetDisplayFilter(const std::function<bool(const DisplaySrcId& src)>& filter);
   std::string DumpDisplaySources() const;
   
   struct SegDisplayState
   {
      const SegSrcId *source;
      SegDisplayFrame state;
   };
   SegDisplayState GetSegDisplayState(const std::string &link);

private:
   const MsgPluginAPI& m_msgAPI;
   const unsigned int m_endpointId;

   const unsigned int m_getStateSrcMsgId;
   const unsigned int m_onStateChangedMsgId;
   static void OnStateSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   std::vector<StateSrcId> m_stateSources;

   using floatCacheLambda = std::function<float(const std::string &)>;
   ankerl::unordered_dense::map<std::string, floatCacheLambda> m_floatCache;

   const unsigned int m_getSegSrcMsgId;
   const unsigned int m_onSegChangedMsgId;
   static void OnSegSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   std::vector<SegSrcId> m_segSources;

   using segCacheLambda = std::function<SegDisplayState(const std::string &)>;
   ankerl::unordered_dense::map<std::string, segCacheLambda> m_segCache;

   const unsigned int m_getDisplaySrcMsgId;
   const unsigned int m_onDisplayChangedMsgId;
   static void OnDisplaySrcChanged(const unsigned int msgId, void *userData, void *msgData);
   std::vector<DisplaySrcId> m_displaySources;
   std::function<bool(const DisplaySrcId& src)> m_displayFilter;

   using displayCacheLambda = std::function<DisplayState(const std::string &)>;
   ankerl::unordered_dense::map<std::string, displayCacheLambda> m_displayCache;

   static std::string trim_string(const std::string &str);
   static bool try_parse_int(const std::string &str, int &value);
};
