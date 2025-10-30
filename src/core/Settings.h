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
   // Custom store that delegates to existing implementation while migrating
   class MigrationStore : public VPX::Properties::PropertyStore
   {
   public:
      explicit MigrationStore(Settings* settings)
         : PropertyStore(Settings::GetRegistry())
         , m_settings(settings)
      {
      }
      ~MigrationStore() override = default;

      void Reset(VPX::Properties::PropertyRegistry::PropId propId) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->DeleteValue(section, prop->m_propId);
      }

      int GetInt(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         switch (prop->m_type)
         {
         case VPX::Properties::PropertyDef::Type::Int: return m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetIntProperty(propId)->m_def);
         case VPX::Properties::PropertyDef::Type::Bool: return m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetBoolProperty(propId)->m_def);
         case VPX::Properties::PropertyDef::Type::Enum: return m_settings->LoadValueWithDefault(section, prop->m_propId, Settings::GetRegistry().GetEnumProperty(propId)->m_def);
         default: assert(false); return 0;
         }
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, int value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

      float GetFloat(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         const auto prop = Settings::GetRegistry().GetFloatProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         return m_settings->LoadValueWithDefault(section, prop->m_propId, prop->m_def);
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, float value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

      const string& GetString(VPX::Properties::PropertyRegistry::PropId propId) const override
      {
         const auto prop = Settings::GetRegistry().GetStringProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         static string value;
         value = m_settings->LoadValueWithDefault(section, prop->m_propId, prop->m_def);
         return value;
      }
      void Set(VPX::Properties::PropertyRegistry::PropId propId, const string& value) override
      {
         const auto &prop = Settings::GetRegistry().GetProperty(propId);
         const Settings::Section section = Settings::GetSection(prop->m_groupId);
         m_settings->SaveValue(section, prop->m_propId, value);
      }

   private:
      Settings* m_settings;
   } m_store;

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

#define PropBool(groupId, propId, label, comment, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(#groupId, #propId, label, comment, defVal)); \
   static inline const VPX::Properties::BoolPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId); } \
   inline bool Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(bool v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, defVal)); \
   static inline const VPX::Properties::IntPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId); } \
   inline int Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(int v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropEnum(groupId, propId, label, comment, type, defVal, ...) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(#groupId, #propId, label, comment, 0, defVal, vector<string> { __VA_ARGS__ })); \
   static inline const VPX::Properties::EnumPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetEnumProperty(m_prop##groupId##_##propId); } \
   inline type Get##groupId##_##propId() const { return (type)(m_store.GetInt(m_prop##groupId##_##propId)); } \
   inline void Set##groupId##_##propId(type v, bool asTableOverride) { Set(m_prop##groupId##_##propId, (int)v, asTableOverride); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropFloat(groupId, propId, label, comment, minVal, maxVal, step, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, step, defVal)); \
   static inline const VPX::Properties::FloatPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId); } \
   inline float Get##groupId##_##propId() const { return m_store.GetFloat(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(float v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropString(groupId, propId, label, comment, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(#groupId, #propId, label, comment, defVal)); \
   static inline const VPX::Properties::StringPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId); } \
   inline const string& Get##groupId##_##propId() const { return m_store.GetString(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(const string& v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

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

   // Graphics synchronisation and latency reduction
#if defined(ENABLE_BGFX)
   PropEnum(Player, SyncMode, "Synchronization"s,
      "None: Use this if your display supports variable refresh rate or if you are experiencing stutters.\r\n\r\nVertical Sync: Synchronize on display sync."s,
      int, 1, "No Sync", "Vertical Sync");
#else
   PropEnum(Player, SyncMode, "Synchronization"s,
      "None: No synchronization.\r\nVertical Sync: Synchronize on video sync which avoids video tearing, but has higher input latency.\r\nAdaptive Sync: Synchronize on video sync, "
      "except for late frames (below target FPS), also has higher input latency.\r\nFrame Pacing: Targets real time simulation with low input- and video-latency (also dynamically adjusts "
      "framerate)."s,
      int, 1, "No Sync", "Vertical Sync", "Adaptive Sync", "Frame Pacing");
#endif
   PropInt(Player, MaxFramerate, "Limit Framerate", "-1 will not limit FPS\r\n0 will limit to the display refresh rates\r\nOther values will limit the FPS to it (energy saving/less heat, framerate stability)"s, -1, 300, -1);
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
   PropFloat(Player, HDRGlobalExposure, "HDR Display Global Exposure", "Global exposure scale multiplier for HDR capable displays"s, 0.f, 5.f, 0.01f, 1.f);
   PropBool(Player, ForceBloomOff, "Disable Bloom"s, "Disable postprocessed bloom filter"s, false);
   PropBool(Player, ForceMotionBlurOff, "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s, false);
   PropBool(Player, ForceAnisotropicFiltering, "Force Anisotropic Filtering"s, "Force anisotropic filtering for better rendering quality at the cost of a bit of performance"s, true);
   PropBool(Player, CompressTextures, "Compress Textures"s, "Automatically compress textures at game startup (slow) for better performance"s, false);
   PropBool(Player, UseNVidiaAPI, "Alternative Depth Buffer"s, "Use NVidia API to manage Depth Buffer on DirectX 9 build. May solve some rendering issues"s, false);
   PropBool(Player, SoftwareVertexProcessing, "Software Vertex Processing"s, "Activate this if you have issues using an Intel graphics chip"s, false);
   PropBool(Player, DisableAO, "Disable Ambient Occlusion"s, ""s, false);
   PropBool(Player, DynamicAO, "Dynamic Ambient Occlusion"s, ""s, true);
   PropEnum(Player, PFReflection, "Reflection Quality"s, "Limit the quality of reflections for better performance.\r\n'Dynamic' is recommended and will give the best results, but may harm performance.\r\n'Static Only' has no performance cost (except for VR rendering).\r\nOther options feature different trade-offs between quality and performance."s, int, 0, "Disable Reflections"s, "Balls Only"s, "Static Only"s, "Static & Balls"s, "Static & Unsynced Dynamic"s, "Dynamic"s);
#ifndef __LIBVPINBALL__
   PropInt(Player, MaxTexDimension, "Maximum texture dimension"s, "Images sized above this limit will be automatically scaled down on load."s, 512, 16384, 0);
#else
   PropInt(Player, MaxTexDimension, "Maximum texture dimension"s, "Images sized above this limit will be automatically scaled down on load."s, 512, 16384, 1536);
#endif
   PropInt(Player, AlphaRampAccuracy, "Detail Level"s, "Images sized above this limit will be automatically scaled down on load."s, 1, 10, 10);

   // Aliasing & sharpening
   PropFloat(Player, AAFactor, "Full Scene Anti Aliasing"s, "Enables brute-force Up/Downsampling (similar to DSR).\r\nThis delivers very good quality but has a significant impact on performance.\r\n200% means twice the resolution to be handled while rendering"s, 0.5f, 2.f, 0.1f, 1.f);
   PropEnum(Player, MSAASamples, "MSAA level"s,
      "Set the amount of MSAA samples.\r\nMSAA can help reduce geometry aliasing at the cost of performance and GPU memory.\r\nThis can improve image quality if not using supersampling"s,
      int, 0, "Disabled",
      "4 Samples", "6 Samples", "8 Samples");
   PropEnum(Player, FXAA, "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s, int, 1, "Disabled"s,
      "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s);
   PropEnum(Player, Sharpen, "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, int, 0, "Disabled"s,
      "CAS"s, "Bilateral CAS"s);

   // Ball rendering
   PropBool(Player, BallAntiStretch, "Unstretch Ball"s, "Compensate ball stretching"s, false);
   PropBool(Player, DisableLightingForBalls, "Disable Ball Lighting"s, "Disable lighting and reflection effects on balls, e.g. to help the visually handicapped."s, false);
   PropBool(Player, BallTrail, "Ball Trail"s, "Legacy ball trails"s, false);
   PropFloat(Player, BallTrailStrength, "Ball Trail Strength", "Strength of the fake ball trail"s, 0.f, 5.f, 0.01f, 0.5f);
   
   // Misc player settings
   PropBool(Player, TouchOverlay, "Touch Overlay"s, "Display an overlay showing touch regions"s, false);
   PropBool(Player, EnableCameraModeFlyAround, "Legacy Fly Over Mode"s, "Enable moving camera when using Tweak menu (legacy, replaced by LiveUI fly mode)."s, false);

   // Nudge & Plumb settings
   PropFloat(Player, NudgeOrientation0, "Sensor #0 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 1.0f, 0.f);
   PropFloat(Player, NudgeOrientation1, "Sensor #1 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 1.0f, 0.f);
   PropBool(Player, NudgeFilter0, "Sensor #0 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, NudgeFilter1, "Sensor #1 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, SimulatedPlumb, "Plumb simulation"s, "Enable/Disable mechanical Tilt plumb simulation"s, true);
   PropFloat(Player, PlumbInertia, "Plumb Inertia", ""s, 0.001f, 1.f, 0.0001f, 0.35f);
   PropFloat(Player, PlumbThresholdAngle, "Plumb Threshold", "Define threshold angle at which a Tilt is caused"s, 5.0f, 60.f, 0.1f, 35.f);
   PropBool(Player, EnableLegacyNudge, "Legacy Keyboard nudge"s, "Enable/Disable legacy keyboard nudge mode"s, false);
   PropFloat(Player, LegacyNudgeStrength, "Legacy Nudge Strength"s, "Strength of nudge when using the legacy keyboard nudge mode"s, 0.f, 90.f, 0.1f, 1.f);
   PropFloat(Player, NudgeStrength, "Visual Nudge Strength"s, "Changes the visual effect/screen shaking when nudging the table."s, 0.f, 0.25f, 0.001f, 0.02f);

   // Plunger settings
   PropBool(Player, PlungerRetract, "One Second Retract"s, "Enable retracting the plunger after a 1 second press when using the digital plunger emulation through keyboard or joystick button"s, false);
   PropBool(Player, PlungerLinearSensor, "Linear Sensor"s, "Select between symmetric (linear) and assymetric sensor"s, false);

   // VR settings
   PropFloat(PlayerVR, Orientation, "View orientation"s, ""s, -180.f, 180.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableX, "View Offset X"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableY, "View Offset Y"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableZ, "View Offset Z"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropBool(PlayerVR, UsePassthroughColor, "Color Keyed Passthrough"s, "Replace VR background by a user defined color, to allow color keyed passthrough (for example using Virtual Desktop)"s, false);
   PropInt(PlayerVR, PassthroughColor, "Color Keyed Passthrough color"s, "Color that will replace the background"s, 0x000000, 0xFFFFFF, 0xBB4700);
   PropEnum(Player, VRPreview, "Preview mode"s, "Select preview mode"s, int, 1, "Disabled"s, "Left Eye"s, "Right Eye"s, "Both Eyes"s);
   PropBool(PlayerVR, ShrinkPreview, "Shrink preview"s, ""s, false);

   // Stereo settings
   PropBool(Player, Stereo3DEnabled, "Enable Stereo Rendering"s, "Allow to temporarily disable stereo rendering"s, false);
   PropEnum(Player, Stereo3D, "Stereo rendering"s, "Stereo rendering mode"s, StereoMode, 0, "Disabled"s, "Top / Bottom"s, "Interlaced (e.g. LG TVs)"s,
      "Flipped Interlaced (e.g. LG TVs)"s, "Side by Side"s, "Anaglyph Red/Cyan"s, "Anaglyph Green/Magenta"s, "Anaglyph Blue/Amber"s, "Anaglyph Cyan/Red"s, 
      "Anaglyph Magenta/Green"s, "Anaglyph Amber/Blue"s, "Anaglyph Custom 1"s, "Anaglyph Custom 2"s, "Anaglyph Custom 3"s, "Anaglyph Custom 4"s);
   PropFloat(Player, Stereo3DEyeSeparation, "Eye distance"s, "Physical distance (mm) between eyes"s, 5.f, 200.f, 0.1f, 63.f);
   PropFloat(Player, Stereo3DBrightness, "Stereo Brightness"s, "Brightness adjustment applied to stereo rendering"s, 0.f, 2.f, 0.01f, 1.f);
   PropFloat(Player, Stereo3DSaturation, "Stereo Saturation"s, "Saturation adjustment applied to stereo rendering"s, 0.f, 2.f, 0.01f, 1.f);
   PropFloat(Player, Stereo3DDefocus, "Anaglyph Defocus"s, "Defocusing of the lesser eye to anaglyph stereo rendering"s, 0.f, 1.f, 0.01f, 0.f);
   PropFloat(Player, Stereo3DLeftContrast, "Anaglyph Left Contrast"s, "Left eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 0.01f, 1.f);
   PropFloat(Player, Stereo3DRightContrast, "Anaglyph Right Contrast"s, "Right eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 0.01f, 1.f);
   PropEnum(Player, Anaglyph1Filter, "Anaglyph Filter #1"s, "Anaglyph filter applied to anaglyph profile #1"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph2Filter, "Anaglyph Filter #2"s, "Anaglyph filter applied to anaglyph profile #2"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph3Filter, "Anaglyph Filter #3"s, "Anaglyph filter applied to anaglyph profile #3"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph4Filter, "Anaglyph Filter #4"s, "Anaglyph filter applied to anaglyph profile #4"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph5Filter, "Anaglyph Filter #5"s, "Anaglyph filter applied to anaglyph profile #5"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph6Filter, "Anaglyph Filter #6"s, "Anaglyph filter applied to anaglyph profile #6"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph7Filter, "Anaglyph Filter #7"s, "Anaglyph filter applied to anaglyph profile #7"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph8Filter, "Anaglyph Filter #7"s, "Anaglyph filter applied to anaglyph profile #8"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph9Filter, "Anaglyph Filter #7"s, "Anaglyph filter applied to anaglyph profile #9"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
   PropEnum(Player, Anaglyph10Filter, "Anaglyph Filter #7"s, "Anaglyph filter applied to anaglyph profile #10"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);

   // Real world cabinet & player settings
   PropFloat(Player, ScreenWidth, "Screen Width"s, "Physical width (cm) of the display area of the playfield (main) screen."s, 5.f, 200.f, 0.1f, 95.89f);
   PropFloat(Player, ScreenHeight, "Screen Height"s, "Physical height (cm) of the display area of the playfield (main) screen."s, 5.f, 200.f, 0.1f, 53.94f);
   PropFloat(Player, ScreenInclination, "Screen Inclination"s, "Inclination (degree) of the playfield (main) screen. 0 is horizontal."s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(Player, LockbarWidth, "Lockbar Width"s, "Lockbar width in centimeters (measured on the cabinet)."s, 10.f, 150.f, 0.1f, 70.f);
   PropFloat(Player, LockbarHeight, "Lockbar Height"s, "Lockbar height in centimeters (measured on the cabinet, from ground to top of lockbar)."s, 0.f, 250.f, 0.1f, 85.f);
   PropFloat(Player, ScreenPlayerX, "Player X"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(Player, ScreenPlayerY, "Player Y"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -70.f, 30.f, 0.1f, -10.f);
   PropFloat(Player, ScreenPlayerZ, "Player Z"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, 30.f, 100.f, 0.1f, 70.f);

   // Overall scene lighting settings
   PropBool(Player, OverrideTableEmissionScale, "Override Table Light Level"s, "Replace table light level by a custom one"s, false);
   PropFloat(Player, EmissionScale, "Day/Night"s, "Select a custom level between daylight or night time lighting"s, 0.f, 1.f, 0.001f, 1.f);
   PropBool(Player, DynamicDayNight, "Use Automatic Light Level"s, "Automatically compute scene lighting based on sun's position"s, false);
   PropFloat(Player, Latitude, "Latitude"s, "Latitude used to compute sun's position"s, -90.f, 90.f, 0.01f, 52.52f);
   PropFloat(Player, Longitude, "Longitude"s, "Longitude used to compute sun's position"s, -180.f, 180.f, 0.01f, 13.37f);

   // Backglass anciliary window settings
   PropInt(Backglass, BackglassWndX, "Backglass X"s, "X position of the backglass window"s, -INT_MAX, INT_MAX, 0);
   PropInt(Backglass, BackglassWndY, "Backglass Y"s, "Y position of the backglass window"s, -INT_MAX, INT_MAX, 0);
   PropInt(Backglass, BackglassWndWidth, "Backglass Width"s, "Width of the backglass window"s, -INT_MAX, INT_MAX, 0);
   PropInt(Backglass, BackglassWndHeight, "Backglass Height"s, "Height of the backglass window"s, -INT_MAX, INT_MAX, 0);

   // ScoreView anciliary window settings
   PropInt(ScoreView, ScoreViewWndX, "ScoreView X"s, "X position of the 'Score View' window"s, -INT_MAX, INT_MAX, 0);
   PropInt(ScoreView, ScoreViewWndY, "ScoreView Y"s, "Y position of the 'Score View' window"s, -INT_MAX, INT_MAX, 0);
   PropInt(ScoreView, ScoreViewWndWidth, "ScoreView Width"s, "Width of the 'Score View' window"s, -INT_MAX, INT_MAX, 0);
   PropInt(ScoreView, ScoreViewWndHeight, "ScoreView Height"s, "Height of the 'Score View' window"s, -INT_MAX, INT_MAX, 0);

   // Debugging & Live editing settings
   PropBool(Editor, ThrowBallsAlwaysOn, "Throw Balls Always On"s, "Permanently enable 'throw ball' debugging mode"s, false);
   PropBool(Editor, BallControlAlwaysOn, "Ball COntrol Always On"s, "Permanently enable 'ball control' debugging mode"s, false);

   // Default camera setup
   PropEnum(DefaultCamera, DesktopMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(DefaultCamera, DesktopLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 25.f);
   PropFloat(DefaultCamera, DesktopFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 50.f);
   PropFloat(DefaultCamera, DesktopScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, DesktopScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, DesktopScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, DesktopCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(DefaultCamera, DesktopCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 20.f);
   PropFloat(DefaultCamera, DesktopCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(DefaultCamera, DesktopViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 0.1f, 14.f);
   PropEnum(DefaultCamera, FSSMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(DefaultCamera, FSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 50.f);
   PropFloat(DefaultCamera, FSSFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 77.f);
   PropFloat(DefaultCamera, FSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, FSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, FSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(DefaultCamera, FSSCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(DefaultCamera, FSSCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 20.f);
   PropFloat(DefaultCamera, FSSCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(DefaultCamera, FSSViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 0.1f, 22.f);

   // Table override settings
   // These properties are specials as they are meant to override table data at play time. They are not meant to be saved to the application setting file, but
   // only to table override settings. Also, their defaults are redefined when a table is played, depending on the table data and UI options.
   PropEnum(TableOverride, ViewDTMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewDTLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewDTLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(TableOverride, ViewDTScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(TableOverride, ViewDTScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 0.001f, 1.f);
   PropFloat(TableOverride, ViewDTPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 20.f);
   PropFloat(TableOverride, ViewDTPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewDTHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropEnum(TableOverride, ViewFSSMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewFSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewFSSLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewFSSHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropEnum(TableOverride, ViewCabMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 2, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewCabLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewCabLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewCabHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropFloat(TableOverride, Difficulty, "Difficulty"s, "Overall difficulty (slope, flipper size, trajectories scattering,...)"s, 0.f, 100.f, 0.1f, 100.f);
   PropFloat(TableOverride, Exposure, "Camera Exposure"s, "Overall brightness of the rendered scene"s, 0.f, 2.f, 0.1f, 1.f);
#ifdef ENABLE_BGFX
   PropEnum(TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s,
      "Neutral"s, "AgX Punchy"s);
#else
   PropEnum(
      TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s);
#endif

   // Parts Defaults: Balls
   PropFloat(DefaultPropsBall, Mass, "Ball Mass"s, ""s, 0.1f, 2.f, 0.001f, 1.f);
   PropFloat(DefaultPropsBall, Radius, "Ball Radius"s, ""s, 0.1f, 50.f, 0.001f, 25.f);
   PropBool(DefaultPropsBall, ForceReflection, "Force Reflection"s, ""s, false);
   PropBool(DefaultPropsBall, DecalMode, "Decal Mode"s, ""s, false);
   PropString(DefaultPropsBall, Image, "Ball Image"s, ""s, ""s);
   PropString(DefaultPropsBall, DecalImage, "Decal Image"s, ""s, ""s);
   PropFloat(DefaultPropsBall, BulbIntensityScale, "Bulb Reflection Intensity Scale"s, ""s, 0.f, 10.f, 0.001f, 1.f);
   PropFloat(DefaultPropsBall, PFReflStrength, "Playfield Reflection Strength"s, ""s, 0.f, 10.f, 0.001f, 1.f);
   PropInt(DefaultPropsBall, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, 0xFFFFFF);
   PropBool(DefaultPropsBall, SphereMap, "Use Sphere Mapping"s, "Use sphere mapped 3D texturing"s, true);
   PropBool(DefaultPropsBall, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
   PropBool(DefaultPropsBall, TimerEnabled, "Timer Enabled"s, ""s, false);
   PropInt(DefaultPropsBall, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

#undef PropBool
#undef PropInt
#undef PropEnum
#undef PropFloat
#undef PropString


public:
   Settings(Settings* parent = nullptr);

   void SetParent(Settings *parent) { m_parent = parent; }

   void SetIniPath(const string &path) { m_iniPath = path; }
   bool LoadFromFile(const string &path, const bool createDefault);
   void SaveToFile(const string &path);
   void Save();

   void CopyOverrides(const Settings &settings);

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

   static Section GetSection(const string& szName);
   static const string& GetSectionName(const Section section);
   static int GetNPluginSections() { return (int)m_settingKeys.size() - Plugin00; }

   bool HasValue(const Section section, const string &key, const bool searchParent = false) const;

   bool LoadValue(const Section section, const string &key, string &val) const;
   bool LoadValue(const Section section, const string &key, float &pfloat) const;
   bool LoadValue(const Section section, const string &key, int &pint) const;
   bool LoadValue(const Section section, const string &key, unsigned int &val) const;

   // The following method must only be used for settings previously validated to guarantee successfull loading
   void Validate(const bool addDefaults);
   void ResetValue(const Section section, const string &key);
   string LoadValueString(const Section section, const string &key) const { string v; LoadValue(section, key, v); return v; }
   float LoadValueFloat(const Section section, const string &key) const { float v; bool ok = LoadValue(section, key, v); assert(ok); return v; }
   bool LoadValueBool(const Section section, const string &key) const { unsigned int v; bool ok = LoadValue(section, key, v); assert(ok); return !!v; }
   int LoadValueInt(const Section section, const string &key) const { int v; bool ok = LoadValue(section, key, v); assert(ok); return v; }
   unsigned int LoadValueUInt(const Section section, const string &key) const { unsigned int v; bool ok = LoadValue(section, key, v); assert(ok); return v; }

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
   OptionDef& RegisterSetting(const Section section, const string &id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step, float defaultValue,
      OptionUnit unit, const vector<string> &literals);
   static const vector<OptionDef>& GetPluginSettings() { return m_pluginOptions; }

private:
#if 0
   bool LoadValue(const Section section, const string &key, void *const szbuffer, const size_t size) const;
   bool SaveValue(const Section section, const string &key, const char *val, const bool overrideMode = false);
#endif

   void RegisterStringSetting(const Section section, const string &key, const string &defVal, const bool addDefaults, const string &comments = string());
   void RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments = string());
   void RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments = string());
   void RegisterUIntSetting(const Section section, const string &key, const unsigned int defVal, const unsigned int minVal, const unsigned int maxVal, const bool addDefaults, const string &comments = string());
   void RegisterFloatSetting(const Section section, const string &key, const float defVal, const float minVal, const float maxVal, const bool addDefaults, const string &comments = string());

   bool m_modified = false;
   string m_iniPath;
   mINI::INIStructure m_ini;
   Settings * m_parent;

   // Shared across all settings
   static vector<OptionDef> m_pluginOptions;
   static vector<string> m_settingKeys;
};
