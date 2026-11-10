// license:GPLv3+

#pragma once

#include <SDL3/SDL_hidapi.h>
#include <SDL3/SDL_hints.h>

// Enumerates HID devices through SDL, including those that are not game controllers (Open Pinball Device, U-Shock, ...).
// SDL_hid_enumerate() only lists game controllers by default (SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS): the filter is lifted
// for this call only, then reset to its default or environment value (VPX does not set it otherwise). The hint applies at once,
// and SDL's joystick enumeration runs on the same main thread, so it never sees the lifted filter.
// Requires SDL_hid_init(). Free the result with SDL_hid_free_enumeration().
inline SDL_hid_device_info* EnumerateAllHIDDevices(unsigned short vendorId = 0, unsigned short productId = 0)
{
   SDL_SetHintWithPriority(SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS, "0", SDL_HINT_OVERRIDE);
   SDL_hid_device_info* const devices = SDL_hid_enumerate(vendorId, productId);
   SDL_ResetHint(SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS);
   return devices;
}
