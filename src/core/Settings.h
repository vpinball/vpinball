// license:GPLv3+

#pragma once

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "unordered_dense.h"

#include "PropertyRegistry.h"
#include "LayeredINIPropertyStore.h"

#include "vpversion.h"


// This class holds the settings registry.
// A setting registry can have a parent, in which case, missing settings will be looked for in the parent.
// This is used to allow overriding part of the settings for a specific table while still using the base application value for others.
class Settings final
{
public:
   Settings();
   Settings(Settings *parent);

   void SetIniPath(const string &path);
   bool Load(const bool createDefault);
   void Load(const Settings &settings);
   void Save();
   bool IsModified() const { return m_store.IsModified(); }
   void SetModified(const bool modified) { m_store.SetModified(modified); } // Allow to set the modified flag, for example to force a save

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

private:
   static inline const VPX::Properties::PropertyRegistry::PropId m_propInvalid {};
   static string GetBackwardCompatibleSection(const string &groupId);
   string m_iniPath;
   Settings *const m_parent;
   VPX::Properties::LayeredINIPropertyStore m_store;

public:
#ifdef __GNUC__
#define PropBool(groupId, propId, label, comment, defVal)                                                                                                                                    \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId;                                                                                                        \
   static inline const VPX::Properties::BoolPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId); }                          \
   static inline const bool Get##groupId##_##propId##_Default() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId)->m_def; }                                                 \
   static inline const void Set##groupId##_##propId##_Default(bool v) { GetRegistry().Register(GetRegistry().GetBoolProperty(m_prop##groupId##_##propId)->WithDefault(v)); }                 \
   inline bool Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                        \
   inline void Set##groupId##_##propId(bool v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#else
#define PropBool(groupId, propId, label, comment, defVal)                                                                                                                                    \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, defVal));                                 \
   static inline const VPX::Properties::BoolPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId); }                          \
   static inline const bool Get##groupId##_##propId##_Default() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId)->m_def; }                                                 \
   static inline const void Set##groupId##_##propId##_Default(bool v) { GetRegistry().Register(GetRegistry().GetBoolProperty(m_prop##groupId##_##propId)->WithDefault(v)); }                 \
   inline bool Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                        \
   inline void Set##groupId##_##propId(bool v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#endif

#ifdef __GNUC__
#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal)                                                                                                                     \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId;                                                                                                        \
   static inline const VPX::Properties::IntPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId); }                            \
   static inline const int Get##groupId##_##propId##_Default() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId)->m_def; }                                                   \
   static inline const void Set##groupId##_##propId##_Default(int v) { GetRegistry().Register(GetRegistry().GetIntProperty(m_prop##groupId##_##propId)->WithDefault(v)); }                   \
   inline int Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                         \
   inline void Set##groupId##_##propId(int v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                 \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#else
#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal)                                                                                                                     \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, minVal, maxVal, defVal));                  \
   static inline const VPX::Properties::IntPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId); }                            \
   static inline const int Get##groupId##_##propId##_Default() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId)->m_def; }                                                   \
   static inline const void Set##groupId##_##propId##_Default(int v) { GetRegistry().Register(GetRegistry().GetIntProperty(m_prop##groupId##_##propId)->WithDefault(v)); }                   \
   inline int Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); }                                                                                         \
   inline void Set##groupId##_##propId(int v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                                 \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#endif
#define PropIntUnbounded(groupId, propId, label, comment, defVal) PropInt(groupId, propId, label, comment, INT_MIN, INT_MAX, defVal)

#ifdef __GNUC__
#define PropEnumWithMin(groupId, propId, label, comment, type, minVal, defVal, ...)                                                                                                          \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId;                                                                                                        \
   static inline const VPX::Properties::EnumPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetEnumProperty(m_prop##groupId##_##propId); }                          \
   static inline const type Get##groupId##_##propId##_Default() { return (type)(GetRegistry().GetEnumProperty(m_prop##groupId##_##propId)->m_def); }                                         \
   static inline const void Set##groupId##_##propId##_Default(type v) { GetRegistry().Register(GetRegistry().GetEnumProperty(m_prop##groupId##_##propId)->WithDefault((int)v)); }            \
   inline type Get##groupId##_##propId() const { return (type)(m_store.GetInt(m_prop##groupId##_##propId)); }                                                                                \
   inline void Set##groupId##_##propId(type v, bool asTableOverride) { Set(m_prop##groupId##_##propId, (int)v, asTableOverride); }                                                           \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#else
#define PropEnumWithMin(groupId, propId, label, comment, type, minVal, defVal, ...)                                                                                                          \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId = GetRegistry().Register(                                                                        \
      std::make_unique<VPX::Properties::EnumPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, minVal, defVal, vector<string> { __VA_ARGS__ }));                  \
   static inline const VPX::Properties::EnumPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetEnumProperty(m_prop##groupId##_##propId); }                          \
   static inline const type Get##groupId##_##propId##_Default() { return (type)(GetRegistry().GetEnumProperty(m_prop##groupId##_##propId)->m_def); }                                         \
   static inline const void Set##groupId##_##propId##_Default(type v) { GetRegistry().Register(GetRegistry().GetEnumProperty(m_prop##groupId##_##propId)->WithDefault((int)v)); }            \
   inline type Get##groupId##_##propId() const { return (type)(m_store.GetInt(m_prop##groupId##_##propId)); }                                                                                \
   inline void Set##groupId##_##propId(type v, bool asTableOverride) { Set(m_prop##groupId##_##propId, (int)v, asTableOverride); }                                                           \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#endif
#define PropEnum1(groupId, propId, label, comment, type, defVal, ...) PropEnumWithMin(groupId, propId, label, comment, type, 1, defVal, __VA_ARGS__)
#define PropEnum(groupId, propId, label, comment, type, defVal, ...) PropEnumWithMin(groupId, propId, label, comment, type, 0, defVal, __VA_ARGS__)

#ifdef __GNUC__
#define PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, step, defVal)                                                                                                      \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId;                                                                                                        \
   static inline const VPX::Properties::FloatPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId); }                        \
   static inline const float Get##groupId##_##propId##_Default() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId)->m_def; }                                               \
   static inline const void Set##groupId##_##propId##_Default(float v) { GetRegistry().Register(GetRegistry().GetFloatProperty(m_prop##groupId##_##propId)->WithDefault(v)); }               \
   inline float Get##groupId##_##propId() const { return m_store.GetFloat(m_prop##groupId##_##propId); }                                                                                     \
   inline void Set##groupId##_##propId(float v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                               \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#else
#define PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, step, defVal)                                                                                                      \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, minVal, maxVal, step, defVal));          \
   static inline const VPX::Properties::FloatPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId); }                        \
   static inline const float Get##groupId##_##propId##_Default() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId)->m_def; }                                               \
   static inline const void Set##groupId##_##propId##_Default(float v) { GetRegistry().Register(GetRegistry().GetFloatProperty(m_prop##groupId##_##propId)->WithDefault(v)); }               \
   inline float Get##groupId##_##propId() const { return m_store.GetFloat(m_prop##groupId##_##propId); }                                                                                     \
   inline void Set##groupId##_##propId(float v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                               \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#endif
#define PropFloatUnbounded(groupId, propId, label, comment, defVal) PropFloatStepped(groupId, propId, label, comment, FLT_MIN, FLT_MAX, 0.f, defVal)
#define PropFloat(groupId, propId, label, comment, minVal, maxVal, defVal) PropFloatStepped(groupId, propId, label, comment, minVal, maxVal, 0.f, defVal)

#ifdef __GNUC__
#define PropString(groupId, propId, label, comment, defVal)                                                                                                                                  \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId;                                                                                                        \
   static inline const VPX::Properties::StringPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId); }                      \
   static inline const string &Get##groupId##_##propId##_Default() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId)->m_def; }                                            \
   static inline const void Set##groupId##_##propId##_Default(const string &v) { GetRegistry().Register(GetRegistry().GetStringProperty(m_prop##groupId##_##propId)->WithDefault(v)); }      \
   inline const string &Get##groupId##_##propId() const { return m_store.GetString(m_prop##groupId##_##propId); }                                                                            \
   inline void Set##groupId##_##propId(const string &v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                       \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#else
#define PropString(groupId, propId, label, comment, defVal)                                                                                                                                  \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId                                                                                                  \
      = GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(GetBackwardCompatibleSection(#groupId), #propId, label, comment, defVal));                               \
   static inline const VPX::Properties::StringPropertyDef *Get##groupId##_##propId##_Property() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId); }                      \
   static inline const string &Get##groupId##_##propId##_Default() { return GetRegistry().GetStringProperty(m_prop##groupId##_##propId)->m_def; }                                            \
   static inline const void Set##groupId##_##propId##_Default(const string &v) { GetRegistry().Register(GetRegistry().GetStringProperty(m_prop##groupId##_##propId)->WithDefault(v)); }      \
   inline const string &Get##groupId##_##propId() const { return m_store.GetString(m_prop##groupId##_##propId); }                                                                            \
   inline void Set##groupId##_##propId(const string &v, bool asTableOverride) { Set(m_prop##groupId##_##propId, v, asTableOverride); }                                                       \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }
#endif

#ifdef __GNUC__
#define PropArray(groupId, propId, type, propType, getType, ...)                                                                                                                             \
   static const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId[];                                                                                                      \
   static inline const VPX::Properties::propType##PropertyDef *Get##groupId##_##propId##_Property(int index)                                                                                 \
   {                                                                                                                                                                                         \
      return GetRegistry().Get##propType##Property(m_prop##groupId##_##propId[index]);                                                                                                       \
   }                                                                                                                                                                                         \
   inline type Get##groupId##_##propId(int index) const { return (type)(m_store.Get##getType(m_prop##groupId##_##propId[index])); }                                                          \
   inline void Set##groupId##_##propId(int index, type v, bool asTableOverride) { Set(m_prop##groupId##_##propId[index], (type)v, asTableOverride); }                                        \
   inline void Reset##groupId##_##propId(int index) { Reset(m_prop##groupId##_##propId[index]); }
#else
#define PropArray(groupId, propId, type, propType, getType, ...)                                                                                                                             \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId[] = { __VA_ARGS__ };                                                                             \
   static inline const VPX::Properties::propType##PropertyDef *Get##groupId##_##propId##_Property(int index)                                                                                 \
   {                                                                                                                                                                                         \
      return GetRegistry().Get##propType##Property(m_prop##groupId##_##propId[index]);                                                                                                       \
   }                                                                                                                                                                                         \
   inline type Get##groupId##_##propId(int index) const { return (type)(m_store.Get##getType(m_prop##groupId##_##propId[index])); }                                                          \
   inline void Set##groupId##_##propId(int index, type v, bool asTableOverride) { Set(m_prop##groupId##_##propId[index], (type)v, asTableOverride); }                                        \
   inline void Reset##groupId##_##propId(int index) { Reset(m_prop##groupId##_##propId[index]); }
#endif
   #include "Settings_properties.inl"

#undef PropBool
#undef PropInt
#undef PropEnum
#undef PropFloat
#undef PropString
#undef PropArray
};
