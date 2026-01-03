#pragma once

#include "fonts/IconsForkAwesome.h"
#include "parts/pintable.h"

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
   template <class T> void TimerSection(T* obj, const std::function<TimerDataRoot*(T*)>& getTimerData);

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
   void SetLengthUnit(Unit lengthUnit) { m_lengthUnit = lengthUnit; }

   void Header(const string& typeName, const std::function<string()>& getName, const std::function<void(const string&)>& setName);
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

   int GetModifiedField() const { return m_modified; }
   void ResetModified()
   {
      m_modified = -1;
      m_modifyFieldId = 0;
   }

private:
   const char* ICON_SAVE = ICON_FK_FLOPPY_O;

   bool IsStartup() const { return m_table->m_liveBaseTable && m_showStartup; }
   template <class T> T* GetStartupObj(T* obj) const;

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


// Inline deifnition of template functions

template <class T> T* PropertyPane::GetStartupObj(T* obj) const
{
   T* startupObj = nullptr;
   if constexpr (std::is_base_of_v<IEditable, T>)
      startupObj = static_cast<T*>(m_table->GetStartupFromLive<IEditable>(obj));
   else if constexpr (std::is_base_of_v<PinTable, T>)
      startupObj = m_table->m_liveBaseTable;
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

template <class T> void PropertyPane::TimerSection(T* obj, const std::function<TimerDataRoot*(T*)>& getTimerData)
{
   if (BeginSection("Timer"s))
   {
      Checkbox<T>(
         obj, "Enable"s, //
         [getTimerData](const T* pObj) { return getTimerData((T*)pObj)->m_TimerEnabled; }, //
         [getTimerData](T* pObj, bool v) { getTimerData(pObj)->m_TimerEnabled = v; });

      Combo<T>(
         obj, "Timer mode"s, vector { "Fixed interval"s, "Per Frame"s, "Game Logic Sync"s }, //
         [getTimerData](const T* pObj)
         {
            int interval = getTimerData((T*)pObj)->m_TimerInterval;
            return interval == -2 ? 2 : interval == -1 ? 1 : 0;
         }, //
         [getTimerData](T* pObj, int v)
         {
            if (v == 2)
               getTimerData(pObj)->m_TimerInterval = -2;
            else if (v == 1)
               getTimerData(pObj)->m_TimerInterval = -1;
            else
               getTimerData(pObj)->m_TimerInterval = 100;
         });
      if (getTimerData(GetEditedPart<T>(obj))->m_TimerInterval >= 0)
      {
         InputInt<T>(
            obj, "Interval (ms)"s, //
            [getTimerData](const T* pObj) { return getTimerData((T*)pObj)->m_TimerInterval; }, //
            [getTimerData](T* pObj, int v) { getTimerData(pObj)->m_TimerInterval = v; });
      }
      EndSection();
   }
}

template <class T> inline void PropertyPane::Checkbox(T* obj, const string& label, const std::function<bool(const T*)>& getter, const std::function<void(T*, bool)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   PropertyLabel(label);
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      bool value = getter(displayObj);
      if (ImGui::Checkbox(label.c_str(), &value))
      {
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      bool value = getter(obj);
      if (ImGui::Checkbox(("##" + label).c_str(), &value))
      {
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T> inline void PropertyPane::InputInt(T* obj, const string& label, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   PropertyLabel(label);
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      int value = getter(displayObj);
      if (ImGui::InputInt(label.c_str(), &value))
      {
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      int value = getter(obj);
      if (ImGui::InputInt(("##" + label).c_str(), &value))
      {
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T>
inline void PropertyPane::InputFloat(T* obj, const string& label, const std::function<float(const T*)>& getter, const std::function<void(T*, float)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   float displayValue = 0.f, value;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ConvertUnit will set it to the supported converted display unit
   ConvertUnit(unit, displayUnit, value, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      displayValue = value = getter(displayObj);
      ConvertUnit(unit, displayUnit, displayValue, nDecimalAdjust);
      if (ImGui::InputFloat(("##" + label).c_str(), &displayValue, 0.f, 0.f, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value, nDecimalAdjust);
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      displayValue = value = getter(obj);
      ConvertUnit(unit, displayUnit, displayValue, nDecimalAdjust);
      if (ImGui::InputFloat(("##" + label).c_str(), &displayValue, 0.f, 0.f, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value, nDecimalAdjust);
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T>
inline void PropertyPane::InputFloat2(
   T* obj, const string& label, const std::function<Vertex2D(const T*)>& getter, const std::function<void(T*, const Vertex2D&)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   Vertex2D displayValue, value;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ConvertUnit will set it to the supported converted display unit
   ConvertUnit(unit, displayUnit, value.x, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      displayValue = value = getter(displayObj);
      ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
      if (ImGui::InputFloat2(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      displayValue = value = getter(obj);
      ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
      if (ImGui::InputFloat2(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T>
inline void PropertyPane::InputFloat3(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter, Unit unit, int nDecimals)
{
   assert(m_inSection);
   m_modifyFieldId++;
   vec3 displayValue, value;
   int nDecimalAdjust;
   Unit displayUnit = m_lengthUnit; // ConvertUnit will set it to the supported converted display unit
   ConvertUnit(unit, displayUnit, value.x, nDecimalAdjust);
   string format = "%." + std::to_string(max(0, nDecimals + nDecimalAdjust)) + 'f';
   const char* unitLabel = GetUnitLabel(displayUnit);
   PropertyLabel(unitLabel ? (label + " (" + unitLabel + ')') : label);
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      displayValue = value = getter(displayObj);
      ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.z, nDecimalAdjust);
      if (ImGui::InputFloat3(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.z, nDecimalAdjust);
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      displayValue = value = getter(obj);
      ConvertUnit(unit, displayUnit, displayValue.x, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.y, nDecimalAdjust);
      ConvertUnit(unit, displayUnit, displayValue.z, nDecimalAdjust);
      if (ImGui::InputFloat3(("##" + label).c_str(), &displayValue.x, format.c_str(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
      {
         value = displayValue;
         ConvertUnit(displayUnit, unit, value.x, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.y, nDecimalAdjust);
         ConvertUnit(displayUnit, unit, value.z, nDecimalAdjust);
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T> inline void PropertyPane::InputRGB(T* obj, const string& label, const std::function<vec3(const T*)>& getter, const std::function<void(T*, const vec3&)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   PropertyLabel(label);
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      vec3 value = getter(displayObj);
      if (ImGui::ColorEdit3(("##" + label).c_str(), &value.x))
      {
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      vec3 value = getter(obj);
      if (ImGui::ColorEdit3(("##" + label).c_str(), &value.x))
      {
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T> inline void PropertyPane::InputString(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   vector<char> buffer(256);
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   PropertyLabel(label);
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      string value = getter(displayObj);
      memcpy(buffer.data(), value.c_str(), min(value.length(), buffer.size() - 1));
      if (ImGui::InputText(("##" + label).c_str(), buffer.data(), buffer.size()))
      {
         value = string(buffer.data());
         setter(displayObj, value);
         m_modified = m_modifyFieldId;
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      string value = getter(obj);
      memcpy(buffer.data(), value.c_str(), min(value.length(), buffer.size() - 1));
      if (ImGui::InputText(("##" + label).c_str(), buffer.data(), buffer.size()))
      {
         value = string(buffer.data());
         setter(obj, value);
         m_modified = m_modifyFieldId;
      }
   }
}

template <class T>
inline void PropertyPane::Combo(T* obj, const string& label, const std::vector<string>& values, const std::function<int(const T*)>& getter, const std::function<void(T*, int)>& setter)
{
   assert(m_inSection);
   m_modifyFieldId++;
   T* startupObj = m_sectionHasSync ? GetStartupObj<T>(obj) : nullptr;
   PropertyLabel(label);
   if (startupObj)
   {
      T* displayObj = m_showStartup ? startupObj : obj;
      T* otherObj = m_showStartup ? obj : startupObj;
      ImGui::PushID(label.c_str());
      int value = getter(displayObj);
      if (ImGui::BeginCombo(("##" + label).c_str(), values[value].c_str()))
      {
         for (size_t i = 0; i < values.size(); i++)
         {
            if (ImGui::Selectable((values[i] + "##Item" + std::to_string(i)).c_str()))
            {
               setter(displayObj, static_cast<int>(i));
               value = static_cast<int>(i);
               m_modified = m_modifyFieldId;
            }
         }
         ImGui::EndCombo();
      }
      ImGui::SetCursorScreenPos(m_syncPos);
      ImGui::BeginDisabled(value == getter(otherObj));
      if (ImGui::Button(ICON_SAVE))
      {
         setter(otherObj, getter(displayObj));
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
      ImGui::PopID();
   }
   else
   {
      string value = values[getter(obj)];
      if (ImGui::BeginCombo(("##" + label).c_str(), value.c_str()))
      {
         for (size_t i = 0; i < values.size(); i++)
         {
            if (ImGui::Selectable((values[i] + "##Item" + std::to_string(i)).c_str()))
            {
               setter(obj, static_cast<int>(i));
               m_modified = m_modifyFieldId;
            }
         }
         ImGui::EndCombo();
      }
   }
}

template <class T> inline void PropertyPane::ImageCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> images(m_table->m_vimage.size());
   const std::function<string(Texture*)> map = [](const Texture* image) -> string { return image->m_name; };
   std::transform(m_table->m_vimage.begin(), m_table->m_vimage.end(), images.begin(), map);
   std::sort(images.begin(), images.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   images.insert(images.begin(), ""s);
   Combo<T>(obj, label, images, [&](const T* obj) { return max(0, FindIndexOf(images, getter(obj))); }, [&](T* obj, int v) { setter(obj, images[v]); });
}

template <class T> inline void PropertyPane::MaterialCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> materials(m_table->m_materials.size());
   const std::function<string(Material*)> map = [](const Material* material) { return material->m_name; };
   std::transform(m_table->m_materials.begin(), m_table->m_materials.end(), materials.begin(), map);
   std::sort(materials.begin(), materials.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   materials.insert(materials.begin(), ""s);
   Combo<T>(obj, label, materials, [&](const T* obj) { return max(0, FindIndexOf(materials, getter(obj))); }, [&](T* obj, int v) { setter(obj, materials[v]); });
}

template <class T> inline void PropertyPane::SurfaceCombo(T* obj, const string& label, const std::function<string(const T*)>& getter, const std::function<void(T*, const string&)>& setter)
{
   std::vector<string> surfaces;
   for (const IEditable* pe : m_table->m_vedit)
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
   for (const IEditable* pe : m_table->m_vedit)
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
   for (int i = 0; i < m_table->m_vcollection.size(); i++)
      collections.push_back(MakeString(m_table->m_vcollection[i].m_wzName));
   std::sort(collections.begin(), collections.end(), [](const std::string& a, const std::string& b)
      { return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return tolower(c1) < tolower(c2); }); });
   collections.insert(collections.begin(), ""s);
   Combo<T>(obj, label, collections, [&](const T* obj) { return max(0, FindIndexOf(collections, getter(obj))); }, [&](T* obj, int v) { setter(obj, collections[v]); });
}

}
