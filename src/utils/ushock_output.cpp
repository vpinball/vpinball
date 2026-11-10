// This is some very ancient (and, if we're being frank, very
// awful) code dedicated to the Ultracade Ushock device, which
// (one gathers from reading the code) was an early, primitive
// predecessor of HID output controller devices like the LedWiz.
// The Ushock has a single output for a pinball knocker.  This
// code implements bespoke HID access to the Ushock specifically
// (through SDL's HIDAPI, formerly through the Win32 HID API).
// ("PBW" is another name seen a lot here, which
// I believe stands for PinballBallWizard, which as I recall is
// another product from the same vendor.  Or maybe the same one.
// Who knows)
//
// The entire concept of this module has long since been
// superseded by DOF (DirectOutput Framework), which is why
// you don't see a bunch of similar modules for LedWiz's,
// PacLed's, and a dozen other devices, thank goodness.
//
// This code was formerly named with the extremely presumptuous
// prefix "hid_" for all of its public functions.  It is most
// certainly not "hid" code generically; it happens to *use*
// HID to do its one extremely narrow job, but naming everything
// here "hid_xxx" makes about as much sense as naming it "c_xxx"
// because it's written in C.  So, as of 9/2024, it has been
// renamed to more properly indicate its function.  (This wasn't
// just because its old naming was so piquing, but rather because
// the old naming was creating a collision with the hidapi library,
// which also uses hid_xxx as its naming convention.  Of the two,
// I think hidapi has the far better claim on the name)

#include "core/stdafx.h"

#include "utils/ushock_output.h"

// Desktop only, like Open Pinball Device: on mobile platforms, SDL's HID access may trigger USB or Bluetooth permission prompts
#if !defined(__LIBVPINBALL__) && !defined(__EMSCRIPTEN__)
#define USHOCK_SUPPORTED
#include "input/HIDEnumeration.h"
#include <hid-report-parser/hid_report_parser.h>
#endif

#ifdef USHOCK_SUPPORTED
static bool sHidInitialized = false;
static SDL_hid_device* sDevice = nullptr;
static size_t sOutputReportSize = 0; // In bytes, including the report ID byte
static uint32_t sLastWritten = 0;

// Size of output report 0 in bytes, including the report ID byte (what the Win32 HID API reported as OutputReportByteLength),
// or 0 if the interface has no output report or its report descriptor could not be read
static size_t GetOutputReportSize(SDL_hid_device* device)
{
   constexpr size_t maxReportDescriptorSize = 4096; // HID_API_MAX_REPORT_DESCRIPTOR_SIZE
   vector<uint8_t> descriptor(maxReportDescriptorSize);
   const int size = SDL_hid_get_report_descriptor(device, descriptor.data(), descriptor.size());
   if (size <= 0)
      return 0;
   hidrp::DescriptorParser parser;
   hidrp::ReportSizeScanner<hidrp::SCAN_OUTPUT> sizeScanner; // (scans only input reports by default)
   parser.Parse(descriptor.data(), size, &sizeScanner);
   const size_t bits = sizeScanner.ReportSize(hidrp::ReportType::output, 0);
   return bits == 0 ? 0 : (bits + 7) / 8 + 1;
}

static void WriteOutputs(const uint8_t mask)
{
   // Report ID 0, then the output mask, zero padded to the output report size (at least 2 bytes)
   uint8_t buffer[1024] = {};
   buffer[0] = 0;
   buffer[1] = mask;
   // Blocks until the write completes (up to 1s on Windows, 200ms with the former Win32 code): drop a device whose write
   // fails, so that it cannot stall the physics loop on every following output change
   if (SDL_hid_write(sDevice, buffer, std::clamp<size_t>(sOutputReportSize, 2, sizeof(buffer))) < 0)
   {
      PLOGE << "Ushock (PBW controller) write failed, disconnecting";
      SDL_hid_close(sDevice);
      sDevice = nullptr;
   }
   sLastWritten = mask;
}
#endif

static uint32_t sMask = 0;

#define KNOCK_PERIOD_ON  50
#define KNOCK_PERIOD_OFF 500

static int sKnock;
static int sKnockState;
static uint32_t sKnockStamp;

void ushock_output_init()
{
   sMask = 0;
   sKnock = 0;
   sKnockState = 0;
   sKnockStamp = 0;

#ifdef USHOCK_SUPPORTED
   // 0x4b4 == Ultracade, 0x6470 == Ushock
   if (SDL_hid_init() != 0)
      return;
   sHidInitialized = true;

   // The device may expose several HID interfaces: use the first one with an output report (or else the first one that opens)
   SDL_hid_device_info* const devices = EnumerateAllHIDDevices(0x04b4, 0x6470);
   SDL_hid_device* fallback = nullptr;
   for (const SDL_hid_device_info* cur = devices; cur != nullptr && sDevice == nullptr; cur = cur->next)
   {
      SDL_hid_device* const device = SDL_hid_open_path(cur->path);
      if (device == nullptr)
         continue;
      if (const size_t size = GetOutputReportSize(device); size > 1)
      {
         sDevice = device;
         sOutputReportSize = size;
      }
      else if (fallback == nullptr)
         fallback = device;
      else
         SDL_hid_close(device);
   }
   SDL_hid_free_enumeration(devices);
   if (sDevice == nullptr)
   {
      sDevice = fallback;
      sOutputReportSize = 0;
   }
   else if (fallback != nullptr)
      SDL_hid_close(fallback);

   if (sDevice == nullptr)
   {
      PLOGD << "Ushock (PBW controller) not found";
      return;
   }
   PLOGI << "Connected to Ushock (PBW controller), output report size " << sOutputReportSize;

   // Start with all outputs off
   WriteOutputs(0);
#endif
}

// This is the main interface to turn output on and off.
// Once set, the value will remain set until another set call is made.
// The output_mask parameter uses any combination of HID_OUTPUT enum.
void ushock_output_set(const uint8_t output_mask, const bool on)
{
   // Check if the outputs are being turned on.
   if (on)
   {
      sMask |= output_mask;
   }
   else
   {
      sMask &= ~output_mask;
   }
}

void ushock_output_knock(const int count)
{
   if (count)
   {
      sKnock = count;
      sKnockStamp = g_pplayer->m_time_msec;
      sKnockState = 1;
   }
}

void ushock_output_update(const uint32_t cur_time_msec)
{
#ifdef USHOCK_SUPPORTED
   uint8_t mask = (uint8_t)(sMask & 0xff);

   if (sKnockStamp)
   {
      if (cur_time_msec - sKnockStamp < (uint32_t)(sKnockState ? KNOCK_PERIOD_ON : KNOCK_PERIOD_OFF))
      {
         mask |= sKnockState ? (uint8_t)HID_OUTPUT_KNOCKER : (uint8_t)0x00;
      }
      else
      {
         if (sKnockState)
         {
            sKnockState = 0;
            --sKnock;
            if (sKnock == 0)
            {
               sKnockStamp = 0;
            }
            else sKnockStamp = cur_time_msec;
         }
         else
         {
            sKnockState = 1;
            sKnockStamp = cur_time_msec;
         }
      }
   }

   if (sDevice != nullptr && mask != sLastWritten)
      WriteOutputs(mask);
#endif
}

void ushock_output_shutdown()
{
#ifdef USHOCK_SUPPORTED
   if (sDevice != nullptr)
   {
      SDL_hid_close(sDevice);
      sDevice = nullptr;
   }
   if (sHidInitialized)
   {
      SDL_hid_exit();
      sHidInitialized = false;
   }
#endif
}
