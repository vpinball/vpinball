// license:GPLv3+
//
// Open Pinball Device support in VP
//
// This is essentially a sub-module of pininput.cpp that breaks out the
// code for Open Pinball Device so that it can be conditionally included
// in the project, at the CMakeFiles/build script level.  Some of VP's
// target platforms (particularly the mobile platforms) have limited or
// no support for the sort of low-level HID access we need.

#include "core/stdafx.h"
#include "input/OpenPinDevHandler.h"
#include <list>
#include <hidapi/hidapi.h>
#include <hid-report-parser/hid_report_parser.h>



// Open Pinball Device input report v1.0 - input report structure.
//
// This struct defines the in-memory layout, for access from C++ code.
// The data transmitted across the USB connection must be interpreted
// with the exact byte layout in the specification, so DON'T cast a
// byte buffer from a USB read into this struct.  Instead, use the
// Load() method to convert the USB byte layout into the local
// struct representation.
struct OpenPinballDeviceReport
{
   uint64_t timestamp;      // timestamp, microseconds since an arbitrary zero point
   uint32_t genericButtons; // button states for 32 user-assigned on/off buttons
   uint32_t pinballButtons; // button states for pre-defined pinball simulator function buttons
   int16_t axNudge;         // instantaneous nudge acceleration, X axis (left/right)
   int16_t ayNudge;         // instantaneous nudge acceleration, Y axis (front/back)
   int16_t vxNudge;         // instantaneous nudge velocity, X axis
   int16_t vyNudge;         // instantaneous nudge velocity, Y axis
   int16_t plungerPos;      // current plunger position
   int16_t plungerSpeed;    // instantaneous plunger speed

   // load the struct from the USB byte format
   void LoadFromUSB(const uint8_t *p, size_t reportSize)
   {
      // clear all fields
      memset(this, 0, sizeof(*this));

      // Load fields until we exhaust the data.  We can stop as soon as we
      // run out of input bytes, since we've already cleared all of the
      // elements.
      Load(timestamp, p, reportSize) && Load(genericButtons, p, reportSize) && Load(pinballButtons, p, reportSize)
         && Load(axNudge, p, reportSize) && Load(ayNudge, p, reportSize) && Load(vxNudge, p, reportSize)
         && Load(vyNudge, p, reportSize) && Load(plungerPos, p, reportSize) && Load(plungerSpeed, p, reportSize);
   }

   // Load bytes from the USB byte format into a local native integer type.
   // This correctly translates from the packed little-endian wire format
   // into the local integer representation.  Note: ele must be pre-cleared
   // to zero before the call.
   template <typename T> static bool Load(T &ele, const uint8_t *&p, size_t &sizeRemaining)
   {
      // figure how many bytes we have to read for the complete type
      size_t nBytesInT = sizeof(T) / sizeof(uint8_t);
      if (sizeRemaining >= nBytesInT)
      {
         // we can complete the full read
         for (size_t i = 0; i < nBytesInT; ele |= static_cast<T>(*p++) << (i * 8), ++i)
            ;

         // deduct the size consumed and return success
         sizeRemaining -= nBytesInT;
         return true;
      }
      else
      {
         // we can't complete the read, so we've exhausted the input - clear
         // all remaining bytes (so that we don't try to interpret the remaining
         // bytes as a smaller type that occurs later in the struct) and return
         // end-of-file
         sizeRemaining = 0;
         return false;
      }
   }
};

// Open Pinball Device interface.  This wraps a hidapi device handle
// that we have open for reading from an OPD unit.
class OpenPinDev final
{
public:
   OpenPinDev(hid_device *hDevice, BYTE reportID, size_t reportSize, const wchar_t *deviceStructVersionString) :
       hDevice(hDevice), 
       reportID(reportID),
       reportSize(reportSize)
   {
      // Parse the device struct version string into a DWORD, with the major
      // version in the high word and the minor version in the low word.
      // This format can be compared against a reference version with a
      // simple DWORD comparison: deviceStructVersion > 0x00020005 tests
      // for something newer than 2.5.
      const wchar_t *dot = wcschr(deviceStructVersionString, L'.');
      deviceStructVersion = (ParseUInt(deviceStructVersionString) << 16) | (dot != nullptr ? ParseUInt(dot + 1) : 0);

      // put the read handle in non-blocking mode
      hid_set_nonblocking(hDevice, 1);

      // allocate space for the report
      buf.resize(reportSize);

      // Zero our internal report copy
      memset(&r, 0, sizeof(r));
   }

   ~OpenPinDev()
   {
      // close the device handle
      hid_close(hDevice);
   }

   // get a reference to the current report
   const OpenPinballDeviceReport &CurrentReport() const { return r; }

   // read a report into the internal report struct; returns true if a new report
   // was available
   bool ReadReport()
   {
      // Read reports until the buffer is empty, to be sure we have the latest.
      // The reason we loop as long as new reports are available is that HID
      // drivers on some platforms (such as Windows) buffer a number of reports,
      // and return the oldest buffered report on each call.  So the first report
      // we read could be relatively old - the HID polling cycle is typically
      // 8-10ms, and the HID driver might buffer tens of reports, so the oldest
      // report could be hundreds of milliseconds old.  On each call, therefore,
      // we need to keep reading reports until we catch up with the last report
      // available in the buffer, at which point we have the latest instantaneous
      // state of the device.
      //
      // While we're looping, we keep track of whether we've seen any new reports
      // at all, since it's entirely possible for the caller to invoke this before
      // a new report becomes available.
      bool isNewReport = false;
      for (;;)
      {
         // try reading - we're in non-blocking mode, so this will return
         // immediately with a length of zero if no reports are available
         buf[0] = reportID;
         int readResult = hid_read(hDevice, buf.data(), buf.size());

         // If we're out of data, or an error occurred, stop looping.  We
         // treat errors the same as no data available, since the error might
         // be temporary, in which case we should start getting data again as
         // soon as the fault is cleared.
         if (readResult <= 0)
            break;

         // Read completed.  Extract the Open Pinball Device struct from the
         // byte buffer, and flag that a new report is available.
         r.LoadFromUSB(&buf[1], readResult - 1);
         isNewReport = true;
      }

      // return the new-report status
      return isNewReport;
   }

protected:
   // parse a string representing a decimal integer value into a uint32_t
   static uint32_t ParseUInt(const wchar_t *p)
   {
      uint32_t acc = 0;
      for (; *p != 0 && iswdigit(*p); p++)
      {
         acc *= 10;
         acc += (*p - '0');
      }
      return acc;
   }

   // device handle (hidapi library type)
   hid_device *hDevice = nullptr;

   // HID report ID for the device.  This is sent as a prefix byte in each
   // report the device sends through the interface, to identify the report
   // descriptor (i.e., the struct type) associated with the report.
   uint8_t reportID;

   // The version of the Open Pinball Device report structure that the device
   // is using, as identified in the usage string in the report descriptor.
   // This is encoded with the major version number in the high 16 bits, and
   // the minor version number in the low 16 bits: "1.2" is 0x00010002.  We
   // use this format because you can compare two version numbers using a
   // native integer compare operation.
   uint32_t deviceStructVersion;

   // read buffer - space for an incoming report
   size_t reportSize;
   std::vector<BYTE> buf;

   // last report read
   OpenPinballDeviceReport r;
};


// Open Pinball Device context object
class OpenPinDevContext final
{
   friend class OpenPinDevHandler;

   // list of active devices
   std::list<std::unique_ptr<OpenPinDev>> m_openPinDevs;
};

// Initialize the Open Pinball Device interface.  Searches for active
// devices and adds them to our internal list.
OpenPinDevHandler::OpenPinDevHandler(PinInput &pininput)
   : m_pininput(pininput)
   , m_OpenPinDevContext(new OpenPinDevContext())
{
   PLOGI << "OpenPinDev input handler registered";

   // Get a list of available HID devices.  (Passing VID/PID 0/0 
   // to hid_enumerate() gives us all available HIDs.)
   auto *hEnum = hid_enumerate(0, 0);
   if (hEnum == nullptr)
   {
      // enumeration failed - no Open Pinball Devices are available
      return;
   }

   // scan the list
   for (auto *cur = hEnum; cur != nullptr; cur = cur->next)
   {
      // check for a generic Pinball Device usage (usage page 0x05 "Game
      // Controls", usage 0x02 "Pinball Device")
      constexpr unsigned short USAGE_PAGE_GAMECONTROLS = 0x05;
      constexpr unsigned short USAGE_GAMECONTROLS_PINBALLDEVICE = 0x02;
      if (cur->usage_page == USAGE_PAGE_GAMECONTROLS && cur->usage == USAGE_GAMECONTROLS_PINBALLDEVICE)
      {
         // It's at least a generic Pinball Device, which is a sort
         // of "base class" (loosely speaking) for Open Pinball Device.
         // To determine if it's an Open Pinball Device specifically,
         // we need to check the HID Input Report Descriptor to see
         // if it matches the OPD report structure.  The OPD report
         // has one Byte Array field with usage 0x00 (unknown/vendor-
         // defined) and an associated Usage String with a unique
         // signature.  The signature string is the real means of
         // positive identification - it's not formally a GUID, but
         // it's meant to be sufficiently long and distinctive to
         // serve the same purpose, as a self-selected universal ID
         // that no other device will ever duplicate by accident.

         // open the device so that we can read its report descriptor
         std::unique_ptr<hid_device, decltype(&hid_close)> hDevice(hid_open_path(cur->path), &hid_close);
         if (hDevice != nullptr)
         {
            // read the report descriptor
            std::unique_ptr<unsigned char[]> reportDescBuf(new unsigned char[HID_API_MAX_REPORT_DESCRIPTOR_SIZE]);
            unsigned char *rp = reportDescBuf.get();
            int rdSize = hid_get_report_descriptor(hDevice.get(), rp, HID_API_MAX_REPORT_DESCRIPTOR_SIZE);
            if (rdSize > 0)
            {
               // parse the usages
               hidrp::DescriptorParser parser;
               hidrp::UsageExtractor usageExtractor;
               hidrp::UsageExtractor::Report report;
               usageExtractor.ScanDescriptor(rp, rdSize, report);

               // scan the collections
               bool found = false;
               for (const auto &col : report.collections)
               {
                  // check for the generic USB "Pinball Device CA" type (Application Collection, usage page 5, usage 2)
                  if (col.type == hidrp::COLLECTION_TYPE_APPLICATION
                      && col.usage_page == USAGE_PAGE_GAMECONTROLS && col.usage == USAGE_GAMECONTROLS_PINBALLDEVICE)
                  {
                     // got it - scan the input fields in this collection
                     auto InputIndex = static_cast<int>(hidrp::ReportType::input);
                     for (const auto &f : col.fields[InputIndex])
                     {
                        // Check for an opaque byte array, usage 0x00 (undefined/vendor-specific),
                        // with an associated usage string that matches the OPD signature string.
                        constexpr size_t nStrBuf = 128;
                        wchar_t strBuf[nStrBuf];
                        if (f.usageRanges.size() == 1 && f.usageRanges.front().Equals(USAGE_PAGE_GAMECONTROLS, 0)
                           && f.stringRanges.size() == 1 && !f.stringRanges.front().IsRange()
                           && hid_get_indexed_string(hDevice.get(), f.stringRanges.front().GetSingle(), strBuf, nStrBuf) == 0
                           && wcsncmp(strBuf, L"OpenPinballDeviceStruct/", 24) == 0)
                        {
                           // matched
                           found = true;

                           // Figure the report size for the associated input report ID.  The
                           // report size scanner returns the combined size of all of the fields
                           // in the report in BITS, so take ceil(bits/8) to get the report size
                           // in bytes.  Then add 1 for the HID Report ID byte prefix that every
                           // HID report must include.  This gives us the size of the USB packets
                           // the device sends.
                           hidrp::ReportSizeScanner sizeScanner;
                           parser.Parse(rp, rdSize, &sizeScanner);
                           size_t reportSize = (sizeScanner.ReportSize(hidrp::ReportType::input, f.reportID) + 7) / 8 + 1;

                           // add it to the active device list, releasing ownership of the device
                           // handle to the list object
                           m_OpenPinDevContext->m_openPinDevs.emplace_back(
                               new OpenPinDev(hDevice.release(), f.reportID, reportSize, &strBuf[24]));

                           // stop searching - there should be only one match
                           break;
                        }
                     }
                  }

                  // stop as soon as we find a match
                  if (found)
                  {
                     PLOGI << "OpenPinballDevice found and registered.";
                     m_pininput.SetupJoyMapping(GetJoyId(0), PinInput::InputLayout::OpenPinDev);
                     break;
                  }
               }
            }
         }
      }
   }

   // done with the device list dev
   hid_free_enumeration(hEnum);
}

// Terminate the Open Pinball Device subsystem.  Closes all open
// devices and discards associated memory structures.
OpenPinDevHandler ::~OpenPinDevHandler()
{
   // discard the context object
   delete m_OpenPinDevContext;
   m_OpenPinDevContext = nullptr;
}

// Read input from the Open Pinball Device inputs
void OpenPinDevHandler::Update()
{
   // Combined report.  In keeping with Visual Pinball's treatment of
   // multiple gamepads, we merge the input across devices if there are
   // multiple Pinball Devices sending us data.
   OpenPinballDeviceReport cr = {};

   // read input from each device
   bool isNewReport = false;
   for (auto &p : m_OpenPinDevContext->m_openPinDevs)
   {
      // check for a new report
      if (p->ReadReport())
         isNewReport = true;

      // Merge the data into the combined struct.  For the accelerometer
      // and plunger analog quantities, just arbitrarily pick the last
      // input that's sending non-zero values.  Devices that don't have
      // those sensors attached will send zeroes, so this strategy yields
      // sensible results for the sensible case where the user only has
      // one plunger and one accelerometer, but they're attached to
      // separate Open Pinball Device microcontrollers.  If the user has
      // multiple accelerometers in the system, our merge strategy will
      // arbitrarily pick whichever one enumerated last, which isn't
      // necessarily a sensible result, but that seems fair enough
      // because the user's actual configuration isn't sensible either.
      // I mean, what do they expect us to do with two accelerometer
      // inputs?  Note that this is a different situation from the
      // traditional multiple-joysticks case, because in the case of
      // joysticks, there are plenty of good reasons to have more than
      // one attached.  One might be set up as a pinball device, and two
      // more *actual joysticks* might be present as well because the
      // user also plays some non-pinball video games.  Joysticks are
      // generic: we can't tell from the HID descriptor if it's an
      // accelerometer pretending to be a joystick, vs an actual
      // joystick.  The Pinball Device definition doesn't suffer from
      // that ambiguity.  We can be sure that an accelerometer there
      // is an accelerometer, so there aren't any valid use cases where
      // you'd have two or more of them.
      //
      // Merge the buttons by ORing all of the button masks together.
      // If the user has configured the same button number on more than
      // one device, they probably actually want the buttons to perform
      // the same logical function, so ORing them yields the right result.
      const auto &r = p->CurrentReport();
      if (r.axNudge != 0)
         cr.axNudge = r.axNudge;
      if (r.ayNudge != 0)
         cr.ayNudge = r.ayNudge;
      if (r.vxNudge != 0)
         cr.vxNudge = r.vxNudge;
      if (r.vyNudge != 0)
         cr.vyNudge = r.vyNudge;
      if (r.plungerPos != 0)
         cr.plungerPos = r.plungerPos;
      if (r.plungerSpeed != 0)
         cr.plungerSpeed = r.plungerSpeed;
      cr.genericButtons |= r.genericButtons;
      cr.pinballButtons |= r.pinballButtons;
   }

   // if there were no reports, there's no need to update the player
   if (!isNewReport)
      return;

   // Axis scaling factor.  All Open Pinball Device analog axes are
   // INT16's (-32768..+32767).  The VP functional axes are designed
   // for joystick input, so we must rescale to VP's joystick scale.
   constexpr int scaleFactor = (2 * JOYRANGEMX) / 65536;

   // Process the analog axis inputs.
   // 
   // The UI was hacked to have a custom axis #9 corresponding to OpenPinDev (instead of an axis)
   // This allows to detect the axis associated with the OpenPinDev device when the mapping is
   // done (but this is somewhat wrong as this approach is not consistent and not scalable to all
   // existing hardware => this needs to be removed).
   // 
   // We simply push the value of the singleton device using virtual custom axis 10..13.
   // TODO This should be reimplemented in favor of the general handling. as virtual axis does not 
   // follow the overall scheme and makes the definition inconsistent, and won't scale with other hardware.
   {
      // Nudge X input - use velocity or acceleration input, according to the user preferences
      int const val = (g_pplayer->IsAccelInputAsVelocity() ? cr.vxNudge : cr.axNudge) * scaleFactor;
      m_pininput.PushJoystickAxisEvent(GetJoyId(0), 10, -val);
   }
   {
      // Nudge Y input - use velocity or acceleration input, according to the user preferences
      int const val = (g_pplayer->IsAccelInputAsVelocity() ? cr.vyNudge : cr.ayNudge) * scaleFactor;
      m_pininput.PushJoystickAxisEvent(GetJoyId(0), 11, -val);
   }
   {
      // Plunger position input
      const int val = cr.plungerPos * scaleFactor;
      m_pininput.PushJoystickAxisEvent(GetJoyId(0), 12, -val);
   }
   {
      // Plunger speed input
      int const val = cr.plungerSpeed * scaleFactor;
      m_pininput.PushJoystickAxisEvent(GetJoyId(0), 13, -val);
   }

   // Check for button state changes to the generic buttons, which map
   // to the like-numbered joystick buttons.  Fire a joystick button
   // event for each button with a change of state since our last read.
   if (cr.genericButtons != m_openPinDev_generic_buttons)
   {
      // Visit each button.  VP's internal joystick buttons are
      // numbered #1 to #32.
      for (int buttonNum = 1, bit = 1; buttonNum <= 32; ++buttonNum, bit <<= 1)
      {
         // check for a state change
         const bool isDown = (cr.genericButtons & bit) != 0;
         const bool wasDown = (m_openPinDev_generic_buttons & bit) != 0;
         if (isDown != wasDown)
            m_pininput.PushJoystickButtonEvent(GetJoyId(0), buttonNum, isDown);
      }

      // remember the new button state
      m_openPinDev_generic_buttons = cr.genericButtons;
   }
   if (cr.pinballButtons != m_openPinDev_pinball_buttons)
   {
      // mapping from Open Pinball Device mask bits to VP/VPM keys
      static constexpr struct KeyMap
      {
         uint32_t mask; // bit for the key in OpenPinballDeviceReportStruct::pinballButtons
         EnumAssignKeys mappedAction; // mapped action, or EnumAssignKeys::eCKeys if a direct VPM key is used instead
         BYTE vpmKey; // DIK_xxx key ID of VPM key, or 0 if an action assignment is used instead
      } keyMap[] = {
         { 0x00000001, eStartGameKey },             // Start (start game)
         { 0x00000002, eExitGame },                 // Exit (end game)
         { 0x00000004, eAddCreditKey },             // Coin 1 (left coin chute)
         { 0x00000008, eAddCreditKey2 },            // Coin 2 (middle coin chute)
         { 0x00000010, eCKeys, DIK_5 },             // Coin 3 (right coin chute)
         { 0x00000020, eCKeys, DIK_6 },             // Coin 4 (fourth coin chute/dollar bill acceptor)
         { 0x00000040, eCKeys, DIK_2 },             // Extra Ball/Buy-In
         { 0x00000080, ePlungerKey },               // Launch Ball
         { 0x00000100, eLockbarKey },               // Fire button (lock bar top button)
         { 0x00000200, eLeftFlipperKey },           // Left flipper button primary switch
         { 0x00000400, eRightFlipperKey },          // Right flipper button primary switch
         { 0x00000800, eStagedLeftFlipperKey, 0 },  // Left flipper button secondary switch (upper flipper actuator)
         { 0x00001000, eStagedRightFlipperKey, 0 }, // Right flipper button secondary switch (upper flipper actuator)
         { 0x00002000, eLeftMagnaSave },            // Left MagnaSave button
         { 0x00004000, eRightMagnaSave },           // Right MagnaSave button
         { 0x00008000, eMechanicalTilt },           // Tilt bob
         { 0x00010000, eCKeys, DIK_HOME },          // Slam tilt switch
         { 0x00020000, eCKeys, DIK_END },           // Coin door position switch
         { 0x00040000, eCKeys, DIK_7 },             // Service panel Cancel
         { 0x00080000, eCKeys, DIK_8 },             // Service panel Down
         { 0x00100000, eCKeys, DIK_9 },             // Service panel Up
         { 0x00200000, eCKeys, DIK_0 },             // Service panel Enter
         { 0x00400000, eLeftTiltKey },              // Left Nudge
         { 0x00800000, eCenterTiltKey },            // Forward Nudge
         { 0x01000000, eRightTiltKey },             // Right Nudge
         { 0x02000000, eVolumeUp },                 // Audio volume up
         { 0x04000000, eVolumeDown },               // Audio volume down
      };

      // Visit each pre-assigned button
      const KeyMap *m = keyMap;
      for (size_t i = 0; i < std::size(keyMap); ++i, ++m)
      {
         // check for a state change
         uint32_t const mask = m->mask;

         const bool isDown = (cr.pinballButtons & mask) != 0;
         const bool wasDown = (m_openPinDev_pinball_buttons & mask) != 0;
         if (isDown != wasDown)
         {
            if (m->mappedAction != EnumAssignKeys::eCKeys)
               m_pininput.FireActionEvent(m->mappedAction, isDown);
            else
               PinInput::FireGenericKeyEvent(isDown ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m->vpmKey);
         }
      }

      // remember the new button state
      m_openPinDev_pinball_buttons = cr.pinballButtons;
   }
}
