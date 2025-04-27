// license:GPLv3+

#pragma once

#include "unordered_dense.h"

class BaseTexture;
class IEditable;

//
// General URI scheme allowing to link part properties to script/plugin resources
// 
//////////////////////////////////////////////////////////////////////////////////////////
// Warning: This is a not yet stable, pre-alpha syntax which will be validated over time
//////////////////////////////////////////////////////////////////////////////////////////
//
// The syntax is: scheme://authority/path?query
// 
// - 'default' scheme is used to access 'default' ressources (DMD, alpha displays, etc.) while 'script' scheme is used to 
//   access ressources defined through script:
//   . '/dmd' authority (no path) is used to access the default DMD
//   . '/alpha' authority (no path) is used to access the default alpha segment displays
//     - 'sub=xx' is used to select the xx-th display (0-based)
//     - 'elt=yy' is used to select the yy-th element of the display (0-based)
//   When using 'script' scheme, the following additional query parameters are available:
//     - 'src=xx' specifies part name from which the resources is gathered
//
//   examples:
//   - default://dmd               => default DMD
//   - default://alpha?sub=0&elt=0 => first element of the first display block of the default alpha display group
//   - script://dmd?src=Table1     => DMD defined from the script on part Table1 (the main table)
//
// - 'plugin' scheme is used to access plugin ressources. The authority is the plugin id. It is followed by the query path:
//   . '/getstate' path is used to get the state of a plugin contributed ressource
//     - 'src=xx' specifies requested resource type between display, alpha and lamp
//     - 'id=xx' specifies id of the resource, defaults to 0
//     For output type, the following additional query parameters are available:
//       - 'io=xx' where xx is the output id
//       - 'value=xx' where xx specify a specific property (for example, brightness or tint for a lamp)
//     For display type, the following additional query parameters are available:
//       - 'x=xx' and 'y=yy' specify the coordinates of the pixel to gather
//     For alpha type, the following additional query parameters are available:
//     - 'sub=xx' is used to select the xx-th display (0-based)
//     - 'elt=yy' is used to select the yy-th element of the display (0-based)
//
//   examples:
//   - plugin://pinmame/getstate?src=display&id=0                      => DMD or display, with id #0, provided by pinmame plugin
//   - plugin://pinmame/getstate?src=alpha&id=1                        => Alphanumeric segment display group #1
//   - plugin://pinmame/getstate?src=output&io=2                       => Output #2 of IO block #0, default property (relative luminance for lamps)
//   - plugin://pinmame/getstate?src=output&id=0&io=2&value=brightness => Output #2 Relative luminance
//   - plugin://pinmame/getstate?src=output&id=0&io=2&value=tint       => Output #2 Tint
//   - plugin://pinmame/getstate?src=display&id=0&x=0&y=0              => Intensity/Color of the top left dot of the first display
//
class ResURIResolver final
{
public:
   ResURIResolver();
   ~ResURIResolver();

   void RequestVisualUpdate();
   void RequestPhysicsUpdate();

   // Controlled device state
   float GetControllerOutput(CtlResId id, const unsigned int outputId);
   float GetOutput(const string &link);

   // DMDs and video displays
   struct ControllerDisplay
   {
      DmdSrcId dmdId;
      int frameId = -1;
      BaseTexture *frame = nullptr;
   };
   ControllerDisplay GetControllerDisplay(CtlResId id);
   BaseTexture *GetDisplay(const string &link);

   // Segment displays
   struct ControllerSegDisplay
   {
      CtlResId segId;
      unsigned int nElements;
      float *frame = nullptr;
      vector<vector<SegElementType>> displays;
   };
   ControllerSegDisplay GetControllerSegDisplay(CtlResId id);
   struct SegDisplay
   {
      float *frame = nullptr;
      vector<SegElementType> displays;
   };
   SegDisplay GetSegDisplay(const string &link);

private:
   unsigned int m_visualTimestamp = 0;
   unsigned int m_physicsTimestamp = 0;

   static void OnDevSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   vector<DevSrcId> m_controllerDevices;
   unsigned int m_getDevSrcMsgId, m_onDevChangedMsgId;
   typedef std::function<float(const string &)> outputCacheLambda;
   ankerl::unordered_dense::map<string, outputCacheLambda> m_outputCache;

   static void OnDmdSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   unsigned int m_getDmdMsgId, m_getDmdSrcMsgId, m_onDmdChangedMsgId;
   vector<ControllerDisplay> m_controllerDisplays;
   DmdSrcId m_defaultDmdId { 0 };
   typedef std::function<BaseTexture *(const string &)> texCacheLambda;
   ankerl::unordered_dense::map<string, texCacheLambda> m_texCache;

   static void OnSegSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   unsigned int m_getSegMsgId, m_getSegSrcMsgId, m_onSegChangedMsgId;
   vector<ControllerSegDisplay> m_controllerSegDisplays;
   CtlResId m_defaultSegId { 0 };
   typedef std::function<SegDisplay(const string &)> segCacheLambda;
   ankerl::unordered_dense::map<string, segCacheLambda> m_segCache;
};
