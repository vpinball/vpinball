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
   OpenPinDev(hid_device *hDevice, uint8_t reportID, size_t reportSize, const wchar_t *deviceStructVersionString, uint16_t deviceId) :
       hDevice(hDevice), 
       reportID(reportID),
       reportSize(reportSize),
       deviceId(deviceId)
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

         OpenPinballDeviceReport prev_r = r;

         // Read completed.  Extract the Open Pinball Device struct from the
         // byte buffer, and flag that a new report is available.
         r.LoadFromUSB(&buf[1], readResult - 1);

         if (onNewReport)
            onNewReport(this, prev_r, r);

         isNewReport = true;
      }

      // return the new-report status
      return isNewReport;
   }

   uint16_t GetDeviceId() const { return deviceId; }

   void SetOnNewReportHandler(std::function<void(const OpenPinDev *const pindev, const OpenPinballDeviceReport &prevReport, const OpenPinballDeviceReport &report)> handler)
   {
      onNewReport = handler;
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

   // Id given by InputManager
   uint16_t deviceId = 0xFFFF;

   // On new report handler
   std::function<void(const OpenPinDev * const pindev, const OpenPinballDeviceReport &prevReport, const OpenPinballDeviceReport &report)> onNewReport;

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
   std::vector<uint8_t> buf;

   // last report read and the one before
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
OpenPinDevHandler::OpenPinDevHandler(InputManager &pininput)
   : m_inputManager(pininput)
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
                           && strBuf == L"OpenPinballDeviceStruct/"s)
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

                           // Register device to the input manager
                           uint16_t deviceId = m_inputManager.RegisterDevice("OpenPinDev", InputManager::DeviceType::OpenPinDev, "OpenPinDev");
                           m_inputManager.RegisterElementName(deviceId, false, 0, "Start Game");
                           m_inputManager.RegisterElementName(deviceId, false, 1, "Quit Game");
                           m_inputManager.RegisterElementName(deviceId, false, 2, "Coin");
                           m_inputManager.RegisterElementName(deviceId, false, 3, "Coin 2");
                           m_inputManager.RegisterElementName(deviceId, false, 4, "Coin 3");
                           m_inputManager.RegisterElementName(deviceId, false, 5, "Coin 4");
                           m_inputManager.RegisterElementName(deviceId, false, 6, "Extra Ball");
                           m_inputManager.RegisterElementName(deviceId, false, 7, "Launch Ball");
                           m_inputManager.RegisterElementName(deviceId, false, 8, "Fire Button");
                           m_inputManager.RegisterElementName(deviceId, false, 9, "Left Flipper");
                           m_inputManager.RegisterElementName(deviceId, false, 10, "Right Flipper");
                           m_inputManager.RegisterElementName(deviceId, false, 11, "Left Staged Flipper");
                           m_inputManager.RegisterElementName(deviceId, false, 12, "Right Staged Flipper");
                           m_inputManager.RegisterElementName(deviceId, false, 13, "Left Magna");
                           m_inputManager.RegisterElementName(deviceId, false, 14, "Right Magna");
                           m_inputManager.RegisterElementName(deviceId, false, 15, "Tilt");
                           m_inputManager.RegisterElementName(deviceId, false, 16, "Slam tilt");
                           m_inputManager.RegisterElementName(deviceId, false, 17, "Coin door");
                           m_inputManager.RegisterElementName(deviceId, false, 18, "Service Cancel");
                           m_inputManager.RegisterElementName(deviceId, false, 19, "Service Down");
                           m_inputManager.RegisterElementName(deviceId, false, 20, "Service Up");
                           m_inputManager.RegisterElementName(deviceId, false, 21, "Service Enter");
                           m_inputManager.RegisterElementName(deviceId, false, 22, "Left Nudge");
                           m_inputManager.RegisterElementName(deviceId, false, 23, "Forward Nudge");
                           m_inputManager.RegisterElementName(deviceId, false, 24, "Right Nudge");
                           m_inputManager.RegisterElementName(deviceId, false, 25, "Audio Up");
                           m_inputManager.RegisterElementName(deviceId, false, 26, "Audio Down");
                           for (int i = 0; i < 32; i++)
                              m_inputManager.RegisterElementName(deviceId, false, static_cast<uint16_t>(0x0100 | i), "Button #" + std::to_string(i));
                           m_inputManager.RegisterElementName(deviceId, true, 0x0200, "Plunger Position");
                           m_inputManager.RegisterElementName(deviceId, true, 0x0201, "Plunger Speed");
                           m_inputManager.RegisterElementName(deviceId, true, 0x0202, "Nudge X Acceleration");
                           m_inputManager.RegisterElementName(deviceId, true, 0x0203, "Nudge Y Acceleration");
                           m_inputManager.RegisterElementName(deviceId, true, 0x0204, "Nudge X Speed");
                           m_inputManager.RegisterElementName(deviceId, true, 0x0205, "Nudge Y Speed");
                           auto defaultMapping = [this, deviceId](
                              std::function<bool(const vector<ButtonMapping>&, unsigned int)> mapButton, //
                              std::function<bool(const SensorMapping&, SensorMapping::Type type, bool isLinear)> mapPlunger, //
                              std::function<bool(const SensorMapping&, const SensorMapping&)> mapNudge)
                           {
                              bool success = true;
                              success &= mapButton(ButtonMapping::Create(deviceId, 0), m_inputManager.GetStartActionId()); // Start (start game)
                              success &= mapButton(ButtonMapping::Create(deviceId, 1), m_inputManager.GetExitGameActionId()); // Exit (end game)
                              success &= mapButton(ButtonMapping::Create(deviceId, 2), m_inputManager.GetAddCreditActionId(0)); // Coin 1 (left coin chute)
                              success &= mapButton(ButtonMapping::Create(deviceId, 3), m_inputManager.GetAddCreditActionId(1)); // Coin 2 (middle coin chute)
                              success &= mapButton(ButtonMapping::Create(deviceId, 4), m_inputManager.GetAddCreditActionId(2)); // Coin 3 (right coin chute)
                              success &= mapButton(ButtonMapping::Create(deviceId, 5), m_inputManager.GetAddCreditActionId(3)); // Coin 4 (fourth coin chute/dollar bill acceptor)
                              success &= mapButton(ButtonMapping::Create(deviceId, 6), m_inputManager.GetExtraBallActionId()); // Extra Ball/Buy-In
                              success &= mapButton(ButtonMapping::Create(deviceId, 7), m_inputManager.GetLaunchBallActionId()); // Launch Ball
                              success &= mapButton(ButtonMapping::Create(deviceId, 8), m_inputManager.GetLockbarActionId()); // Fire button (lock bar top button)
                              success &= mapButton(ButtonMapping::Create(deviceId, 9), m_inputManager.GetLeftFlipperActionId()); // Left flipper button primary switch
                              success &= mapButton(ButtonMapping::Create(deviceId, 10), m_inputManager.GetRightFlipperActionId()); // Right flipper button primary switch
                              success &= mapButton(ButtonMapping::Create(deviceId, 11), m_inputManager.GetStagedLeftFlipperActionId()); // Left flipper button secondary switch (upper flipper actuator)
                              success &= mapButton(ButtonMapping::Create(deviceId, 12), m_inputManager.GetStagedRightFlipperActionId()); // Right flipper button secondary switch (upper flipper actuator)
                              success &= mapButton(ButtonMapping::Create(deviceId, 13), m_inputManager.GetLeftMagnaActionId()); // Left MagnaSave button
                              success &= mapButton(ButtonMapping::Create(deviceId, 14), m_inputManager.GetRightMagnaActionId()); // Right MagnaSave button
                              success &= mapButton(ButtonMapping::Create(deviceId, 15), m_inputManager.GetTiltActionId()); // Tilt bob
                              success &= mapButton(ButtonMapping::Create(deviceId, 16), m_inputManager.GetSlamTiltActionId()); // Slam tilt switch
                              success &= mapButton(ButtonMapping::Create(deviceId, 17), m_inputManager.GetCoinDoorActionId()); // Coin door position switch
                              success &= mapButton(ButtonMapping::Create(deviceId, 18), m_inputManager.GetServiceActionId(0)); // Service panel Cancel
                              success &= mapButton(ButtonMapping::Create(deviceId, 19), m_inputManager.GetServiceActionId(1)); // Service panel Down
                              success &= mapButton(ButtonMapping::Create(deviceId, 20), m_inputManager.GetServiceActionId(2)); // Service panel Up
                              success &= mapButton(ButtonMapping::Create(deviceId, 21), m_inputManager.GetServiceActionId(3)); // Service panel Enter
                              success &= mapButton(ButtonMapping::Create(deviceId, 22), m_inputManager.GetLeftNudgeActionId()); // Left Nudge
                              success &= mapButton(ButtonMapping::Create(deviceId, 23), m_inputManager.GetCenterNudgeActionId()); // Forward Nudge
                              success &= mapButton(ButtonMapping::Create(deviceId, 24), m_inputManager.GetRightNudgeActionId()); // Right Nudge
                              success &= mapButton(ButtonMapping::Create(deviceId, 25), m_inputManager.GetVolumeUpActionId()); // Audio volume up
                              success &= mapButton(ButtonMapping::Create(deviceId, 26), m_inputManager.GetVolumeDownActionId()); // Audio volume down
                              success &= mapPlunger(SensorMapping::Create(deviceId, 0x200, SensorMapping::Type::Position), SensorMapping::Type::Position, true); // Plunger position
                              success &= mapPlunger(SensorMapping::Create(deviceId, 0x201, SensorMapping::Type::Velocity), SensorMapping::Type::Velocity, true); // Plunger speed
                              success &= mapNudge(SensorMapping::Create(deviceId, 0x204, SensorMapping::Type::Velocity), SensorMapping::Create(deviceId, 0x205, SensorMapping::Type::Velocity)); // Nudge speed
                              return success;
                           };
                           m_inputManager.RegisterDefaultMapping(deviceId, defaultMapping);

                           // Setup new device
                           OpenPinDev* pinDev = new OpenPinDev(hDevice.release(), f.reportID, reportSize, &strBuf[24], deviceId);
                           pinDev->SetOnNewReportHandler(
                              [this](const OpenPinDev *const pindev, const OpenPinballDeviceReport &prevReport, const OpenPinballDeviceReport &report)
                           {
                              uint64_t timestampNs = report.timestamp * 1000ULL;
                              for (unsigned int buttonNum = 1, bit = 1; buttonNum <= 27; ++buttonNum, bit <<= 1)
                              {
                                 const bool isDown = (report.pinballButtons & bit) != 0;
                                 const bool wasDown = (prevReport.pinballButtons & bit) != 0;
                                 if (isDown != wasDown)
                                    m_inputManager.PushButtonEvent(pindev->GetDeviceId(), static_cast<uint16_t>(buttonNum), timestampNs, isDown);
                              }

                              for (unsigned int buttonNum = 1, bit = 1; buttonNum <= 32; ++buttonNum, bit <<= 1)
                              {
                                 const bool isDown = (report.genericButtons & bit) != 0;
                                 const bool wasDown = (prevReport.genericButtons & bit) != 0;
                                 if (isDown != wasDown)
                                    m_inputManager.PushButtonEvent(pindev->GetDeviceId(), static_cast<uint16_t>(0x0100 | buttonNum), timestampNs, isDown);
                              }

                              if (report.plungerPos != prevReport.plungerPos)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x200, timestampNs, static_cast<float>(report.plungerPos) / 32768.f);
                              if (report.plungerSpeed != prevReport.plungerSpeed)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x201, timestampNs, static_cast<float>(report.plungerSpeed) / 32768.f);
                              if (report.axNudge != prevReport.axNudge)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x202, timestampNs, static_cast<float>(report.axNudge) / 32768.f);
                              if (report.ayNudge != prevReport.ayNudge)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x203, timestampNs, static_cast<float>(report.ayNudge) / 32768.f);
                              if (report.vxNudge != prevReport.vxNudge)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x204, timestampNs, static_cast<float>(report.vxNudge) / 32768.f);
                              if (report.vyNudge != prevReport.vyNudge)
                                 m_inputManager.PushAxisEvent(pindev->GetDeviceId(), 0x205, timestampNs, static_cast<float>(report.vyNudge) / 32768.f);
                           });

                           // add it to the active device list, releasing ownership of the device
                           // handle to the list object
                           m_OpenPinDevContext->m_openPinDevs.emplace_back(pinDev);

                           // stop searching - there should be only one match
                           break;
                        }
                     }
                  }

                  // stop as soon as we find a match
                  if (found)
                  {
                     PLOGI << "OpenPinballDevice found and registered.";
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
OpenPinDevHandler::~OpenPinDevHandler()
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
      p->ReadReport();
}
