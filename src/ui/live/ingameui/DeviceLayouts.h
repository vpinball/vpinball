// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

#include "input/InputManager.h"

class RenderDevice;
class Sampler;

namespace VPX::InGameUI
{

// Position of an input element on a device's layout artwork. A bound element matches
// an anchor either by its id (fixed hardware ids: SDL scancodes and mouse buttons,
// Arcade2TV-XR pins, OpenXR action indices) or by one of its registered element
// names (SDL gamepad labels like "A" / "Left Trigger" / "DPad Up").
struct DeviceLayoutAnchor
{
   uint16_t elementId = 0xFFFF;
   vector<string> elementNames;
   ImVec2 pos; // normalized 0..1 position on the layout artwork
   string caption; // short label drawn on the artwork at the anchor position
   // Optional authored position of the mapping callout block (SVG 'callouts'
   // layer), normalized like pos; may leave 0..1 to sit in the margins the
   // renderer adds around the artwork. When absent, the renderer picks a
   // position automatically around the closest artwork edge
   bool hasCallout = false;
   ImVec4 callout; // x, y, w, h
};

// A visual layout of an input device: an artwork image plus the positions of its
// input elements, used by InputDevicePage to draw mapped-input callouts over the
// device picture.
struct DeviceLayout
{
   const char* id; // "gamepad" | "arcade2tv-xr" | "vr" | "keyboard" | "mouse"
   const char* assetFile; // image file, relative to the application assets folder
   float aspect; // artwork width / height ratio
   vector<DeviceLayoutAnchor> anchors;
};

// Returns the layout matching the given device, or nullptr when it is not a
// recognized device. Works for disconnected devices as type, name and element
// names are persisted in the settings.
const DeviceLayout* FindDeviceLayout(const InputManager& input, uint16_t deviceId);

// Returns the anchor matching a device element (by id, else by registered name).
const DeviceLayoutAnchor* FindDeviceAnchor(const DeviceLayout& layout, uint16_t elementId, const string& elementName);

// Canonicalizes an OpenXR element name to its friendly form ("user_hand_left_input_trigger_value" -> "Left Trigger (Analog)")
string XRElementNameToFriendly(const string& elementName);

// Loads a layout artwork image into a sampler
std::shared_ptr<Sampler> LoadDeviceLayoutImage(RenderDevice* rd, const string& assetFile);

}
