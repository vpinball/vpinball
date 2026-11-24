// license:GPLv3+

#include "core/stdafx.h"
#include "DeviceLayouts.h"

#include "core/VPApp.h"
#include "renderer/RenderDevice.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "ui/live/IconsLucide.h"

#include <nlohmann/json.hpp>


namespace VPX::InGameUI
{

namespace
{

   // Positions are normalized (0..1) on the layout artwork in src/assets/input/
   const DeviceLayout kGamepadLayout {
   .id = "gamepad",
   .assetFile = "input/gamepad.webp",
   .aspect = 1024.f / 601.f,
   .anchors = {
      { 0xFFFF, { "Left Trigger"s }, { 0.3554f, 0.1440f }, "LT" },
      { 0xFFFF, { "Right Trigger"s }, { 0.6605f, 0.1440f }, "RT" },
      { 0xFFFF, { "Left Shoulder"s }, { 0.3554f, 0.2306f }, "LB" },
      { 0xFFFF, { "Right Shoulder"s }, { 0.6605f, 0.2306f }, "RB" },
      { 0xFFFF, { "Back"s }, { 0.4539f, 0.4417f }, "Back" },
      { 0xFFFF, { "Guide"s, "Touchpad"s, "Misc #1"s, "Misc #2"s, "Misc #3"s, "Misc #4"s, "Misc #5"s, "Misc #6"s }, { 0.5080f, 0.3660f }, "Guide" },
      { 0xFFFF, { "Start"s }, { 0.5620f, 0.4417f }, "Start" },
      { 0xFFFF, { "Left Stick"s, "Left Stick X"s, "Left Stick Y"s }, { 0.3331f, 0.4147f }, "LS" },
      { 0xFFFF, { "Right Stick"s, "Right Stick X"s, "Right Stick Y"s }, { 0.6160f, 0.5987f }, "RS" },
      { 0xFFFF, { "DPad Up"s, "DPad Down"s, "DPad Left"s, "DPad Right"s }, { 0.3999f, 0.5987f }, "DPad" },
      { 0xFFFF, { "Y"s, ICON_LC_TRIANGLE, "North"s }, { 0.6828f, 0.3443f }, "Y" },
      { 0xFFFF, { "X"s, ICON_LC_SQUARE, "West"s }, { 0.6415f, 0.4147f }, "X" },
      { 0xFFFF, { "B"s, ICON_LC_CIRCLE, "East"s }, { 0.7241f, 0.4147f }, "B" },
      { 0xFFFF, { "A"s, ICON_LC_X, "South"s }, { 0.6828f, 0.4851f }, "A" },
      { 0xFFFF, { "Left Paddle 1"s, "Left Paddle 2"s }, { 0.2639f, 0.7622f }, "LP" },
      { 0xFFFF, { "Right Paddle 1"s, "Right Paddle 2"s }, { 0.7521f, 0.7622f }, "RP" },
   },
};

   // Arcade2TV-XR pinball controller (X-Arcade Tankstick deck: side flipper buttons,
   // two 8 button clusters, 2 sticks flanking the player strip, central trackball,
   // plunger). Element ids are the fixed pin numbers registered by SDLInputHandler.
   const DeviceLayout kArcade2TVLayout {
   .id = "arcade2tv-xr",
   .assetFile = "input/arcade2tv-xr.webp",
   .aspect = 1024.f / 483.f,
   .anchors = {
      { 0x0006, {}, { 0.0845f, 0.2577f }, "L Flip" },
      { 0x0010, {}, { 0.9012f, 0.2566f }, "R Flip" },
      { 0x0007, {}, { 0.3715f, 0.2549f }, "P1" },
      { 0x0011, {}, { 0.6289f, 0.2574f }, "P2" },
      { 0x0002, {}, { 0.2544f, 0.4044f }, "L1" },
      { 0x0003, {}, { 0.3125f, 0.4032f }, "L2" },
      { 0x0005, {}, { 0.3705f, 0.4044f }, "L3" },
      { 0x0000, {}, { 0.2550f, 0.5240f }, "L4" },
      { 0x0001, {}, { 0.3119f, 0.5240f }, "L5" },
      { 0x0008, {}, { 0.3699f, 0.5240f }, "L6" },
      { 0x0004, {}, { 0.2265f, 0.6445f }, "L7" },
      { 0x0009, {}, { 0.2840f, 0.6458f }, "L8" },
      { 0x000C, {}, { 0.7692f, 0.4032f }, "R1" },
      { 0x000D, {}, { 0.8273f, 0.4044f }, "R2" },
      { 0x000F, {}, { 0.8841f, 0.4044f }, "R3" },
      { 0x000A, {}, { 0.7692f, 0.5240f }, "R4" },
      { 0x000B, {}, { 0.8267f, 0.5240f }, "R5" },
      { 0x0012, {}, { 0.8865f, 0.5340f }, "R6" },
      { 0x000E, {}, { 0.7407f, 0.6445f }, "R7" },
      { 0x0013, {}, { 0.7983f, 0.6470f }, "R8" },
      { 0x0200, {}, { 0.1597f, 0.4690f }, "L Stick" },
      { 0x0201, {}, { 0.1591f, 0.4702f }, "L Stick" },
      { 0x0203, {}, { 0.6723f, 0.4690f }, "R Stick" },
      { 0x0204, {}, { 0.6723f, 0.4690f }, "R Stick" },
      { 0x0205, {}, { 0.8959f, 0.7694f }, "Plunger" },
   },
};

   // OpenXR VR controller pair (left hand on the left, right hand on the right).
   // Anchors are keyed by the registered element names (see XRInputHandler).
   const DeviceLayout kVRLayout {
   .id = "vr",
   .assetFile = "input/vr-controller.webp",
   .aspect = 1024.f / 547.f,
   .anchors = {
      { 0xFFFF, { "Left Trigger"s, "Left Trigger (Analog)"s, "Left Select"s }, { 0.2049f, 0.1551f }, "Trig" },
      { 0xFFFF, { "Left Grip"s, "Left Grip (Analog)"s }, { 0.2617f, 0.7507f }, "Grip" },
      { 0xFFFF, { "Left Thumbstick Click"s, "Left Thumbstick X"s, "Left Thumbstick Y"s, "Left Trackpad Click"s, "Left Trackpad X"s, "Left Trackpad Y"s }, { 0.1564f, 0.4037f }, "Stick" },
      { 0xFFFF, { "Left X"s }, { 0.2169f, 0.4892f }, "X" },
      { 0xFFFF, { "Left Y"s }, { 0.2396f, 0.3752f }, "Y" },
      { 0xFFFF, { "Left Menu"s }, { 0.1635f, 0.5591f }, "Menu" },
      { 0xFFFF, { "Right Trigger"s, "Right Trigger (Analog)"s, "Right Select"s }, { 0.8067f, 0.1585f }, "Trig" },
      { 0xFFFF, { "Right Grip"s, "Right Grip (Analog)"s }, { 0.7515f, 0.7501f }, "Grip" },
      { 0xFFFF, { "Right Thumbstick Click"s, "Right Thumbstick X"s, "Right Thumbstick Y"s, "Right Trackpad Click"s, "Right Trackpad X"s, "Right Trackpad Y"s }, { 0.8608f, 0.4014f }, "Stick" },
      { 0xFFFF, { "Right A"s }, { 0.7970f, 0.4869f }, "A" },
      { 0xFFFF, { "Right B"s }, { 0.7764f, 0.3752f }, "B" },
      { 0xFFFF, { "Right Menu"s }, { 0.8524f, 0.5568f }, "Menu" },
   },
};

   // US ANSI 104-key keyboard: main block with realistic key widths, spaced
   // function row, nav cluster above the arrows, and numpad. Anchors are keyed
   // by the SDL scancode used as element id; positions are the key centers in
   // key units (1u = 40px) over the 23.2u x 6.7u document (0.1u margin).
   // Regenerate with tools/input-layouts/gen_keyboard.py.
   DeviceLayout BuildKeyboardLayout()
   {
      struct KeyCell
      {
         uint16_t scancode;
         ImVec2 pos; // normalized 0..1 position on the artwork
         const char* caption;
      };
      static constexpr KeyCell cells[] = {
         { 41, { 0.0231f, 0.0822f }, "Esc" },
         { 58, { 0.1102f, 0.0822f }, "F1" },
         { 59, { 0.1538f, 0.0822f }, "F2" },
         { 60, { 0.1973f, 0.0822f }, "F3" },
         { 61, { 0.2409f, 0.0822f }, "F4" },
         { 62, { 0.3062f, 0.0822f }, "F5" },
         { 63, { 0.3498f, 0.0822f }, "F6" },
         { 64, { 0.3933f, 0.0822f }, "F7" },
         { 65, { 0.4369f, 0.0822f }, "F8" },
         { 66, { 0.5022f, 0.0822f }, "F9" },
         { 67, { 0.5457f, 0.0822f }, "F10" },
         { 68, { 0.5893f, 0.0822f }, "F11" },
         { 69, { 0.6328f, 0.0822f }, "F12" },
         { 70, { 0.6981f, 0.0822f }, "PrtSc" },
         { 71, { 0.7417f, 0.0822f }, "ScrLk" },
         { 72, { 0.7852f, 0.0822f }, "Pause" },
         { 53, { 0.0231f, 0.3143f }, "`" },
         { 30, { 0.0667f, 0.3143f }, "1" },
         { 31, { 0.1102f, 0.3143f }, "2" },
         { 32, { 0.1538f, 0.3143f }, "3" },
         { 33, { 0.1973f, 0.3143f }, "4" },
         { 34, { 0.2409f, 0.3143f }, "5" },
         { 35, { 0.2844f, 0.3143f }, "6" },
         { 36, { 0.3280f, 0.3143f }, "7" },
         { 37, { 0.3715f, 0.3143f }, "8" },
         { 38, { 0.4151f, 0.3143f }, "9" },
         { 39, { 0.4586f, 0.3143f }, "0" },
         { 45, { 0.5022f, 0.3143f }, "-" },
         { 46, { 0.5457f, 0.3143f }, "=" },
         { 42, { 0.6111f, 0.3143f }, "Bksp" },
         { 43, { 0.0340f, 0.4691f }, "Tab" },
         { 20, { 0.0885f, 0.4691f }, "Q" },
         { 26, { 0.1320f, 0.4691f }, "W" },
         { 8, { 0.1756f, 0.4691f }, "E" },
         { 21, { 0.2191f, 0.4691f }, "R" },
         { 23, { 0.2627f, 0.4691f }, "T" },
         { 28, { 0.3062f, 0.4691f }, "Y" },
         { 24, { 0.3498f, 0.4691f }, "U" },
         { 12, { 0.3933f, 0.4691f }, "I" },
         { 18, { 0.4369f, 0.4691f }, "O" },
         { 19, { 0.4804f, 0.4691f }, "P" },
         { 47, { 0.5240f, 0.4691f }, "[" },
         { 48, { 0.5675f, 0.4691f }, "]" },
         { 49, { 0.6219f, 0.4691f }, "\\" },
         { 57, { 0.0395f, 0.6238f }, "Caps" },
         { 4, { 0.0993f, 0.6238f }, "A" },
         { 22, { 0.1429f, 0.6238f }, "S" },
         { 7, { 0.1864f, 0.6238f }, "D" },
         { 9, { 0.2300f, 0.6238f }, "F" },
         { 10, { 0.2735f, 0.6238f }, "G" },
         { 11, { 0.3171f, 0.6238f }, "H" },
         { 13, { 0.3606f, 0.6238f }, "J" },
         { 14, { 0.4042f, 0.6238f }, "K" },
         { 15, { 0.4477f, 0.6238f }, "L" },
         { 51, { 0.4913f, 0.6238f }, ";" },
         { 52, { 0.5348f, 0.6238f }, "'" },
         { 40, { 0.6056f, 0.6238f }, "Enter" },
         { 225, { 0.0504f, 0.7785f }, "Shift" },
         { 29, { 0.1211f, 0.7785f }, "Z" },
         { 27, { 0.1647f, 0.7785f }, "X" },
         { 6, { 0.2082f, 0.7785f }, "C" },
         { 25, { 0.2518f, 0.7785f }, "V" },
         { 5, { 0.2953f, 0.7785f }, "B" },
         { 17, { 0.3389f, 0.7785f }, "N" },
         { 16, { 0.3824f, 0.7785f }, "M" },
         { 54, { 0.4260f, 0.7785f }, "," },
         { 55, { 0.4695f, 0.7785f }, "." },
         { 56, { 0.5131f, 0.7785f }, "/" },
         { 229, { 0.5947f, 0.7785f }, "Shift" },
         { 224, { 0.0286f, 0.9333f }, "Ctrl" },
         { 227, { 0.0830f, 0.9333f }, "Win" },
         { 226, { 0.1375f, 0.9333f }, "Alt" },
         { 44, { 0.3008f, 0.9333f }, "Space" },
         { 230, { 0.4641f, 0.9333f }, "Alt" },
         { 231, { 0.5185f, 0.9333f }, "Win" },
         { 118, { 0.5729f, 0.9333f }, "Menu" },
         { 228, { 0.6274f, 0.9333f }, "Ctrl" },
         { 73, { 0.6981f, 0.3143f }, "Ins" },
         { 74, { 0.7417f, 0.3143f }, "Home" },
         { 75, { 0.7852f, 0.3143f }, "PgUp" },
         { 76, { 0.6981f, 0.4691f }, "Del" },
         { 77, { 0.7417f, 0.4691f }, "End" },
         { 78, { 0.7852f, 0.4691f }, "PgDn" },
         { 82, { 0.7417f, 0.7785f }, "↑" },
         { 80, { 0.6981f, 0.9333f }, "←" },
         { 81, { 0.7417f, 0.9333f }, "↓" },
         { 79, { 0.7852f, 0.9333f }, "→" },
         { 83, { 0.8506f, 0.3143f }, "Num" },
         { 84, { 0.8941f, 0.3143f }, "/" },
         { 85, { 0.9377f, 0.3143f }, "*" },
         { 86, { 0.9812f, 0.3143f }, "-" },
         { 95, { 0.8506f, 0.4691f }, "7" },
         { 96, { 0.8941f, 0.4691f }, "8" },
         { 97, { 0.9377f, 0.4691f }, "9" },
         { 87, { 0.9812f, 0.5464f }, "+" },
         { 92, { 0.8506f, 0.6238f }, "4" },
         { 93, { 0.8941f, 0.6238f }, "5" },
         { 94, { 0.9377f, 0.6238f }, "6" },
         { 89, { 0.8506f, 0.7785f }, "1" },
         { 90, { 0.8941f, 0.7785f }, "2" },
         { 91, { 0.9377f, 0.7785f }, "3" },
         { 88, { 0.9812f, 0.8559f }, "Enter" },
         { 98, { 0.8723f, 0.9333f }, "0" },
         { 99, { 0.9377f, 0.9333f }, "." },
      };
      DeviceLayout layout { .id = "keyboard", .assetFile = "input/keyboard.webp", .aspect = 1024.f / 288.f, .anchors = {} };
      layout.anchors.reserve(std::size(cells));
      for (const KeyCell& cell : cells)
         layout.anchors.push_back({ cell.scancode, {}, cell.pos, cell.caption });
      return layout;
   }

   const DeviceLayout kMouseLayout {
   .id = "mouse",
   .assetFile = "input/mouse.webp",
   .aspect = 1024.f / 979.f,
   .anchors = {
      { SDL_BUTTON_LEFT, {}, { 0.4339f, 0.2933f }, "LMB" },
      { SDL_BUTTON_MIDDLE, {}, { 0.5485f, 0.2375f }, "MMB" },
      { SDL_BUTTON_RIGHT, {}, { 0.6716f, 0.2963f }, "RMB" },
      { SDL_BUTTON_X1, {}, { 0.2211f, 0.4841f }, "X1" },
      { SDL_BUTTON_X2, {}, { 0.2136f, 0.6343f }, "X2" },
   },
};

   const DeviceLayout kKeyboardLayout = BuildKeyboardLayout();

   // Layout anchors can be authored in tools/input-layouts (see README.md): an
   // 'anchors' layer in each SVG is exported to src/assets/input/<id>.layout,
   // which is loaded here and overrides the compiled-in fallback below.
   void ApplyLayoutFile(DeviceLayout& layout, const std::filesystem::path& file)
   {
      std::ifstream stream(file);
      if (!stream.is_open())
         return;
      const nlohmann::json json = nlohmann::json::parse(stream, nullptr, false);
      if (!json.is_object())
         return;
      const auto anchors = json.find("anchors");
      if (anchors == json.end() || !anchors->is_array() || anchors->empty())
         return;
      // A malformed entry throws: keep the compiled-in fallback
      vector<DeviceLayoutAnchor> parsed;
      try
      {
         for (const auto& entry : *anchors)
         {
            DeviceLayoutAnchor anchor;
            anchor.elementId = static_cast<uint16_t>(entry.value("id", 0xFFFF));
            for (const auto& name : entry.value("names", nlohmann::json::array()))
               anchor.elementNames.push_back(name.get<string>());
            if (const auto pos = entry.find("pos"); pos != entry.end() && pos->is_array() && pos->size() == 2)
               anchor.pos = ImVec2((*pos)[0].get<float>(), (*pos)[1].get<float>());
            anchor.caption = entry.value("caption", ""s);
            if (const auto callout = entry.find("callout"); callout != entry.end() && callout->is_array() && callout->size() == 4)
            {
               anchor.hasCallout = true;
               anchor.callout = ImVec4((*callout)[0].get<float>(), (*callout)[1].get<float>(), (*callout)[2].get<float>(), (*callout)[3].get<float>());
            }
            parsed.push_back(std::move(anchor));
         }
      }
      catch (const nlohmann::json::exception& e)
      {
         PLOGE << "Invalid device layout file " << file.string() << ": " << e.what();
         return;
      }
      layout.anchors = std::move(parsed);
      if (const auto aspect = json.find("aspect"); aspect != json.end() && aspect->is_number())
         layout.aspect = aspect->get<float>();
   }

   // Returns the layout for the given id: a static copy of the compiled table,
   // overlaid once with the authored .layout file when present next to the artwork
   const DeviceLayout& GetLayout(const DeviceLayout& compiled)
   {
      static std::unordered_map<const char*, DeviceLayout> runtimeLayouts;
      if (const auto it = runtimeLayouts.find(compiled.id); it != runtimeLayouts.end())
         return it->second;
      DeviceLayout layout = compiled;
      ApplyLayoutFile(layout, g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, std::filesystem::path(compiled.assetFile).replace_extension("layout")));
      return runtimeLayouts.emplace(compiled.id, std::move(layout)).first->second;
   }

} // namespace

const DeviceLayout* FindDeviceLayout(const InputManager& input, uint16_t deviceId)
{
   switch (input.GetDeviceType(deviceId))
   {
   case InputManager::DeviceType::Keyboard: return &GetLayout(kKeyboardLayout);
   case InputManager::DeviceType::Mouse: return &GetLayout(kMouseLayout);
   case InputManager::DeviceType::VRController: return &GetLayout(kVRLayout);
   default: break;
   }
   if (input.GetDeviceName(deviceId).starts_with("Arcade2TV-XR"s))
      return &GetLayout(kArcade2TVLayout);
   // SDL gamepads expose standard element names (registered from the gamepad bindings)
   const auto elements = input.GetDeviceElements(deviceId);
   const auto hasElement = [&elements](const char* name) { return std::ranges::any_of(elements, [name](const auto& e) { return e.name == name; }); };
   if (hasElement("Left Trigger") || hasElement("Left Stick X") || hasElement("DPad Up"))
      return &GetLayout(kGamepadLayout);
   return nullptr;
}

// OpenXR elements can be registered either under their friendly name ("Left
// Trigger (Analog)") or under the sanitized interaction profile path
// ("user_hand_left_input_trigger_value") depending on where/how the device was
// registered; anchors always use the friendly form, so path names are
// canonicalized back at match time (see XRInputHandler's naming table)
string XRElementNameToFriendly(const string& elementName)
{
   // Accept both the raw OpenXR path ("/user/hand/left/input/trigger/click") and
   // its sanitized action name ("user_hand_left_input_trigger_click")
   string name = elementName;
   std::replace(name.begin(), name.end(), '/', '_');
   if (name.starts_with('_'))
      name.erase(0, 1);
   string prefix;
   const std::string_view left { "user_hand_left_input_" };
   const std::string_view right { "user_hand_right_input_" };
   if (name.starts_with(left))
   {
      prefix = "Left "s;
      name = name.substr(left.size());
   }
   else if (name.starts_with(right))
   {
      prefix = "Right "s;
      name = name.substr(right.size());
   }
   else
      return elementName;
   static const std::pair<const char*, const char*> names[] { { "select_click", "Select" }, { "menu_click", "Menu" }, //
      { "squeeze_click", "Grip" }, { "squeeze_value", "Grip (Analog)" }, //
      { "trigger_click", "Trigger" }, { "trigger_value", "Trigger (Analog)" }, //
      { "x_click", "X" }, { "y_click", "Y" }, { "a_click", "A" }, { "b_click", "B" }, //
      { "thumbstick_click", "Thumbstick Click" }, { "thumbstick_x", "Thumbstick X" }, { "thumbstick_y", "Thumbstick Y" }, //
      { "trackpad_click", "Trackpad Click" }, { "trackpad_x", "Trackpad X" }, { "trackpad_y", "Trackpad Y" } };
   for (const auto& [path, friendly] : names)
      if (name == path)
         return prefix + friendly;
   return elementName;
}

const DeviceLayoutAnchor* FindDeviceAnchor(const DeviceLayout& layout, uint16_t elementId, const string& elementName)
{
   const string friendlyName = XRElementNameToFriendly(elementName);
   for (const DeviceLayoutAnchor& anchor : layout.anchors)
   {
      if (anchor.elementId != 0xFFFF && anchor.elementId == elementId)
         return &anchor;
      if (!elementName.empty())
         for (const string& name : anchor.elementNames)
            if (name == elementName || name == friendlyName)
               return &anchor;
   }
   return nullptr;
}

std::shared_ptr<Sampler> LoadDeviceLayoutImage(RenderDevice* rd, const string& assetFile)
{
   const std::filesystem::path path = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, assetFile);
   const std::shared_ptr<BaseTexture> tex = BaseTexture::CreateFromFile(path);
   if (tex == nullptr || tex->width() == 0 || tex->height() == 0)
      return nullptr;
   return std::make_shared<Sampler>(rd, assetFile, tex, false);
}

}
