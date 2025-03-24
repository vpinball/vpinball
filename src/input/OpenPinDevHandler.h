#pragma once

#include "pininput.h"

class OpenPinDevHandler final : public InputHandler
{
public:
   explicit OpenPinDevHandler(PinInput& pininput);
   ~OpenPinDevHandler() override;
   void Update() override;

private:
   static constexpr uint64_t GetJoyId(const int index) { return static_cast<uint64_t>(0x400000000) | static_cast<uint64_t>(index); }

private:
   PinInput& m_pininput;

   // Open Pinball Device context.  This is an opaque object managed
   // by the OPD implementation module, so that the whole implementation
   // can be detached at the build script level.
   OpenPinDevContext* m_OpenPinDevContext = nullptr;
   
   // Open Pinball Device button status, for detecting button up/down events
   uint32_t m_openPinDev_generic_buttons = 0;
   uint32_t m_openPinDev_pinball_buttons = 0;
   //bool m_openPinDev_flipper_l = false;
   //bool m_openPinDev_flipper_r = false;
};
