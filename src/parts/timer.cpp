// license:GPLv3+

#include "core/stdafx.h"
#include "timer.h"

#include "core/VPApp.h"
#include "parts/Collection.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"


Timer::~Timer()
{
}

Timer *Timer::CopyForPlay() const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Timer)
   return dst;
}

HRESULT Timer::Init(const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   SetDefaults(fromMouseClick);
   m_d.m_v.x = x;
   m_d.m_v.y = y;
   return S_OK;
}

void Timer::SetDefaults(const bool fromMouseClick)
{
#define LinkProp(field, prop) field = fromMouseClick ? g_app->m_settings.GetDefaultPropsTimer_##prop() : Settings::GetDefaultPropsTimer_##prop##_Default()
   LinkProp(m_timerEnabled, TimerEnabled);
   LinkProp(m_timerInterval, TimerInterval);
#undef LinkProp
}

void Timer::WriteRegDefaults()
{
#define LinkProp(field, prop) g_app->m_settings.SetDefaultPropsTimer_##prop(field, false)
   LinkProp(m_timerEnabled, TimerEnabled);
   LinkProp(m_timerInterval, TimerInterval);
#undef LinkProp
}

void Timer::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
}

void Timer::MoveOffset(const float dx, const float dy)
{
   m_d.m_v.x += dx;
   m_d.m_v.y += dy;
}

Vertex2D Timer::GetCenter() const
{
   return m_d.m_v;
}

void Timer::PutCenter(const Vertex2D& pv)
{
   m_d.m_v = pv;
}

STDMETHODIMP Timer::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ITimer,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

void Timer::Save(IObjectWriter& writer, const bool saveForUndo)
{
   writer.WriteVector2(FID(VCEN), m_d.m_v);
   writer.WriteBool(FID(TMON), m_timerEnabled);
   writer.WriteInt(FID(TMIN), m_timerInterval);
   writer.WriteWideString(FID(NAME), m_wzName);
   writer.WriteBool(FID(BGLS), m_desktopBackdrop);
   SaveSharedEditableFields(writer);
   writer.EndObject();
}

void Timer::Load(IObjectReader& reader)
{
   SetDefaults(false);
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         switch (tag)
         {
         case FID(PIID): reader.AsInt(); break;
         case FID(VCEN): m_d.m_v = reader.AsVector2(); break;
         case FID(TMON): m_timerEnabled = reader.AsBool(); break;
         case FID(TMIN): m_timerInterval = reader.AsInt(); break;
         case FID(NAME): m_wzName = reader.AsWideString(); break;
         case FID(BGLS): m_desktopBackdrop = reader.AsBool(); break;
         default: LoadSharedEditableField(tag, reader); break;
         }
         return true;
      });
}
