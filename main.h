#pragma once
#include "def.h"
#include "resource.h"

//#include <atlimpl.cpp>

class HitTimer;
class PinBinary;

// if defined, we are compiling in changes which should only be active
// for ultracade products and are changes never intended to be released
// to the public either because they have no value to the public in general
//
// EG: bugfixes and so on should not be in #ifdef ULTRACADE blocks...
#define ULTRACADE

#define INCLUDE_DEPRECATED_FEATURES 1
#include "ScinAll.h"

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

#include "cdjpeg.h"
#include "XAudPlayer.h"
#include "media/FileIO.h"
#include "PinUndo.h"
#include "ISelect.h"
#include "IEditable.h"
#include "PropBrowser.h"
#include "CodeView.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"
#include "shadowsur.h"

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

#include "BallEx.h"

#include "pin/draw.h"
#include "pin/collideex.h"
#include "pin/ball.h"
#include "pin/hittimer.h"
#include "pin/hitable.h"
#include "pin/wall.h"
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
#include "display.h"
#include "mixer.h"
#include "hid.h"
#include "plumb.h"

#include "wintimer.h"

#include "slintf.h"

#include "extern.h"
//
