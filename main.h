#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <MMSystem.h>

#ifdef USE_DINPUT8
#define DIRECTINPUT_VERSION 0x0800
#else
#define DIRECTINPUT_VERSION 0x0700
#endif
#include <dinput.h>
#include <dsound.h>

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include "minid3d9.h"

//#include <richedit.h>
//#include <atlcom.h>
#include <atlbase.h>
//#include <atlhost.h>
#include <atlctl.h>
//#include <assert.h>

//#include <commctrl.h>

//#include "stdio.h"
//#include "wchar.h"

#include <OLEAUTO.h>

#include <wincrypt.h>

#include <intrin.h>
#include <xmmintrin.h>

#include <vector>
#include <string>
#include <algorithm>
using std::string;
using std::vector;

#include "helpers.H"

#include "def.h"

#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/bbox.h"

#include "resource.h"

#include "memutil.h"
#include "disputil.h"

#include "dispid.h"

#include "variant.h"
#include "vector.h"
#include "vectorsort.h"
#include "vpinball_i.h"
#include "regutil.h"

#include "idebug.h"

#include "eventproxy.h"

#include "worker.h"

#include "xaudplayer.h"
#include "media/fileio.h"
#include "pinundo.h"
#include "iselect.h"

#include "ieditable.h"
#include "propbrowser.h"
#include "codeview.h"

#include "media/lzwreader.h"
#include "media/lzwwriter.h"

#include "media/wavread.h"

#include "pininput.h"
#include "pinsound.h"
#include "pinbinary.h"

#include "vpinball.h"
#include "pintable.h"

#include "mesh.h"
#include "pin/collide.h"
#include "pin3d.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"

#include "ballex.h"

#include "pin/collideex.h"
#include "pin/ball.h"
#include "pin/hittimer.h"
#include "pin/hitable.h"
#include "pin/hitflipper.h"
#include "pin/hitplunger.h"
#include "pin/player.h"

#include "color.h"

#include "dragpoint.h"
#include "timer.h"
#include "surface.h"
#include "flipper.h"
#include "plunger.h"
#include "textbox.h"
#include "dispreel.h"
#include "lightseq.h"
#include "bumper.h"
#include "trigger.h"
#include "light.h"
#include "kicker.h"
#include "decal.h"
#include "primitive.h"
#include "hittarget.h"
#include "gate.h"
#include "spinner.h"
#include "ramp.h"
#include "flasher.h"
#include "rubber.h"
#include "mixer.h"
#include "hid.h"
#include "plumb.h"

#include "kdtree.h"

#include "wintimer.h"

#include "slintf.h"
#include "trace.h"

#include "extern.h"

#include "editablereg.h"

__forceinline float getBGxmult()
{
   const bool useAA = (g_pplayer->m_AA && (g_pplayer->m_ptable->m_useAA == -1)) || (g_pplayer->m_ptable->m_useAA == 1);
   return (float)g_pplayer->m_width * (float)(1.0 / EDITOR_BG_WIDTH)
      * (useAA ? 2.0f : 1.0f);
}

__forceinline float getBGymult()
{
   return getBGxmult() /
      (((float)g_pplayer->m_screenwidth / (float)g_pplayer->m_screenheight) / (float)((double)EDITOR_BG_WIDTH / EDITOR_BG_HEIGHT));
}
