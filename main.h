#pragma once

#include <Windows.h>

#include "Richedit.h"
#include "RichOle.h"

#include <atlcom.h>

#ifdef VBA
#define APC_IMPORT_MIDL
#endif

#include <atlbase.h>
#include <atlhost.h>
//extern CComModule _Module;
#ifdef VBA
#include <apcCpp.h>
using namespace MSAPC;
#endif

#include "math.h"

#include "commctrl.h"

#include <activdbg.h>

#include <atlctl.h>
#include <atlhost.h>

#define DIRECTINPUT_VERSION 0x0700
#define D3D_OVERLOADS 1
#include "dinput.h"
#include "ddraw.h"
#include "d3d.h"
#include "dsound.h"

#include <ocidl.h>	// Added by ClassView

#include "stdio.h"
#include "wchar.h"

#include "HELPERS.H"

#include <xmmintrin.h>

#include <vector>
#include <string>
using namespace std;

#include "def.h"
#include "resource.h"

class HitTimer;
class PinBinary;

#define INCLUDE_DEPRECATED_FEATURES 1 // Scintilla
#include "Scintilla.h"
#include "scilexer.h"

#include "pintypes.h"
#include "memutil.h"
#include "disputil.h"

#include "dispid.h"

#include "variant.h"
#include "vector.h"
#include "exvector.h"
#include "vectorsort.h"
#include "VBATest.h"
#include "OLEAUTO.h"
#include "regutil.h"

#include "IDebug.h"

#include "EventProxy.h"

#include "worker.h"

#include "XAudPlayer.h"
#include "media/FileIO.h"
#include "PinUndo.h"
#include "ISelect.h"
#include "IEditable.h"
#include "PropBrowser.h"
#include "CodeView.h"

#include "media/lzwreader.h"
#include "media/lzwwriter.h"
#include "media/wavread.h"
#include "PinInput.h"
#include "PinSound.h"
#include "PinImage.h"
#include "PinBinary.h"

#include "VPinball.h"
#include "TableGlobal.h"
#include "PinTable.h"

#include "Mesh.h"
#include "pin/collide.h"
#include "Pin3D.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"
#include "shadowsur.h"

#include "BallEx.h"

#include "pin/draw.h"
#include "pin/collideex.h"
#include "pin/ball.h"
#include "pin/hittimer.h"
#include "pin/hitable.h"
#include "pin/hitflipper.h"
#include "pin/hitplunger.h"
#include "pin/player.h"

#include "DragPoint.h"
#include "timer.h"
#include "surface.h"
#include "flipper.h"
#include "plunger.h"
#include "textbox.h"
#include "dispreel.h"
#include "lightseq.h"
#include "comcontrol.h"
#include "bumper.h"
#include "Trigger.h"
#include "Light.h"
#include "Kicker.h"
#include "Decal.h"
#include "Primitive.h"
#include "Gate.h"
#include "Spinner.h"
#include "Ramp.h"
#include "flasher.h"
#include "display.h"
#include "mixer.h"
#include "hid.h"
#include "plumb.h"

#include "wintimer.h"

#include "slintf.h"

#include "extern.h"
