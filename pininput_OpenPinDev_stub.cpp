// license:GPLv3+
//
// Open Pinball Device Stub
//
// This module is a set of stub functions for Open Pinball Device support
// in VP, for platforms that don't provide sufficient low-level HID access
// to implement this feature.  This module can be substituted in the
// CMakeLists.txt and/or other build scripts for pininput_OpenPinDev.cpp,
// which has the full implementation for platforms where the third-party
// hidapi library is available.


#include "stdafx.h"
#include "pininput.h"

void PinInput::InitOpenPinballDevices() { }
void PinInput::TerminateOpenPinballDevices() { }
void PinInput::ReadOpenPinballDevices(const U32 /*cur_time_msec*/) { }
