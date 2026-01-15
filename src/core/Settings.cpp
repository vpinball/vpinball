// license:GPLv3+

#include "core/stdafx.h"
#include "Settings.h"

#include <cstdio>
#include <filesystem>


VPX::Properties::PropertyRegistry &Settings::GetRegistry()
{
   static VPX::Properties::PropertyRegistry registry;
   return registry;
}

string Settings::GetBackwardCompatibleSection(const string &groupId)
{
   if (groupId.starts_with("Plugin"))
   {
      return "Plugin." + groupId.substr(6);
   }
   else if (groupId.starts_with("DefaultProps"))
   {
      return "DefaultProps\\" + groupId.substr(12);
   }
   return groupId;
}

Settings::Settings()
   : m_parent(nullptr)
   , m_store(GetRegistry())
{
}

Settings::Settings(Settings *parent)
   : m_parent(parent)
   , m_store(parent->m_store)
{
   assert(parent != nullptr);
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, float v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Float);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, int v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Int || GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Enum);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, bool v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Bool);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, const string &v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::String);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::SetIniPath(const string &path)
{
   m_store.SetIniPath(path);
}

const string& Settings::GetIniPath() const
{
   return m_store.GetIniPath();
}

void Settings::UpdateDefaults()
{
   // Update some settings defaults that depends either on other settings or on host machine
   // If parented, we use the parent's value as our defaults, so no need to redefine anything
   if (m_parent)
      return;
   auto& reg = GetRegistry();

   // Windows default depends on the display device specs
   SDL_InitSubSystem(SDL_INIT_VIDEO);
   for (int i = 0; i < 4; i++)
   {
      VPX::RenderOutput::OutputMode mode = i == VPXWindowId::VPXWINDOW_Playfield ? VPX::RenderOutput::OutputMode::OM_WINDOW : (VPX::RenderOutput::OutputMode)GetWindow_Mode(i);
      switch (mode)
      {
      case VPX::RenderOutput::OutputMode::OM_DISABLED: break;

      case VPX::RenderOutput::OutputMode::OM_WINDOW:
      {
         // Android won't have window dimension until after we launch VPinballPlayerActivity
         if (g_isAndroid)
            break;

         const auto& conf = VPX::Window::GetDisplayConfig(GetWindow_Display(i));
         reg.Register(GetWindow_FSWidth_Property(i)->WithDefault(conf.width));
         reg.Register(GetWindow_FSHeight_Property(i)->WithDefault(conf.height));
         reg.Register(GetWindow_Width_Property(i)->WithDefault(i == 0 ? conf.width : (conf.width / 4)));
         reg.Register(GetWindow_Height_Property(i)->WithDefault(i == 0 ? conf.height : min(conf.width * 4 / 9, conf.height)));
         break;
      }
      case VPX::RenderOutput::OutputMode::OM_EMBEDDED:
      {
         const auto w = GetWindow_FullScreen(VPXWindowId::VPXWINDOW_Playfield) ? GetWindow_FSWidth(VPXWindowId::VPXWINDOW_Playfield) : GetWindow_Width(VPXWindowId::VPXWINDOW_Playfield);
         const auto h = GetWindow_FullScreen(VPXWindowId::VPXWINDOW_Playfield) ? GetWindow_FSHeight(VPXWindowId::VPXWINDOW_Playfield) : GetWindow_Height(VPXWindowId::VPXWINDOW_Playfield);
         reg.Register(GetWindow_Width_Property(i)->WithDefault(w / 4));
         reg.Register(GetWindow_Height_Property(i)->WithDefault(min(w * 4 / 9, h)));
         break;
      }
      }
   }
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Settings::Reset() { m_store.Reset(); }

bool Settings::Load(const bool createDefault)
{
   if (m_store.Load())
   {
      PLOGI << "Settings file was loaded from '" << m_store.GetIniPath() << '\'';
      UpdateDefaults();
      return true;
   }
   else if (createDefault)
   {
      PLOGI << "Settings file was not found at '" << m_store.GetIniPath() << "', creating a default one";
      try
      {
         m_store.GenerateTemplate(m_store.GetIniPath());
      }
      catch (const std::exception&)
      {
         ShowError("Failed to generate the default setting file");
      }
      if (!m_store.Load())
      {
         PLOGE << "Loading of default settings file failed";
      }
      UpdateDefaults();
      return true;
   }
   else
   {
      PLOGI << "Settings file was not found at '" << m_store.GetIniPath() << '\'';
      return false;
   }
}

void Settings::Save()
{
   m_store.Save();
}

void Settings::Load(const Settings &settings)
{
   m_store.Load(settings.m_store);
   UpdateDefaults();
}

#ifdef __GNUC__
#define PropBoolBase(groupId, propId, label, comment, isContextual, defVal) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId = \
      GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, isContextual, defVal));

#define PropIntBase(groupId, propId, label, comment, isContextual, minVal, maxVal, defVal) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId = \
      GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, isContextual, minVal, maxVal, defVal));

#define PropEnumBase(groupId, propId, label, comment, isContextual, type, minVal, defVal, ...) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId = \
      GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, isContextual, minVal, defVal, vector<string>{__VA_ARGS__}));

#define PropFloatBase(groupId, propId, label, comment, isContextual, minVal, maxVal, step, defVal) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId = \
      GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, isContextual, minVal, maxVal, step, defVal));

#define PropStringBase(groupId, propId, label, comment, isContextual, defVal) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId = \
      GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, isContextual, defVal));

#define PropArray(groupId, propId, type, propType, getType, ...) \
   const VPX::Properties::PropertyRegistry::PropId Settings::m_prop##groupId##_##propId[] = { __VA_ARGS__ };

#define PropBool(groupId, propId, label, comment, defVal) PropBoolBase(groupId, propId, label, comment, false, defVal)
#define PropBoolDyn(groupId, propId, label, comment, defVal) PropBoolBase(groupId, propId, label, comment, true, defVal)

#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal) PropIntBase(groupId, propId, label, comment, false, minVal, maxVal, defVal)
#define PropIntDyn(groupId, propId, label, comment, minVal, maxVal, defVal) PropIntBase(groupId, propId, label, comment, true, minVal, maxVal, defVal)
#define PropIntUnbounded(groupId, propId, label, comment, defVal) PropIntBase(groupId, propId, label, comment, false, INT_MIN, INT_MAX, defVal)

#define PropEnumWithMin(groupId, propId, label, comment, type, minVal, defVal, ...) PropEnumBase(groupId, propId, label, comment, false, type, minVal, defVal, __VA_ARGS__)
#define PropEnum1(groupId, propId, label, comment, type, defVal, ...) PropEnumBase(groupId, propId, label, comment, false, type, 1, defVal, __VA_ARGS__)
#define PropEnum(groupId, propId, label, comment, type, defVal, ...) PropEnumBase(groupId, propId, label, comment, false, type, 0, defVal, __VA_ARGS__)
#define PropEnumDyn(groupId, propId, label, comment, type, defVal, ...) PropEnumBase(groupId, propId, label, comment, true, type, 0, defVal, __VA_ARGS__)

#define PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, step, defVal) PropFloatBase(groupId, propId, label, comment, false, minVal, maxVal, step, defVal)
#define PropFloatSteppedDyn(groupId, propId, label, comment, minVal, maxVal, step, defVal) PropFloatBase(groupId, propId, label, comment, true, minVal, maxVal, step, defVal)
#define PropFloatUnbounded(groupId, propId, label, comment, defVal) PropFloatBase(groupId, propId, label, comment, false, FLT_MIN, FLT_MAX, 0.f, defVal)
#define PropFloat(groupId, propId, label, comment, minVal, maxVal, defVal) PropFloatBase(groupId, propId, label, comment, false, minVal, maxVal, 0.f, defVal)
#define PropFloatDyn(groupId, propId, label, comment, minVal, maxVal, defVal) PropFloatBase(groupId, propId, label, comment, true, minVal, maxVal, 0.f, defVal)

#define PropString(groupId, propId, label, comment, defVal) PropStringBase(groupId, propId, label, comment, false, defVal)
#define PropStringDyn(groupId, propId, label, comment, defVal) PropStringBase(groupId, propId, label, comment, true, defVal)

#include "Settings_properties.inl"

#endif
