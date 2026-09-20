#pragma once

#include "fonts/IconsForkAwesome.h"
#include "utils/fileio.h"
#include "parts/Sound.h"
#include "parts/pintable.h"
#include "parts/Collection.h"
#include "parts/Material.h"
#include "renderer/Texture.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::EditorUI
{

class PropertyPane final
{
public:
   PropertyPane(PinTable* table);
   ~PropertyPane();

   void SetShowStartup(bool showStartup);

   bool BeginSection(const string& name);
   void EndSection();
   template <class T> T* GetEditedPart(T* obj) const;
   void TimerSection(IEditable* obj);

   enum class Unit
   {
      None,
      Percent, // 0..1 value
      PercentX100, // 0..100 value
      VPLength,
      VPMass,
      VPSpeed,
      Degree,
      Inches,
      Millimeters
   };
   static const char* GetUnitLabel(Unit unit);
   static void ConvertUnit(Unit from, Unit& to, float& value, int& nDecimalAdjust);
   static void ResolveUnit(Unit from, Unit& to, int& nDecimalAdjust); // Resolves display unit & decimals without converting a value
   void SetLengthUnit(Unit lengthUnit) { m_lengthUnit = lengthUnit; }

   void Header(const string& typeName, const std::function<wstring()>& getName, const std::function<void(const wstring&)>& setName);
   void EditableHeader(const string& typeName, IEditable* editable);
   void Separator(const string& label) const;
   template <class T> void Checkbox(T* obj, const string& label, const std::function<bool(const T*)>& getter, const std::function<void(T*, bool)>& setter);
   template <class T> void InputInt(T* obj, const string& label, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter);
   template <class T> void InputFloat(T* obj, const string& label, const std::function<float(const T*)>& getter, const std::function<void(T*, float)>& setter, Unit unit, int nDecimals);
   template <class T>
   void InputFloat2(T* obj, const string& label, const std::function<Vertex2D(const T*)>& getter, const std::function<void(T*, const Vertex2D&)>& setter, Unit unit, int nDecimals);
   template <class T>
   void InputFloat3(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter, Unit unit, int nDecimals);
   template <class T> void InputRGB(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter);
   template <class T> void InputString(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void Combo(T* obj, const string& label, const std::vector<string>& values, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter);
   template <class T> void ImageCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void MaterialCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void SurfaceCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void LightmapCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void RenderProbeCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void CollectionCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void SoundCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter);
   template <class T> void Font(T* obj, const std::function<FontDesc(const T*)>& getter, const std::function<void(T*, const FontDesc&)>& setter);

   int GetModifiedField() const { return m_modified; }

private:
   static constexpr const char* ICON_SAVE = ICON_FK_FLOPPY_O;

   bool IsStartup() const { return m_table->m_liveBaseTable && m_showStartup; }
   template <class T> T* GetStartupObj(T* obj) const;

   // Shared handling of fields that can be synchronized between a live table and its
   // startup version: BeginSyncField resolves the displayed instance and its counterpart
   // (null when there is none), EndSyncField draws the "copy to other version" button.
   template <class T> struct SyncField
   {
      T* display;
      T* other;
   };
   template <class T> SyncField<T> BeginSyncField(T* obj);
   template <class T, class Getter, class Setter, class V> void EndSyncField(const SyncField<T>& sync, const Getter& getter, const Setter& setter, const V& displayValue);

   void PropertyLabel(const string& label);

   int m_modified = 0;
   int m_modifyFieldId = 1;

   Unit m_lengthUnit = Unit::None;
   bool m_showStartup = false;
   PinTable* const m_table;
   bool m_inSection = false;
   bool m_sectionHasSync = false;
   float m_syncWidth = 0.f;
   ImVec2 m_syncPos;
};


// Inline definition of template functions

template <class T> T* PropertyPane::GetStartupObj(T* obj) const
{
   T* startupObj = nullptr;
   if constexpr (std::is_base_of_v<PinTable, T>)
      startupObj = m_table->m_liveBaseTable;
   else if constexpr (std::is_base_of_v<IEditable, T>)
      startupObj = static_cast<T*>(m_table->GetStartupFromLive<IEditable>(obj));
   else
      startupObj = m_table->GetStartupFromLive<T>(obj);
   return startupObj;
}

template <class T> T* PropertyPane::GetEditedPart(T* obj) const
{
   if (m_showStartup)
   {
      T* startupObj = GetStartupObj<T>(obj);
      if (startupObj)
         return startupObj;
   }
   return obj;
}

template <class T> PropertyPane::SyncField<T> PropertyPane::BeginSyncField(T* obj)
{
   T* const startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   if (startupObj == nullptr)
      return { obj, nullptr };
   return m_showStartup ? SyncField<T> { startupObj, obj } : SyncField<T> { obj, startupObj };
}

template <class T, class Getter, class Setter, class V> void PropertyPane::EndSyncField(const SyncField<T>& sync, const Getter& getter, const Setter& setter, const V& displayValue)
{
   if (sync.other == nullptr)
      return;
   ImGui::SetCursorScreenPos(m_syncPos);
   ImGui::BeginDisabled(displayValue == getter(sync.other));
   if (ImGui::Button(ICON_SAVE))
   {
      setter(sync.other, getter(sync.display));
      m_modified = m_modifyFieldId;
   }
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
   {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::Text("Copy this value to the %s version", m_showStartup ? "live" : "startup");
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
   ImGui::EndDisabled();
}

inline void PropertyPane::TimerSection(IEditable* obj)
{
   if (BeginSection("Timer"s))
   {
      Checkbox<IEditable>(
         obj, "Enable"s, //
         [](const IEditable* pObj) { return pObj->m_timerEnabled; }, //
         [](IEditable* pObj, bool v) { pObj->m_timerEnabled = v; });

      Combo<IEditable>(
         obj, "Timer mode"s, vector { "Fixed interval"s, "Per Frame"s, "Game Logic Sync"s }, //
         [](const IEditable* pObj)
         {
            int interval = pObj->m_timerInterval;
            return interval == -2 ? 2 : interval == -1 ? 1 : 0;
         }, //
         [](IEditable* pObj, int v)
         {
            if (v == 2)
               pObj->m_timerInterval = -2;
            else if (v == 1)
               pObj->m_timerInterval = -1;
            else
               pObj->m_timerInterval = 100;
         });
      if (GetEditedPart<IEditable>(obj)->m_timerInterval >= 0)
      {
         InputInt<IEditable>(
            obj, "Interval (ms)"s, //
            [](const IEditable* pObj) { return pObj->m_timerInterval; }, //
            [](IEditable* pObj, int v) { pObj->m_timerInterval = v; });
      }
      EndSection();
   }
}

template <class T> inline void PropertyPane::Checkbox(T* obj, const string& label, const std::function<bool(const T*)>& getter, const std::function<void(T*, bool)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   PropertyLabel(label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   bool value = getter(sync.display);
   if (ImGui::Checkbox(("##" + label).c_str(), &value))
   {
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T> inline void PropertyPane::InputInt(T* obj, const string& label, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   PropertyLabel(label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   int value = getter(sync.display);
   if (ImGui::InputInt(("##" + label).c_str(), &value))
   {
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T>
inline void PropertyPane::InputFloat(T* obj, const string& label, const std::function<float(const T*)>& getter, const std::function<void(T*, float)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ResolveUnit will set it to the supported converted display unit
   ResolveUnit(unit, displayUnit, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   float value = getter(sync.display);
   float displayValue = value;
   ConvertUnit(unit, displayUnit, displayValue, nDecimalAdjust);
   if (ImGui::InputFloat(("##" + label).c_str(), &displayValue, 0.f, 0.f, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
   {
      value = displayValue;
      ConvertUnit(displayUnit, unit, value, nDecimalAdjust);
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T>
inline void PropertyPane::InputFloat2(
   T* obj, const string& label, const std::function<Vertex2D(const T*)>& getter, const std::function<void(T*, const Vertex2D&)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ResolveUnit will set it to the supported converted display unit
   ResolveUnit(unit, displayUnit, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   Vertex2D value = getter(sync.display);
   Vertex2D displayValue = value;
   ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
   ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
   if (ImGui::InputFloat2(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
   {
      value = displayValue;
      ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
      ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T>
inline void PropertyPane::InputFloat3(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ResolveUnit will set it to the supported converted display unit
   ResolveUnit(unit, displayUnit, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   vec3 value = getter(sync.display);
   vec3 displayValue = value;
   ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
   ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
   ConvertUnit(unit, displayUnit, displayValue.z, nDecimalAdjust);
   if (ImGui::InputFloat3(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
   {
      value = displayValue;
      ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
      ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
      ConvertUnit(displayUnit, unit, value.z, nDecimalAdjust);
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T> inline void PropertyPane::InputRGB(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   PropertyLabel(label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   vec3 value = getter(sync.display);
   if (ImGui::ColorEdit3(("##" + label).c_str(), &value.x))
   {
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T> inline void PropertyPane::InputString(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   PropertyLabel(label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   string value = getter(sync.display);
   if (ImGui::InputText(("##" + label).c_str(), &value))
   {
      setter(sync.display, value);
      m_modified = m_modifyFieldId;
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T>
inline void PropertyPane::Combo(T* obj, const string& label, const std::vector<string>& values, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   PropertyLabel(label);
   const SyncField<T> sync = BeginSyncField(obj);
   ImGui::PushID(label.c_str());
   int value = getter(sync.display);
   const int displayIndex = values.empty() ? -1 : std::clamp(value, 0, static_cast<int>(values.size()) - 1);
   if (ImGui::BeginCombo(("##" + label).c_str(), displayIndex < 0 ? "" : values[displayIndex].c_str()))
   {
      for (size_t i = 0; i < values.size(); i++)
      {
         if (ImGui::Selectable((values[i] + "##Item" + std::to_string(i)).c_str()))
         {
            setter(sync.display, static_cast<int>(i));
            value = static_cast<int>(i);
            m_modified = m_modifyFieldId;
         }
      }
      ImGui::EndCombo();
   }
   EndSyncField(sync, getter, setter, value);
   ImGui::PopID();
}

template <class T> inline void PropertyPane::ImageCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> images(m_table->m_vimage.size());
   const std::function<string(Texture*)> map = [](const Texture* image) -> string { return image->m_name; };
   std::ranges::transform(m_table->m_vimage.begin(), m_table->m_vimage.end(), images.begin(), map);
   std::sort(images.begin(), images.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   images.insert(images.begin(), ""s);
   Combo<T>(obj, label, images, [&](const T* obj) { return max(0, FindIndexOf(images, getter(obj))); }, [&](T* obj, int v) { setter(obj, images[v]); });
}

template <class T> inline void PropertyPane::MaterialCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> materials(m_table->m_materials.size());
   const std::function<string(Material*)> map = [](const Material* material) { return material->m_name; };
   std::ranges::transform(m_table->m_materials.begin(), m_table->m_materials.end(), materials.begin(), map);
   std::sort(materials.begin(), materials.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   materials.insert(materials.begin(), ""s);
   Combo<T>(obj, label, materials, [&](const T* obj) { return max(0, FindIndexOf(materials, getter(obj))); }, [&](T* obj, int v) { setter(obj, materials[v]); });
}

template <class T> inline void PropertyPane::SurfaceCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> surfaces;
   for (const IEditable* pe : m_table->GetParts())
      if (pe->GetItemType() == ItemTypeEnum::eItemSurface || pe->GetItemType() == ItemTypeEnum::eItemRamp || pe->GetItemType() == ItemTypeEnum::eItemFlasher)
         surfaces.push_back(pe->GetName());
   std::sort(surfaces.begin(), surfaces.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   surfaces.insert(surfaces.begin(), ""s);
   Combo<T>(obj, label, surfaces, [&](const T* obj) { return max(0, FindIndexOf(surfaces, getter(obj))); }, [&](T* obj, int v) { setter(obj, surfaces[v]); });
}

template <class T> inline void PropertyPane::LightmapCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> lightmaps;
   for (const IEditable* pe : m_table->GetParts())
      if (pe->GetItemType() == ItemTypeEnum::eItemLight)
         lightmaps.push_back(pe->GetName());
   std::sort(lightmaps.begin(), lightmaps.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   lightmaps.insert(lightmaps.begin(), ""s);
   Combo<T>(obj, label, lightmaps, [&](const T* obj) { return max(0, FindIndexOf(lightmaps, getter(obj))); }, [&](T* obj, int v) { setter(obj, lightmaps[v]); });
}

template <class T>
inline void PropertyPane::RenderProbeCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> renderprobes;
   for (const RenderProbe* probe : m_table->m_vrenderprobe)
      renderprobes.push_back(probe->GetName());
   std::sort(renderprobes.begin(), renderprobes.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   renderprobes.insert(renderprobes.begin(), ""s);
   Combo<T>(obj, label, renderprobes, [&](const T* obj) { return max(0, FindIndexOf(renderprobes, getter(obj))); }, [&](T* obj, int v) { setter(obj, renderprobes[v]); });
}

template <class T> void PropertyPane::CollectionCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> collections;
   for (auto pcol : m_table->GetCollections())
      collections.push_back(MakeString(pcol->m_wzName));
   std::sort(collections.begin(), collections.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   collections.insert(collections.begin(), ""s);
   Combo<T>(obj, label, collections, [&](const T* obj) { return max(0, FindIndexOf(collections, getter(obj))); }, [&](T* obj, int v) { setter(obj, collections[v]); });
}

template <class T> void PropertyPane::SoundCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> sounds;
   for (const VPX::Sound* sound : m_table->m_vsound)
      sounds.push_back(sound->GetName());
   std::sort(sounds.begin(), sounds.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   sounds.insert(sounds.begin(), ""s);
   Combo<T>(obj, label, sounds, [&](const T* obj) { return max(0, FindIndexOf(sounds, getter(obj))); }, [&](T* obj, int v) { setter(obj, sounds[v]); });
}

template <class T> void PropertyPane::Font(T* obj, const std::function<FontDesc(const T*)>& getter, const std::function<void(T*, const FontDesc&)>& setter)
{
   InputString<T>(
      obj, "Font"s, //
      [getter](const T* o) { return getter(o).name; }, //
      [getter, setter](T* o, const string& v)
      {
         FontDesc f = getter(o);
         f.name = v;
         setter(o, f);
      });
   InputFloat<T>(
      obj, "Font Size"s, //
      [getter](const T* o) { return static_cast<float>(getter(o).size) / 10000.f; }, //
      [getter, setter](T* o, float v)
      {
         FontDesc f = getter(o);
         f.size = static_cast<uint32_t>(max(0.f, v) * 10000.f);
         setter(o, f);
      },
      Unit::None, 1);
   Checkbox<T>(
      obj, "Bold"s, //
      [getter](const T* o) { return getter(o).IsBold(); }, //
      [getter, setter](T* o, bool v)
      {
         FontDesc f = getter(o);
         f.weight = v ? 700 : 400;
         setter(o, f);
      });
   Checkbox<T>(
      obj, "Italic"s, //
      [getter](const T* o) { return getter(o).IsItalic(); }, //
      [getter, setter](T* o, bool v)
      {
         FontDesc f = getter(o);
         f.attributes = v ? f.attributes | 0x02 : f.attributes & ~0x02;
         setter(o, f);
      });
}
}
