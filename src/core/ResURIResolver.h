// license:GPLv3+

#pragma once

#include "robin_hood.h"

class BaseTexture;
class IEditable;

// This is the initial implementation of a general link URI system to allow to link part properties to script/plugin items
// Warning: for the time being, this is a pre-alpha syntax which will be validated over time
// default://dmd => default DMD (script DMD or plugin selected one with a width >= 128)
// plugin://vpx/getstate?src=display&id=0 => DMD provided by VPX
// plugin://pinmame/getstate?src=display&id=0 => DMD or display, provided by pinmame plugin
// plugin://pinmame/getstate?src=alpha&id=1 => Alphanumeric segment display state
// plugin://pinmame/getstate?src=lamp&id=2&value=brightness => Intensity
// plugin://pinmame/getstate?src=lamp&id=2&value=tint => Tint
// plugin://pinmame/getstate?src=display&id=0&x=0&y=0 => Intensity/Color of the top left dot of the first display
class ResURIResolver final
{
public:
   ResURIResolver();

   void ClearCache();

   BaseTexture *GetDisplay(const string &link, const IEditable *context);

   struct SegDisplay
   {
      float *frame = nullptr;
      vector<SegElementType> displays;
   };
   SegDisplay GetSegDisplay(const string &link, const IEditable *context);

private:
   typedef std::function<BaseTexture *(const string &, const IEditable *)> texCacheLambda;
   robin_hood::unordered_map<string, texCacheLambda> m_texCache;
   typedef std::function<SegDisplay(const string &, const IEditable *)> segCacheLambda;
   robin_hood::unordered_map<string, segCacheLambda> m_segCache;
};
