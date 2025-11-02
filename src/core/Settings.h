// license:GPLv3+

#pragma once

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "unordered_dense.h"

#include "PropertyRegistry.h"
#include "LayeredINIPropertyStore.h"


// This class holds the settings registry.
// A setting registry can have a parent, in which case, missing settings will be looked for in the parent.
// This is used to allow overriding part of the settings for a specific table while still using the base application value for others.
// When saving value, an 'override mode' can be used where the value will be saved only if it not the same as the one in the parent.
class Settings final
{
   // New implementation based on property definition/registry/store
   // As changing this API will impact a large part of the codebase, it is planned to be be performed in 3 steps:
   // 1. Create property API [done]
   // 2. Populate shared property registry, and use it where applicable using PropertyDef/Registry [in progress]
   // 3. Split store implementation from Settings class [to be done]
private:
   static inline const VPX::Properties::PropertyRegistry::PropId m_propInvalid { };

public:
   static VPX::Properties::PropertyRegistry &GetRegistry();

   inline float GetFloat(VPX::Properties::PropertyRegistry::PropId propId) const { return m_store.GetFloat(propId); }
   inline int GetInt(VPX::Properties::PropertyRegistry::PropId propId) const { return m_store.GetInt(propId); }
   inline bool GetBool(VPX::Properties::PropertyRegistry::PropId propId) const { return m_store.GetInt(propId) != 0; }
   inline const string &GetString(VPX::Properties::PropertyRegistry::PropId propId) const { return m_store.GetString(propId); }
   void Set(VPX::Properties::PropertyRegistry::PropId propId, float v, bool asTableOverride);
   void Set(VPX::Properties::PropertyRegistry::PropId propId, int v, bool asTableOverride);
   void Set(VPX::Properties::PropertyRegistry::PropId propId, bool v, bool asTableOverride);
   void Set(VPX::Properties::PropertyRegistry::PropId propId, const string &v, bool asTableOverride);
   inline void Reset(VPX::Properties::PropertyRegistry::PropId propId) { m_store.Reset(propId); }

#define PropBool(groupId, propId, label, comment, defVal)                                                                                                                                    \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(#groupId, #propId, label, comment, defVal));                                                               \
   static inline const VPX::Properties::BoolPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId); }                          \
   inline bool Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                        \
   inline void Set##groupId##_##propId(bool v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal)                                                                                                                     \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, defVal));                                                \
   static inline const VPX::Properties::IntPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId); }                            \
   inline int Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                         \
   inline void Set##groupId##_##propId(int v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                 \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropEnum(groupId, propId, label, comment, type, defVal, ...)                                                                                                                         \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(#groupId, #propId, label, comment, 0, defVal, vector<string> { __VA_ARGS__ }));                            \
   static inline const VPX::Properties::EnumPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetEnumProperty(m_prop##groupId##_##propId); }                          \
   inline type Get##groupId##_##propId() const { return (type)(m_store.GetInt(m_prop##groupId##_##propId)); }                                                                                \
   inline void Set##groupId##_##propId(type v, bool asTableOverride) { Set(m_prop##groupId##_##propId, (int)v, asTableOverride); }                                                           \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, step, defVal)                                                                                                      \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, step, defVal));                                        \
   static inline const VPX::Properties::FloatPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId); }                        \
   inline float Get##groupId##_##propId() const { return m_store.GetFloat(m_prop##groupId##_##propId); }                                                                                     \
   inline void Set##groupId##_##propId(float v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                               \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropFloat(groupId, propId, label, comment, minVal, maxVal, defVal) PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, 0.f, defVal)

#define PropString(groupId, propId, label, comment, defVal)                                                                                                                                  \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(#groupId, #propId, label, comment, defVal));                                                             \
   static inline const VPX::Properties::StringPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId); }                      \
   inline const string &Get##groupId##_##propId() const { return m_store.GetString(m_prop##groupId##_##propId); }                                                                            \
   inline void Set##groupId##_##propId(const string &v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                       \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropArray(groupId, propId, type, propType, getType, ...)                                                                                                                             \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId[] = { __VA_ARGS__ };                                                                             \
   static inline const VPX::Properties::propType##PropertyDef *Get##groupId##_##propId##_Property(int index)                                                                                 \
   {                                                                                                                                                                                         \
      return GetRegistry().Get##propType##Property(m_prop##groupId##_##propId[index]);                                                                                                       \
   }                                                                                                                                                                                         \
   inline type Get##groupId##_##propId(int index) const { return (type)(m_store.Get##getType(m_prop##groupId##_##propId[index])); }                                                          \
   inline void Set##groupId##_##propId(int index, type v, bool asTableOverride) { Set(m_prop##groupId##_##propId[index], (type)v, asTableOverride); }                                        \
   inline void Reset##groupId##_##propId(int index) { Reset(m_prop##groupId##_##propId[index]); }


   // General Application settings
   PropBool(Editor, EnableLog, "Enable Log"s, "Enable general logging to the vinball.log file"s, true);
   PropBool(Editor, DisableHash, "Disable File Validation"s, "Disable file integrity validation"s, false);

   // Audio settings
   PropInt(Player, MusicVolume, "Backglass Volume"s, "Main volume for music and sound played in the backglass speakers"s, 0, 100, 100);
   PropInt(Player, SoundVolume, "Playfield Volume"s, "Main volume for mechanical sounds coming from the playfield"s, 0, 100, 100);
   PropBool(Player, PlayMusic, "Enable Backglass"s, "Enable/Disable backglass game sound & music"s, true);
   PropBool(Player, PlaySound, "Enable Playfield"s, "Enable/Disable playfield mechanical sounds"s, true);
   PropString(Player, SoundDeviceBG, "Backglass Sound Device"s, "Select backglass sound device"s, ""s);
   PropString(Player, SoundDevice, "Playfield Sound Device"s, "Select playfield sound device"s, ""s);
   PropEnum(Player, Sound3D, "Playfield Output Mode"s, "Select how playfield sound is outputed"s, int /* VPX::SoundConfigTypes*/, 0 /* VPX::SoundConfigTypes::SNDCFG_SND3D2CH */,
      "2 front channels"s, "2 rear channels"s, "Up to 6 channels. Rear at lockbar"s, "Up to 6 channels. Front at lockbar"s, "Up to 4 channels. Front at lockbar"s,
      "Side & rear channels. Rear at lockbar. Legacy mixing"s, "Side & rear channels. Rear at lockbar. New mixing"s);

   // Output (windows) settings
   // Main window (a.k.a. playfield)
   PropString(Player, PlayfieldDisplay, "Display"s, "Display used for the main Playfield window"s, ""s);
   PropInt(Player, PlayfieldWndX, "X Position", "Horizontal position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Player, PlayfieldWndY, "Y Position", "Vertical position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Player, PlayfieldWidth, "Width", "Width of the window"s, 0, 16384, 16384);
   PropInt(Player, PlayfieldHeight, "Height", "Height of the window"s, 0, 16384, 16384);
   PropBool(Player, PlayfieldFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode (should be avoided unless you need to change the display resolution)"s, false);
   PropInt(Player, PlayfieldFSWidth, "Width", "Fullscreen display mode width"s, 0, 16384, 16384);
   PropInt(Player, PlayfieldFSHeight, "Height", "Fullscreen display mode height"s, 0, 16384, 16384);
   PropFloat(Player, PlayfieldRefreshRate, "Refresh Rate", "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
   PropInt(Player, PlayfieldColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
   // Backglass Window
   PropEnum(Backglass, BackglassOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s,
      "Floating"s, "Embedded in playfield"s);
   PropString(Backglass, BackglassDisplay, "Display"s, "Display used for the main Backglass window"s, ""s);
   PropInt(Backglass, BackglassWndX, "X Position", "Horizontal position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Backglass, BackglassWndY, "Y Position", "Vertical position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Backglass, BackglassWidth, "Width", "Width of the window"s, 0, 16384, 16384);
   PropInt(Backglass, BackglassHeight, "Height", "Height of the window"s, 0, 16384, 16384);
   PropBool(Backglass, BackglassFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode (should be avoided unless you need to change the display resolution)"s, false);
   PropInt(Backglass, BackglassFSWidth, "Width", "Fullscreen display mode width"s, 0, 16384, 16384);
   PropInt(Backglass, BackglassFSHeight, "Height", "Fullscreen display mode height"s, 0, 16384, 16384);
   PropFloat(Backglass, BackglassRefreshRate, "Fullscreen Refresh Rate", "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
   PropInt(Backglass, BackglassColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
   // ScoreView Window
   PropEnum(ScoreView, ScoreViewOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s,
      "Floating"s, "Embedded in playfield"s);
   PropString(ScoreView, ScoreViewDisplay, "Display"s, "Display used for the main ScoreView window"s, ""s);
   PropInt(ScoreView, ScoreViewWndX, "X Position", "Horizontal position of the window on the selected display"s, 0, 16384, 0);
   PropInt(ScoreView, ScoreViewWndY, "Y Position", "Vertical position of the window on the selected display"s, 0, 16384, 0);
   PropInt(ScoreView, ScoreViewWidth, "Width", "Width of the window"s, 0, 16384, 16384);
   PropInt(ScoreView, ScoreViewHeight, "Height", "Height of the window"s, 0, 16384, 16384);
   PropBool(ScoreView, ScoreViewFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode (should be avoided unless you need to change the display resolution)"s, false);
   PropInt(ScoreView, ScoreViewFSWidth, "Width", "Fullscreen display mode width"s, 0, 16384, 16384);
   PropInt(ScoreView, ScoreViewFSHeight, "Height", "Fullscreen display mode height"s, 0, 16384, 16384);
   PropFloat(ScoreView, ScoreViewRefreshRate, "Fullscreen Refresh Rate", "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
   PropInt(ScoreView, ScoreViewColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
   // Topper Window
   PropEnum(Topper, TopperOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s,
      "Floating"s, "Embedded in playfield"s);
   PropString(Topper, TopperDisplay, "Display"s, "Display used for the Topper window"s, ""s);
   PropInt(Topper, TopperWndX, "X Position", "Horizontal position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Topper, TopperWndY, "Y Position", "Vertical position of the window on the selected display"s, 0, 16384, 0);
   PropInt(Topper, TopperWidth, "Width", "Width of the window"s, 0, 16384, 16384);
   PropInt(Topper, TopperHeight, "Height", "Height of the window"s, 0, 16384, 16384);
   PropBool(Topper, TopperFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode (should be avoided unless you need to change the display resolution)"s, false);
   PropInt(Topper, TopperFSWidth, "Width", "Fullscreen display mode width"s, 0, 16384, 16384);
   PropInt(Topper, TopperFSHeight, "Height", "Fullscreen display mode height"s, 0, 16384, 16384);
   PropFloat(Topper, TopperRefreshRate, "Fullscreen Refresh Rate", "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
   PropInt(Topper, TopperColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
   // VR Preview Window
   PropString(PlayerVR, PreviewDisplay, "Display"s, "Display used for the VR Preview window"s, ""s);
   PropInt(PlayerVR, PreviewWndX, "X Position", "Horizontal position of the window on the selected display"s, 0, 16384, 0);
   PropInt(PlayerVR, PreviewWndY, "Y Position", "Vertical position of the window on the selected display"s, 0, 16384, 0);
   PropInt(PlayerVR, PreviewWidth, "Width", "Width of the window"s, 0, 16384, 16384);
   PropInt(PlayerVR, PreviewHeight, "Height", "Height of the window"s, 0, 16384, 16384);
   PropBool(PlayerVR, PreviewFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode (should be avoided unless you need to change the display resolution)"s, false);
   PropInt(PlayerVR, PreviewFSWidth, "Width", "Fullscreen display mode width"s, 0, 16384, 16384);
   PropInt(PlayerVR, PreviewFSHeight, "Height", "Fullscreen display mode height"s, 0, 16384, 16384);
   PropFloat(PlayerVR, PreviewRefreshRate, "Fullscreen Refresh Rate", "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
   PropInt(PlayerVR, PreviewColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
   // Array access (using VPXWindowId)
   PropArray(Window, Mode, int, Enum, Int, m_propInvalid, m_propBackglass_BackglassOutput, m_propScoreView_ScoreViewOutput, m_propTopper_TopperOutput, m_propInvalid);
   PropArray(Window, Display, string, String, String, m_propPlayer_PlayfieldDisplay, m_propBackglass_BackglassDisplay, m_propScoreView_ScoreViewDisplay, m_propTopper_TopperDisplay,
      m_propPlayerVR_PreviewDisplay);
   PropArray(Window, WndX, int, Int, Int, m_propPlayer_PlayfieldWndX, m_propBackglass_BackglassWndX, m_propScoreView_ScoreViewWndX, m_propTopper_TopperWndX, m_propPlayerVR_PreviewWndX);
   PropArray(Window, WndY, int, Int, Int, m_propPlayer_PlayfieldWndY, m_propBackglass_BackglassWndY, m_propScoreView_ScoreViewWndY, m_propTopper_TopperWndY, m_propPlayerVR_PreviewWndY);
   PropArray(
      Window, Width, int, Int, Int, m_propPlayer_PlayfieldWidth, m_propBackglass_BackglassWidth, m_propScoreView_ScoreViewWidth, m_propTopper_TopperWidth, m_propPlayerVR_PreviewWidth);
   PropArray(
      Window, Height, int, Int, Int, m_propPlayer_PlayfieldHeight, m_propBackglass_BackglassHeight, m_propScoreView_ScoreViewHeight, m_propTopper_TopperHeight, m_propPlayerVR_PreviewHeight);
   PropArray(Window, FullScreen, bool, Bool, Int, m_propPlayer_PlayfieldFullScreen, m_propBackglass_BackglassFullScreen, m_propScoreView_ScoreViewFullScreen, m_propTopper_TopperFullScreen,
      m_propPlayerVR_PreviewFullScreen);
   PropArray(
      Window, FSWidth, int, Int, Int, m_propPlayer_PlayfieldFSWidth, m_propBackglass_BackglassFSWidth, m_propScoreView_ScoreViewFSWidth, m_propTopper_TopperFSWidth, m_propPlayerVR_PreviewFSWidth);
   PropArray(
      Window, FSHeight, int, Int, Int, m_propPlayer_PlayfieldFSHeight, m_propBackglass_BackglassFSHeight, m_propScoreView_ScoreViewFSHeight, m_propTopper_TopperFSHeight, m_propPlayerVR_PreviewFSHeight);
   PropArray(Window, FSRefreshRate, float, Float, Float, m_propPlayer_PlayfieldRefreshRate, m_propBackglass_BackglassRefreshRate, m_propScoreView_ScoreViewRefreshRate,
      m_propTopper_TopperRefreshRate, m_propPlayerVR_PreviewRefreshRate);
   PropArray(Window, FSColorDepth, int, Int, Int, m_propPlayer_PlayfieldColorDepth, m_propBackglass_BackglassColorDepth, m_propScoreView_ScoreViewColorDepth, m_propTopper_TopperColorDepth,
      m_propPlayerVR_PreviewColorDepth);

   // Graphics synchronisation and latency reduction
#if defined(ENABLE_BGFX)
   PropEnum(Player, SyncMode, "Synchronization"s,
      "None: Use this if your display supports variable refresh rate or if you are experiencing stutters.\r\nVertical Sync: Synchronize on display sync."s, int, 1, "No Sync",
      "Vertical Sync");
#else
   PropEnum(Player, SyncMode, "Synchronization"s,
      "None: No synchronization.\r\nVertical Sync: Synchronize on video sync which avoids video tearing, but has higher input latency.\r\nAdaptive Sync: Synchronize on video sync, "
      "except for late frames (below target FPS), also has higher input latency.\r\nFrame Pacing: Targets real time simulation with low input- and video-latency (also dynamically adjusts "
      "framerate)."s,
      int, 1, "No Sync", "Vertical Sync", "Adaptive Sync", "Frame Pacing");
#endif
   PropInt(Player, MaxFramerate, "Limit Framerate",
      "-1 will not limit FPS\r\n0 will limit to the display refresh rates\r\nOther values will limit the FPS to it (energy saving/less heat, framerate stability)"s, -1, 300, -1);
   PropInt(Player, MaxPrerenderedFrames, "Max. Prerendered Frames"s, "Limit the FPS to the given value (energy saving/less heat, framerate stability), 0 will disable it"s, 0, 5, 0);
   PropInt(Player, VisualLatencyCorrection, "Visual Latency Correction"s,
      "Leave at -1 to get default latency correction based on display frequency.\r\nIf you measured your setup latency using tools like Intel's PresentMon, enter the average latency in ms."s,
      -1, 200, -1);

   // Graphics settings
#if defined(ENABLE_BGFX)
#ifdef __ANDROID__
   /* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, int, bgfx::RendererType::OpenGLES, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
      "Metal"s, "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s);*/
   PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, "OpenGLES"s);
#elif defined(__APPLE__)
   /* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, int, bgfx::RendererType::Metal, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
      "Metal"s,
      "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s);*/
   PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, "Metal"s);
#else
   PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, "Default"s);
   /* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics backend used for rendering"s, int, bgfx::RendererType::Count, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
      "Metal"s,
      "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s); */
#endif
#endif
   PropEnum(Player, ShowFPS, "Show FPS"s, "Performance overlay display mode"s, int /* PerfUI::PerfMode */, 0, "Disable"s, "FPS"s, "Full"s);
   PropBool(Player, SSRefl, "Additive Screen Space Reflection"s, "Add global reflection to the entire scene"s, false);
   PropBool(Player, HDRDisableToneMapper, "Disable tonemapping on HDR display"s, "Do not perform tonemapping when rendering on a HDR display"s, true);
   PropFloat(Player, HDRGlobalExposure, "HDR Display Global Exposure", "Global exposure scale multiplier for HDR capable displays"s, 0.f, 5.f, 1.f);
   PropBool(Player, ForceBloomOff, "Disable Bloom"s, "Disable postprocessed bloom filter"s, false);
   PropBool(Player, ForceMotionBlurOff, "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s, false);
   PropBool(Player, ForceAnisotropicFiltering, "Force Anisotropic Filtering"s, "Force anisotropic filtering for better rendering quality at the cost of a bit of performance"s, true);
   PropBool(Player, CompressTextures, "Compress Textures"s, "Automatically compress textures at game startup (slow) for better performance"s, false);
   PropBool(Player, UseNVidiaAPI, "Alternative Depth Buffer"s, "Use NVidia API to manage Depth Buffer on DirectX 9 build. May solve some rendering issues"s, false);
   PropBool(Player, SoftwareVertexProcessing, "Software Vertex Processing"s, "Activate this if you have issues using an Intel graphics chip"s, false);
   PropBool(Player, DisableAO, "Disable Ambient Occlusion"s, ""s, false);
   PropBool(Player, DynamicAO, "Dynamic Ambient Occlusion"s, ""s, true);
   PropEnum(Player, PFReflection, "Reflection Quality"s,
      "Limit the quality of reflections for better performance.\r\n'Dynamic' is recommended and will give the best results, but may harm performance.\r\n'Static Only' has no performance cost (except for VR rendering).\r\nOther options feature different trade-offs between quality and performance."s,
      int, 0, "Disable Reflections"s, "Balls Only"s, "Static Only"s, "Static & Balls"s, "Static & Unsynced Dynamic"s, "Dynamic"s);
#ifndef __LIBVPINBALL__
   PropInt(Player, MaxTexDimension, "Maximum texture dimension"s, "Images sized above this limit will be automatically scaled down on load."s, 512, 16384, 0);
#else
   PropInt(Player, MaxTexDimension, "Maximum texture dimension"s, "Images sized above this limit will be automatically scaled down on load."s, 512, 16384, 1536);
#endif
   PropInt(Player, AlphaRampAccuracy, "Detail Level"s, "Images sized above this limit will be automatically scaled down on load."s, 1, 10, 10);

   // Aliasing & sharpening
   PropFloat(Player, AAFactor, "Full Scene Anti Aliasing"s,
      "Enables brute-force Up/Downsampling (similar to DSR).\r\nThis delivers very good quality but has a significant impact on performance.\r\n200% means twice the resolution to be handled while rendering"s,
      0.5f, 2.f, 1.f);
   PropEnum(Player, MSAASamples, "MSAA level"s,
      "Set the amount of MSAA samples.\r\nMSAA can help reduce geometry aliasing at the cost of performance and GPU memory.\r\nThis can improve image quality if not using supersampling"s,
      int, 0, "Disabled", "4 Samples", "6 Samples", "8 Samples");
   PropEnum(Player, FXAA, "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s, int, 1, "Disabled"s,
      "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s);
   PropEnum(Player, Sharpen, "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, int, 0, "Disabled"s,
      "CAS"s, "Bilateral CAS"s);

   // Ball rendering
   PropBool(Player, BallAntiStretch, "Unstretch Ball"s, "Compensate ball stretching"s, false);
   PropBool(Player, DisableLightingForBalls, "Disable Ball Lighting"s, "Disable lighting and reflection effects on balls, e.g. to help the visually handicapped."s, false);
   PropBool(Player, BallTrail, "Ball Trail"s, "Legacy ball trails"s, false);
   PropFloat(Player, BallTrailStrength, "Ball Trail Strength", "Strength of the fake ball trail"s, 0.f, 5.f, 0.5f);
   PropBool(Player, OverwriteBallImage, "Overwrite ball image"s, "Allow to define images that will be used instead of the table's provided one"s, false);
   PropString(Player, BallImage, "Ball image override"s, "Image to use for the ball instead of the table's provide one"s, ""s);
   PropString(Player, DecalImage, "Decal image override"s, "Image to use for the ball's decal instead of the table's provide one"s, ""s);

   // Misc player settings
   PropBool(Player, TouchOverlay, "Touch Overlay"s, "Display an overlay showing touch regions"s, false);
   PropBool(Player, EnableCameraModeFlyAround, "Legacy Fly Over Mode"s, "Enable moving camera when using Tweak menu (legacy, replaced by LiveUI fly mode)."s, false);

   // UI & input settings
   PropInt(Player, Exitconfirm, "Direct Exit Length"s, "Length of a long ESC press that directly closes the app, (sadly) expressed in seconds * 60."s, 0, 30 * 60, 120);

   // Nudge & Plumb settings
   PropFloat(Player, NudgeOrientation0, "Sensor #0 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 0.f);
   PropFloat(Player, NudgeOrientation1, "Sensor #1 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 0.f);
   PropBool(Player, NudgeFilter0, "Sensor #0 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, NudgeFilter1, "Sensor #1 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, SimulatedPlumb, "Plumb simulation"s, "Enable/Disable mechanical Tilt plumb simulation"s, true);
   PropFloat(Player, PlumbInertia, "Plumb Inertia", ""s, 0.001f, 1.f, 0.35f);
   PropFloat(Player, PlumbThresholdAngle, "Plumb Threshold", "Define threshold angle at which a Tilt is caused"s, 5.0f, 60.f, 35.f);
   PropBool(Player, EnableLegacyNudge, "Legacy Keyboard nudge"s, "Enable/Disable legacy keyboard nudge mode"s, false);
   PropFloat(Player, LegacyNudgeStrength, "Legacy Nudge Strength"s, "Strength of nudge when using the legacy keyboard nudge mode"s, 0.f, 90.f, 1.f);
   PropFloat(Player, NudgeStrength, "Visual Nudge Strength"s, "Changes the visual effect/screen shaking when nudging the table."s, 0.f, 0.25f, 0.02f);

   // Plunger settings
   PropBool(Player, PlungerRetract, "One Second Retract"s,
      "Enable retracting the plunger after a 1 second press when using the digital plunger emulation through keyboard or joystick button"s, false);
   PropBool(Player, PlungerLinearSensor, "Linear Sensor"s, "Select between symmetric (linear) and assymetric sensor"s, false);
   PropInt(Player, PlungerNormalize, "Plunger normalize override"s, "This value may be defined to override the table's plunger normalization"s, 0, 100,
      100); // Hacky: This should be a table override, not a player property as it overrides table data

   // VR settings
   PropFloat(PlayerVR, Orientation, "View orientation"s, ""s, -180.f, 180.f, 0.f);
   PropFloat(PlayerVR, TableX, "View Offset X"s, ""s, -100.f, 100.f, 0.f);
   PropFloat(PlayerVR, TableY, "View Offset Y"s, ""s, -100.f, 100.f, 0.f);
   PropFloat(PlayerVR, TableZ, "View Offset Z"s, ""s, -100.f, 100.f, 0.f);
   PropBool(PlayerVR, UsePassthroughColor, "Color Keyed Passthrough"s, "Replace VR background by a user defined color, to allow color keyed passthrough (for example using Virtual Desktop)"s,
      false);
   PropInt(PlayerVR, PassthroughColor, "Color Keyed Passthrough color"s, "Color that will replace the background"s, 0x000000, 0xFFFFFF, 0xBB4700);
   PropEnum(Player, VRPreview, "Preview mode"s, "Select preview mode"s, int, 1, "Disabled"s, "Left Eye"s, "Right Eye"s, "Both Eyes"s);
   PropBool(PlayerVR, ShrinkPreview, "Shrink preview"s, ""s, false);

   // Stereo settings
   PropBool(Player, Stereo3DEnabled, "Enable Stereo Rendering"s, "Allow to temporarily disable stereo rendering"s, false);
   PropEnum(Player, Stereo3D, "Stereo rendering"s, "Stereo rendering mode"s, StereoMode, 0, "Disabled"s, "Top / Bottom"s, "Interlaced (e.g. LG TVs)"s, "Flipped Interlaced (e.g. LG TVs)"s,
      "Side by Side"s, "Anaglyph Red/Cyan"s, "Anaglyph Green/Magenta"s, "Anaglyph Blue/Amber"s, "Anaglyph Cyan/Red"s, "Anaglyph Magenta/Green"s, "Anaglyph Amber/Blue"s, "Anaglyph Custom 1"s,
      "Anaglyph Custom 2"s, "Anaglyph Custom 3"s, "Anaglyph Custom 4"s);
   PropFloat(Player, Stereo3DEyeSeparation, "Eye distance"s, "Physical distance (mm) between eyes"s, 5.f, 200.f, 63.f);
   PropFloat(Player, Stereo3DBrightness, "Stereo Brightness"s, "Brightness adjustment applied to stereo rendering"s, 0.f, 2.f, 1.f);
   PropFloat(Player, Stereo3DSaturation, "Stereo Saturation"s, "Saturation adjustment applied to stereo rendering"s, 0.f, 2.f, 1.f);
   PropFloat(Player, Stereo3DDefocus, "Anaglyph Defocus"s, "Defocusing of the lesser eye to anaglyph stereo rendering"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Stereo3DLeftContrast, "Anaglyph Left Contrast"s, "Left eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 1.f);
   PropFloat(Player, Stereo3DRightContrast, "Anaglyph Right Contrast"s, "Right eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 1.f);
   PropEnum(Player, Anaglyph1Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph2Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph3Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph4Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph5Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph6Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph7Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph8Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph9Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph10Filter, "Anaglyph Filter"s, "Anaglyph filter applied to anaglyph profile #10"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropArray(Player, AnaglyphFilter, int, Int, Int, m_propPlayer_Anaglyph1Filter, m_propPlayer_Anaglyph2Filter, m_propPlayer_Anaglyph3Filter, m_propPlayer_Anaglyph4Filter,
      m_propPlayer_Anaglyph5Filter, m_propPlayer_Anaglyph6Filter, m_propPlayer_Anaglyph7Filter, m_propPlayer_Anaglyph8Filter, m_propPlayer_Anaglyph9Filter, m_propPlayer_Anaglyph10Filter);
   PropFloat(Player, Anaglyph1DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph2DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph3DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph4DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph5DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph6DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph7DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph8DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph9DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropFloat(Player, Anaglyph10DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
   PropArray(Player, AnaglyphDynDesat, float, Float, Float, m_propPlayer_Anaglyph1DynDesat, m_propPlayer_Anaglyph2DynDesat, m_propPlayer_Anaglyph3DynDesat, m_propPlayer_Anaglyph4DynDesat,
      m_propPlayer_Anaglyph5DynDesat, m_propPlayer_Anaglyph6DynDesat, m_propPlayer_Anaglyph7DynDesat, m_propPlayer_Anaglyph8DynDesat, m_propPlayer_Anaglyph9DynDesat,
      m_propPlayer_Anaglyph10DynDesat);
   PropFloat(Player, Anaglyph1Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph2Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph3Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph4Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph5Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph6Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph7Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph8Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph9Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropFloat(Player, Anaglyph10Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
   PropArray(Player, AnaglyphDeghost, float, Float, Float, m_propPlayer_Anaglyph1Deghost, m_propPlayer_Anaglyph2Deghost, m_propPlayer_Anaglyph3Deghost, m_propPlayer_Anaglyph4Deghost,
      m_propPlayer_Anaglyph5Deghost, m_propPlayer_Anaglyph6Deghost, m_propPlayer_Anaglyph7Deghost, m_propPlayer_Anaglyph8Deghost, m_propPlayer_Anaglyph9Deghost,
      m_propPlayer_Anaglyph10Deghost);
   PropBool(Player, Anaglyph1sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph2sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph3sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph4sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph5sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph6sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph7sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph8sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph9sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropBool(Player, Anaglyph10sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
   PropArray(Player, AnaglyphsRGB, bool, Int, Int, m_propPlayer_Anaglyph1sRGB, m_propPlayer_Anaglyph2sRGB, m_propPlayer_Anaglyph3sRGB, m_propPlayer_Anaglyph4sRGB, m_propPlayer_Anaglyph5sRGB,
      m_propPlayer_Anaglyph6sRGB, m_propPlayer_Anaglyph7sRGB, m_propPlayer_Anaglyph8sRGB, m_propPlayer_Anaglyph9sRGB, m_propPlayer_Anaglyph10sRGB);
   // Red Cyan
   PropFloat(Player, Anaglyph1LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph1LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph1LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
   PropFloat(Player, Anaglyph1RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph1RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph1RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
   // Green Magenta
   PropFloat(Player, Anaglyph2LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph2LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.96f);
   PropFloat(Player, Anaglyph2LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.09f);
   PropFloat(Player, Anaglyph2RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.61f);
   PropFloat(Player, Anaglyph2RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.16f);
   PropFloat(Player, Anaglyph2RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.66f);
   // Blue Amber
   PropFloat(Player, Anaglyph3LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.05f);
   PropFloat(Player, Anaglyph3LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.16f);
   PropFloat(Player, Anaglyph3LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.96f);
   PropFloat(Player, Anaglyph3RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.61f);
   PropFloat(Player, Anaglyph3RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.66f);
   PropFloat(Player, Anaglyph3RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.09f);
   // Cyan Red
   PropFloat(Player, Anaglyph4LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph4LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph4LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.28f);
   PropFloat(Player, Anaglyph4RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph4RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph4RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.07f);
   // Magenta Green
   PropFloat(Player, Anaglyph5LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.61f);
   PropFloat(Player, Anaglyph5LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.16f);
   PropFloat(Player, Anaglyph5LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.66f);
   PropFloat(Player, Anaglyph5RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph5RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.96f);
   PropFloat(Player, Anaglyph5RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.09f);
   // Amber Blue
   PropFloat(Player, Anaglyph6LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.61f);
   PropFloat(Player, Anaglyph6LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.66f);
   PropFloat(Player, Anaglyph6LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.09f);
   PropFloat(Player, Anaglyph6RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.05f);
   PropFloat(Player, Anaglyph6RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.16f);
   PropFloat(Player, Anaglyph6RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.96f);
   // Red Cyan
   PropFloat(Player, Anaglyph7LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph7LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph7LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
   PropFloat(Player, Anaglyph7RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph7RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph7RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
   // Red Cyan
   PropFloat(Player, Anaglyph8LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph8LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph8LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
   PropFloat(Player, Anaglyph8RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph8RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph8RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
   // Red Cyan
   PropFloat(Player, Anaglyph9LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph9LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph9LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
   PropFloat(Player, Anaglyph9RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph9RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph9RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
   // Red Cyan
   PropFloat(Player, Anaglyph10LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
   PropFloat(Player, Anaglyph10LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
   PropFloat(Player, Anaglyph10LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
   PropFloat(Player, Anaglyph10RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
   PropFloat(Player, Anaglyph10RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
   PropFloat(Player, Anaglyph10RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);

   PropArray(Player, AnaglyphLeftRed, float, Float, Float, m_propPlayer_Anaglyph1LeftRed, m_propPlayer_Anaglyph2LeftRed, m_propPlayer_Anaglyph3LeftRed, m_propPlayer_Anaglyph4LeftRed,
      m_propPlayer_Anaglyph5LeftRed, m_propPlayer_Anaglyph6LeftRed, m_propPlayer_Anaglyph7LeftRed, m_propPlayer_Anaglyph8LeftRed, m_propPlayer_Anaglyph9LeftRed,
      m_propPlayer_Anaglyph10LeftRed);
   PropArray(Player, AnaglyphLeftGreen, float, Float, Float, m_propPlayer_Anaglyph1LeftGreen, m_propPlayer_Anaglyph2LeftGreen, m_propPlayer_Anaglyph3LeftGreen,
      m_propPlayer_Anaglyph4LeftGreen, m_propPlayer_Anaglyph5LeftGreen, m_propPlayer_Anaglyph6LeftGreen, m_propPlayer_Anaglyph7LeftGreen, m_propPlayer_Anaglyph8LeftGreen,
      m_propPlayer_Anaglyph9LeftGreen, m_propPlayer_Anaglyph10LeftGreen);
   PropArray(Player, AnaglyphLeftBlue, float, Float, Float, m_propPlayer_Anaglyph1LeftBlue, m_propPlayer_Anaglyph2LeftBlue, m_propPlayer_Anaglyph3LeftBlue, m_propPlayer_Anaglyph4LeftBlue,
      m_propPlayer_Anaglyph5LeftBlue, m_propPlayer_Anaglyph6LeftBlue, m_propPlayer_Anaglyph7LeftBlue, m_propPlayer_Anaglyph8LeftBlue, m_propPlayer_Anaglyph9LeftBlue,
      m_propPlayer_Anaglyph10LeftBlue);
   PropArray(Player, AnaglyphRightRed, float, Float, Float, m_propPlayer_Anaglyph1RightRed, m_propPlayer_Anaglyph2RightRed, m_propPlayer_Anaglyph3RightRed, m_propPlayer_Anaglyph4RightRed,
      m_propPlayer_Anaglyph5RightRed, m_propPlayer_Anaglyph6RightRed, m_propPlayer_Anaglyph7RightRed, m_propPlayer_Anaglyph8RightRed, m_propPlayer_Anaglyph9RightRed,
      m_propPlayer_Anaglyph10RightRed);
   PropArray(Player, AnaglyphRightGreen, float, Float, Float, m_propPlayer_Anaglyph1RightGreen, m_propPlayer_Anaglyph2RightGreen, m_propPlayer_Anaglyph3RightGreen,
      m_propPlayer_Anaglyph4RightGreen, m_propPlayer_Anaglyph5RightGreen, m_propPlayer_Anaglyph6RightGreen, m_propPlayer_Anaglyph7RightGreen, m_propPlayer_Anaglyph8RightGreen,
      m_propPlayer_Anaglyph9RightGreen, m_propPlayer_Anaglyph10RightGreen);
   PropArray(Player, AnaglyphRightBlue, float, Float, Float, m_propPlayer_Anaglyph1RightBlue, m_propPlayer_Anaglyph2RightBlue, m_propPlayer_Anaglyph3RightBlue,
      m_propPlayer_Anaglyph4RightBlue, m_propPlayer_Anaglyph5RightBlue, m_propPlayer_Anaglyph6RightBlue, m_propPlayer_Anaglyph7RightBlue, m_propPlayer_Anaglyph8RightBlue,
      m_propPlayer_Anaglyph9RightBlue, m_propPlayer_Anaglyph10RightBlue)

      // Real world cabinet & player settings
      PropFloat(Player, ScreenWidth, "Screen Width"s, "Physical width (cm) of the display area of the playfield (main) screen (width > height)"s, 5.f, 200.f, 95.89f);
   PropFloat(Player, ScreenHeight, "Screen Height"s, "Physical height (cm) of the display area of the playfield (main) screen (width > height)"s, 5.f, 200.f, 53.94f);
   PropFloat(Player, ScreenInclination, "Screen Inclination"s, "Inclination (degree) of the playfield (main) screen. 0 is horizontal."s, -30.f, 30.f, 0.f);
   PropFloat(Player, LockbarWidth, "Lockbar Width"s, "Lockbar width in centimeters (measured on the cabinet)."s, 10.f, 150.f, 70.f);
   PropFloat(Player, LockbarHeight, "Lockbar Height"s, "Lockbar height in centimeters (measured on the cabinet, from ground to top of lockbar)."s, 0.f, 250.f, 85.f);
   PropFloat(Player, ScreenPlayerX, "Player X"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -30.f, 30.f, 0.f);
   PropFloat(Player, ScreenPlayerY, "Player Y"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -70.f, 30.f, -10.f);
   PropFloat(Player, ScreenPlayerZ, "Player Z"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, 30.f, 100.f, 70.f);

   // Overall scene lighting settings
   PropBool(Player, OverrideTableEmissionScale, "Override Table Light Level"s, "Replace table light level by a custom one"s, false);
   PropFloat(Player, EmissionScale, "Day/Night"s, "Select a custom level between daylight or night time lighting"s, 0.f, 1.f, 1.f);
   PropBool(Player, DynamicDayNight, "Use Automatic Light Level"s, "Automatically compute scene lighting based on sun's position"s, false);
   PropFloat(Player, Latitude, "Latitude"s, "Latitude used to compute sun's position"s, -90.f, 90.f, 52.52f);
   PropFloat(Player, Longitude, "Longitude"s, "Longitude used to compute sun's position"s, -180.f, 180.f, 13.37f);

   // Debugging & Live editing settings
   PropBool(Editor, ThrowBallsAlwaysOn, "Throw Balls Always On"s, "Permanently enable 'throw ball' debugging mode"s, false);
   PropBool(Editor, BallControlAlwaysOn, "Ball COntrol Always On"s, "Permanently enable 'ball control' debugging mode"s, false);

   // Default camera setup
   PropEnum(DefaultCamera, DesktopMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(DefaultCamera, DesktopLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 25.f);
   PropFloat(DefaultCamera, DesktopFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 50.f);
   PropFloat(DefaultCamera, DesktopScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, DesktopScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, DesktopScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, DesktopCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.f);
   PropFloat(DefaultCamera, DesktopCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 20.f);
   PropFloat(DefaultCamera, DesktopCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 70.f);
   PropFloat(DefaultCamera, DesktopViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 14.f);
   PropEnum(DefaultCamera, FSSMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(DefaultCamera, FSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 50.f);
   PropFloat(DefaultCamera, FSSFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 77.f);
   PropFloat(DefaultCamera, FSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, FSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, FSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
   PropFloat(DefaultCamera, FSSCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.f);
   PropFloat(DefaultCamera, FSSCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 20.f);
   PropFloat(DefaultCamera, FSSCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 70.f);
   PropFloat(DefaultCamera, FSSViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 22.f);

   // Table override settings
   // These properties are specials as they are meant to override table data at play time. They are not meant to be saved to the application setting file, but
   // only to table override settings. Also, their defaults are redefined when a table is played, depending on the table data and UI options.
   PropEnum(TableOverride, ViewDTMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewDTLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 25.f);
   PropFloat(TableOverride, ViewDTFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 50.f);
   PropFloat(TableOverride, ViewDTLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, -90.f, 90.f, 0.f);
   PropFloat(TableOverride, ViewDTScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewDTScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewDTScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewDTPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
   PropFloat(TableOverride, ViewDTPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(20.f));
   PropFloat(TableOverride, ViewDTPlayerZ, "Camera Z"s, "View point vertical offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
   PropFloat(TableOverride, ViewDTHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
   PropFloat(TableOverride, ViewDTVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
   PropFloat(TableOverride, ViewDTWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloat(
      TableOverride, ViewDTWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloatStepped(TableOverride, ViewDTRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropEnum(TableOverride, ViewFSSMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewFSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.f);
   PropFloat(TableOverride, ViewFSSFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 25.f);
   PropFloat(TableOverride, ViewFSSLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, -90.f, 90.f, 0.f);
   PropFloat(TableOverride, ViewFSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewFSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewFSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewFSSPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
   PropFloat(TableOverride, ViewFSSPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(0.f));
   PropFloat(TableOverride, ViewFSSPlayerZ, "Camera Z"s, "View point vertical offsets"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
   PropFloat(TableOverride, ViewFSSHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
   PropFloat(TableOverride, ViewFSSVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
   PropFloat(
      TableOverride, ViewFSSWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloat(
      TableOverride, ViewFSSWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloatStepped(TableOverride, ViewFSSRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropEnum(TableOverride, ViewCabMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 2, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewCabLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.f);
   PropFloat(TableOverride, ViewCabFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 25.f);
   PropFloat(TableOverride, ViewCabLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.f);
   PropFloat(TableOverride, ViewCabScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewCabScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewCabScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
   PropFloat(TableOverride, ViewCabPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
   PropFloat(TableOverride, ViewCabPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(0.f));
   PropFloat(TableOverride, ViewCabPlayerZ, "Camera Z"s, "View point vertical offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
   PropFloat(TableOverride, ViewCabHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
   PropFloat(TableOverride, ViewCabVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
   PropFloat(
      TableOverride, ViewCabWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloat(
      TableOverride, ViewCabWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
   PropFloatStepped(TableOverride, ViewCabRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropFloat(TableOverride, Difficulty, "Difficulty"s, "Overall difficulty (slope, flipper size, trajectories scattering,...)"s, 0.f, 100.f, 100.f);
   PropFloat(TableOverride, Exposure, "Camera Exposure"s, "Overall brightness of the rendered scene"s, 0.f, 2.f, 1.f);
#ifdef ENABLE_BGFX
   PropEnum(TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s,
      "Neutral"s, "AgX Punchy"s);
#else
   PropEnum(
      TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s);
#endif

   // DMD Display profiles
   // Legacy
   PropBool(DMD, Profile1Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, true);
   PropBool(DMD, Profile1ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile1DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x2D52FF);
   PropInt(DMD, Profile1UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile1DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile1DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
   PropFloat(DMD, Profile1DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile1DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Classic Neon plasma DMD
   PropBool(DMD, Profile2Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile2ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile2DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x2D52FF);
   PropInt(DMD, Profile2UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile2DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile2DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.6f);
   PropFloat(DMD, Profile2DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile2DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Red Led DMD (used after RoHS regulation entry into force)
   PropBool(DMD, Profile3Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile3ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile3DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x1523FF);
   PropInt(DMD, Profile3UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile3DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile3DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
   PropFloat(DMD, Profile3DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile3DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Green Led
   PropBool(DMD, Profile4Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile4ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile4DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x23FF15);
   PropInt(DMD, Profile4UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile4DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile4DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
   PropFloat(DMD, Profile4DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile4DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Yellow Led
   PropBool(DMD, Profile5Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile5ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile5DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x23FFFF);
   PropInt(DMD, Profile5UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile5DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile5DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
   PropFloat(DMD, Profile5DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile5DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Generic Plasma
   PropBool(DMD, Profile6Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile6ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile6DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0xFFFFFF);
   PropInt(DMD, Profile6UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile6DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile6DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.6f);
   PropFloat(DMD, Profile6DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile6DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Generic Led
   PropBool(DMD, Profile7Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
   PropBool(DMD, Profile7ScaleFX, "Use ScaleFX"s, "Upscale DMD using ScaleFX"s, false);
   PropInt(DMD, Profile7DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0xFFFFFF);
   PropInt(DMD, Profile7UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(DMD, Profile7DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
   PropFloat(DMD, Profile7DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
   PropFloat(DMD, Profile7DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
   PropFloat(DMD, Profile7DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
   // Array access
   PropArray(DMD, ProfileLegacy, bool, Int, Int, m_propDMD_Profile1Legacy, m_propDMD_Profile2Legacy, m_propDMD_Profile3Legacy, m_propDMD_Profile4Legacy, m_propDMD_Profile5Legacy,
      m_propDMD_Profile6Legacy, m_propDMD_Profile7Legacy);
   PropArray(DMD, ProfileScaleFX, bool, Int, Int, m_propDMD_Profile1ScaleFX, m_propDMD_Profile2ScaleFX, m_propDMD_Profile3ScaleFX, m_propDMD_Profile4ScaleFX, m_propDMD_Profile5ScaleFX,
      m_propDMD_Profile6ScaleFX, m_propDMD_Profile7ScaleFX);
   PropArray(DMD, ProfileDotTint, int, Int, Int, m_propDMD_Profile1DotTint, m_propDMD_Profile2DotTint, m_propDMD_Profile3DotTint, m_propDMD_Profile4DotTint, m_propDMD_Profile5DotTint,
      m_propDMD_Profile6DotTint, m_propDMD_Profile7DotTint);
   PropArray(DMD, ProfileUnlitDotColor, int, Int, Int, m_propDMD_Profile1UnlitDotColor, m_propDMD_Profile2UnlitDotColor, m_propDMD_Profile3UnlitDotColor, m_propDMD_Profile4UnlitDotColor,
      m_propDMD_Profile5UnlitDotColor, m_propDMD_Profile6UnlitDotColor, m_propDMD_Profile7UnlitDotColor);
   PropArray(DMD, ProfileDotBrightness, float, Float, Float, m_propDMD_Profile1DotBrightness, m_propDMD_Profile2DotBrightness, m_propDMD_Profile3DotBrightness,
      m_propDMD_Profile4DotBrightness, m_propDMD_Profile5DotBrightness, m_propDMD_Profile6DotBrightness, m_propDMD_Profile7DotBrightness);
   PropArray(DMD, ProfileDotSize, float, Float, Float, m_propDMD_Profile1DotSize, m_propDMD_Profile2DotSize, m_propDMD_Profile3DotSize, m_propDMD_Profile4DotSize, m_propDMD_Profile5DotSize,
      m_propDMD_Profile6DotSize, m_propDMD_Profile7DotSize);
   PropArray(DMD, ProfileDotSharpness, float, Float, Float, m_propDMD_Profile1DotSharpness, m_propDMD_Profile2DotSharpness, m_propDMD_Profile3DotSharpness, m_propDMD_Profile4DotSharpness,
      m_propDMD_Profile5DotSharpness, m_propDMD_Profile6DotSharpness, m_propDMD_Profile7DotSharpness);
   PropArray(DMD, ProfileDiffuseGlow, float, Float, Float, m_propDMD_Profile1DiffuseGlow, m_propDMD_Profile2DiffuseGlow, m_propDMD_Profile3DiffuseGlow, m_propDMD_Profile4DiffuseGlow,
      m_propDMD_Profile5DiffuseGlow, m_propDMD_Profile6DiffuseGlow, m_propDMD_Profile7DiffuseGlow);

   // Alphanumeric Segment Display profiles
   // Neon Plasma
   PropInt(Alpha, Profile1Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x001E96FF); // 0x002D52FF
   PropInt(Alpha, Profile1Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile1Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile1DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // VFD Blueish
   PropInt(Alpha, Profile2Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFEF3F); // 0x00FFEFBF
   PropInt(Alpha, Profile2Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile2Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile2DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // VFD Greenish
   PropInt(Alpha, Profile3Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00ECFF48);
   PropInt(Alpha, Profile3Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile3Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile3DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Red Led
   PropInt(Alpha, Profile4Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x001523FF);
   PropInt(Alpha, Profile4Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile4Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile4DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Green Led
   PropInt(Alpha, Profile5Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x0023FF15);
   PropInt(Alpha, Profile5Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile5Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile5DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Yellow Led
   PropInt(Alpha, Profile6Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x0023FFFF);
   PropInt(Alpha, Profile6Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile6Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile6DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Generic Plasma
   PropInt(Alpha, Profile7Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFFFFF);
   PropInt(Alpha, Profile7Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile7Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile7DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Generic Led
   PropInt(Alpha, Profile8Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFFFFF);
   PropInt(Alpha, Profile8Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
   PropFloat(Alpha, Profile8Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
   PropFloat(Alpha, Profile8DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
   // Array access
   PropArray(Alpha, ProfileColor, int, Int, Int, m_propAlpha_Profile1Color, m_propAlpha_Profile2Color, m_propAlpha_Profile3Color, m_propAlpha_Profile4Color, m_propAlpha_Profile5Color,
      m_propAlpha_Profile6Color, m_propAlpha_Profile7Color, m_propAlpha_Profile8Color);
   PropArray(Alpha, ProfileUnlit, int, Int, Int, m_propAlpha_Profile1Unlit, m_propAlpha_Profile2Unlit, m_propAlpha_Profile3Unlit, m_propAlpha_Profile4Unlit, m_propAlpha_Profile5Unlit,
      m_propAlpha_Profile6Unlit, m_propAlpha_Profile7Unlit, m_propAlpha_Profile8Unlit);
   PropArray(Alpha, ProfileBrightness, float, Float, Float, m_propAlpha_Profile1Brightness, m_propAlpha_Profile2Brightness, m_propAlpha_Profile3Brightness, m_propAlpha_Profile4Brightness,
      m_propAlpha_Profile5Brightness, m_propAlpha_Profile6Brightness, m_propAlpha_Profile7Brightness, m_propAlpha_Profile8Brightness);
   PropArray(Alpha, ProfileDiffuseGlow, float, Float, Float, m_propAlpha_Profile1DiffuseGlow, m_propAlpha_Profile2DiffuseGlow, m_propAlpha_Profile3DiffuseGlow, m_propAlpha_Profile4DiffuseGlow,
      m_propAlpha_Profile5DiffuseGlow, m_propAlpha_Profile6DiffuseGlow, m_propAlpha_Profile7DiffuseGlow, m_propAlpha_Profile8DiffuseGlow);

   // Parts Defaults: Balls
   PropFloat(DefaultPropsBall, Mass, "Ball Mass"s, ""s, 0.1f, 2.f, 1.f);
   PropFloat(DefaultPropsBall, Radius, "Ball Radius"s, ""s, 0.1f, 50.f, 25.f);
   PropBool(DefaultPropsBall, ForceReflection, "Force Reflection"s, ""s, false);
   PropBool(DefaultPropsBall, DecalMode, "Decal Mode"s, ""s, false);
   PropString(DefaultPropsBall, Image, "Ball Image"s, ""s, ""s);
   PropString(DefaultPropsBall, DecalImage, "Decal Image"s, ""s, ""s);
   PropFloat(DefaultPropsBall, BulbIntensityScale, "Bulb Reflection Intensity Scale"s, ""s, 0.f, 10.f, 1.f);
   PropFloat(DefaultPropsBall, PFReflStrength, "Playfield Reflection Strength"s, ""s, 0.f, 10.f, 1.f);
   PropInt(DefaultPropsBall, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, 0xFFFFFF);
   PropBool(DefaultPropsBall, SphereMap, "Use Sphere Mapping"s, "Use sphere mapped 3D texturing"s, true);
   PropBool(DefaultPropsBall, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsBall, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsBall, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Bumper
   PropBool(DefaultPropsBumper, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsBumper, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsBumper, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Decal

   // Parts Defaults: DispReel
   PropBool(DefaultPropsDispReel, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsDispReel, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Flasher
   PropBool(DefaultPropsFlasher, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsFlasher, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Flipper
   PropBool(DefaultPropsFlipper, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsFlipper, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsFlipper, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Gate
   PropBool(DefaultPropsGate, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsGate, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsGate, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: HitTarget
   PropBool(DefaultPropsHitTarget, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsHitTarget, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsHitTarget, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Kicker
   PropBool(DefaultPropsKicker, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsKicker, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Light
   PropBool(DefaultPropsLight, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsLight, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsLight, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: LightSeq
   PropBool(DefaultPropsLightSeq, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsLightSeq, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: PartGroup
   PropBool(DefaultPropsPartGroup, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsPartGroup, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Plunger
   PropBool(DefaultPropsPlunger, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsPlunger, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsPlunger, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Primitive
   PropBool(DefaultPropsPrimitive, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsPrimitive, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsPrimitive, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Ramp
   PropBool(DefaultPropsRamp, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsRamp, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsRamp, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Rubber
   PropBool(DefaultPropsRubber, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsRubber, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsRubber, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Spinner
   PropBool(DefaultPropsSpinner, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsSpinner, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsSpinner, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Surface
   PropBool(DefaultPropsSurface, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsSurface, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsSurface, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Textbox
   PropBool(DefaultPropsTextbox, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsTextbox, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Timer
   PropBool(DefaultPropsTimer, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsTimer, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Parts Defaults: Trigger
   PropBool(DefaultPropsTrigger, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsTrigger, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsTrigger, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

   // Core plugins
#ifdef __LIBVPINBALL__
   static inline constexpr bool isLibVPX = true;
#else
   static inline constexpr bool isLibVPX = false;
#endif
#ifdef __STANDALONE__
   static inline constexpr bool isStandalone = true;
#else
   static inline constexpr bool isStandalone = false;
#endif
   PropBool(PluginAlphaDMD, Enable, "Enable"s, "Enable AlphaDMD plugin"s, false);
   PropBool(PluginB2SLegacy, Enable, "Enable"s, "Enable legacy B2S plugin plugin"s, isStandalone);
   PropBool(PluginFlexDMD, Enable, "Enable"s, "Enable FlexDMD plugin"s, isStandalone);
   PropBool(PluginPinMAME, Enable, "Enable"s, "Enable PinMAME plugin"s, isStandalone);
   PropBool(PluginPUP, Enable, "Enable"s, "Enable PinUp player plugin"s, isLibVPX);
   PropBool(PluginScoreView, Enable, "Enable"s, "Enable ScoreView player plugin"s, isStandalone);
   PropBool(PluginWMP, Enable, "Enable"s, "Enable WMP plugin"s, isStandalone);

#undef PropBool
#undef PropInt
#undef PropEnum
#undef PropFloat
#undef PropString
#undef PropArray


public:
   Settings(Settings *parent = nullptr);

   void SetParent(Settings *parent)
   {
      m_parent = parent;
      m_store.UpdateParent();
   }

   void SetIniPath(const string &path) { m_iniPath = path; }
   bool LoadFromFile(const string &path, const bool createDefault);
   void SaveToFile(const string &path);
   void Save();

   void Copy(const Settings &settings);

   // Only actually save the settings if they have been modified. If you want to force a save (for example if filepath has changed), you need to explicitly set the modified flag
   bool IsModified() const { return m_modified; }
   void SetModified(const bool modified) { m_modified = modified; }

   enum Section
   {
      Controller,

      // UI and Player stuff
      Editor,
      Standalone,
      Player, /* Main playfield Rendering & Display */
      Input,
      DMD, /* DMD Rendering */
      Alpha, /* Alpha segment Rendering */
      Backglass, /* Backglass Display */
      ScoreView, /* ScoreView Display */
      Topper, /* Topper Display */
      PlayerVR,
      RecentDir,
      Version,
      CVEdit,
      TableOverride,
      TableOption,
      ControllerDevices,

      // Optional user defaults for each element
      DefaultPropsBall,
      DefaultPropsBumper,
      DefaultPropsDecal,
      DefaultPropsEMReel,
      DefaultPropsFlasher,
      DefaultPropsFlipper,
      DefaultPropsGate,
      DefaultPropsHitTarget,
      DefaultPropsKicker,
      DefaultPropsLight,
      DefaultPropsLightSequence,
      DefaultPropsPlunger,
      DefaultPropsPrimitive,
      DefaultPropsRamp,
      DefaultPropsRubber,
      DefaultPropsSpinner,
      DefaultPropsWall,
      DefaultPropsTarget,
      DefaultPropsTextBox,
      DefaultPropsTimer,
      DefaultPropsTrigger,
      DefaultCamera,
      DefaultPropsPartGroup,

      // Plugin pages
      Plugin00
   };

   static Section GetSection(const string &szName);
   static const string &GetSectionName(const Section section);
   static int GetNPluginSections() { return (int)m_settingKeys.size() - Plugin00; }

   bool HasValue(const Section section, const string &key, const bool searchParent = false) const;

   bool LoadValue(const Section section, const string &key, string &val) const;
   bool LoadValue(const Section section, const string &key, float &pfloat) const;
   bool LoadValue(const Section section, const string &key, int &pint) const;
   bool LoadValue(const Section section, const string &key, unsigned int &val) const;

   // The following method must only be used for settings previously validated to guarantee successfull loading
   void Validate(const bool addDefaults);
   string LoadValueString(const Section section, const string &key) const
   {
      string v;
      LoadValue(section, key, v);
      return v;
   }
   float LoadValueFloat(const Section section, const string &key) const
   {
      float v;
      bool ok = LoadValue(section, key, v);
      assert(ok);
      return v;
   }
   bool LoadValueBool(const Section section, const string &key) const
   {
      unsigned int v;
      bool ok = LoadValue(section, key, v);
      assert(ok);
      return !!v;
   }
   unsigned int LoadValueUInt(const Section section, const string &key) const
   {
      unsigned int v;
      bool ok = LoadValue(section, key, v);
      assert(ok);
      return v;
   }

   float LoadValueWithDefault(const Section section, const string &key, const float def) const;
   int LoadValueWithDefault(const Section section, const string &key, const int def) const;
   bool LoadValueWithDefault(const Section section, const string &key, const bool def) const;
   string LoadValueWithDefault(const Section section, const string &key, const string &def) const;

   bool SaveValue(const Section section, const string &key, const string &val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const float val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const int val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const unsigned int val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const bool val, const bool overrideMode = false);

   bool DeleteValue(const Section section, const string &key, const bool deleteFromParent = false);
   bool DeleteSubKey(const Section section, const bool deleteFromParent = false);

   enum OptionUnit
   {
      OT_NONE, // Display without a unit
      OT_PERCENT, // Shows valut multiplied by 100, with % as the unit
   };
   struct OptionDef
   {
      Section section;
      string id, name;
      int showMask;
      float minValue, maxValue, step, defaultValue, value;
      OptionUnit unit;
      vector<string> literals;
      string tokenizedLiterals;
   };
   OptionDef &RegisterSetting(const Section section, const string &id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step, float defaultValue,
      OptionUnit unit, const vector<string> &literals);
   static const vector<OptionDef> &GetPluginSettings() { return m_pluginOptions; }

private:
   void RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments = string());
   void RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments = string());

   bool m_modified = false;
   unsigned int m_modificationIndex = 0;
   string m_iniPath;
   mINI::INIStructure m_ini;
   Settings *m_parent;

   // Shared across all settings
   static vector<OptionDef> m_pluginOptions;
   static vector<string> m_settingKeys;

   // Custom store that delegates to existing implementation while migrating
   class MigrationStore : public VPX::Properties::PropertyStore
   {
   public:
      explicit MigrationStore(Settings *settings)
         : PropertyStore(Settings::GetRegistry())
         , m_settings(settings)
         , m_cache(Settings::GetRegistry())
         , m_cacheModificationIndex(settings->m_modificationIndex - 1)
      {
         if (m_settings->m_parent)
            m_cache = VPX::Properties::LayeredINIPropertyStore(m_settings->m_parent->m_store.m_cache);
      }
      ~MigrationStore() override = default;

      void UpdateParent()
      {
         if (m_settings->m_parent)
            m_cache = VPX::Properties::LayeredINIPropertyStore(m_settings->m_parent->m_store.m_cache);
         else
            m_cache = VPX::Properties::LayeredINIPropertyStore(Settings::GetRegistry());
         m_cacheModificationIndex = m_settings->m_modificationIndex - 1;
      }

      void Reset(VPX::Properties::PropertyRegistry::PropId propId) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->DeleteValue(section, prop->m_propId, true);
      }

      void UpdateCache() const
      {
         if (m_settings->m_parent)
            m_settings->m_parent->m_store.UpdateCache();
         if (m_cacheModificationIndex != m_settings->m_modificationIndex)
         {
            m_cache.Load(m_settings->m_ini);
            m_cacheModificationIndex = m_settings->m_modificationIndex;
         }
      }

      int GetInt(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         UpdateCache();
         const int cachedValue = m_cache.GetInt(propId);
         /* const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         int value;
         switch (prop->m_type)
         {
         case VPX::Properties::PropertyDef::Type::Int: value = m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetIntProperty(propId)->m_def); break;
         case VPX::Properties::PropertyDef::Type::Bool: value = m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetBoolProperty(propId)->m_def); break;
         case VPX::Properties::PropertyDef::Type::Enum: value = m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetEnumProperty(propId)->m_def); break;
         default: assert(false); return 0;
         }
         assert(cachedValue == value); */
         return cachedValue;
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, int value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

      float GetFloat(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         UpdateCache();
         const float cachedValue = m_cache.GetFloat(propId);
         /* const auto prop = Settings::GetRegistry().GetFloatProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         const float value = m_settings->LoadValueWithDefault(section, prop->m_propId, prop->m_def);
         assert(cachedValue == value); */
         return cachedValue;
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, float value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

      const string &GetString(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         UpdateCache();
         const string &cachedValue = m_cache.GetString(propId);
         /* const auto prop = Settings::GetRegistry().GetStringProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         const string value = m_settings->LoadValueWithDefault(section, prop->m_propId, prop->m_def);
         assert(cachedValue == value); */
         return cachedValue;
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, const string &value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

   private:
      Settings *m_settings;
      mutable VPX::Properties::LayeredINIPropertyStore m_cache;
      mutable unsigned int m_cacheModificationIndex;
   } m_store;
};
