// license:GPLv3+

#include "core/stdafx.h"
#include "timer.h"

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
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
#undef LinkProp
}

void Timer::WriteRegDefaults()
{
#define LinkProp(field, prop) g_app->m_settings.SetDefaultPropsTimer_##prop(field, false)
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
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

void Timer::UIRenderPass1(Sur * const psur)
{
}

void Timer::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);//RGB(192,192,192));
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(this);

   psur->Ellipse(m_d.m_v.x, m_d.m_v.y, 18);

   psur->Ellipse(m_d.m_v.x, m_d.m_v.y, 15);

   for (int i = 0; i < 12; i++)
   {
      const float angle = (float)(M_PI*2.0 / 12.0)*(float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Line(m_d.m_v.x + sn*9.0f, m_d.m_v.y - cs*9.0f, m_d.m_v.x + sn * 15.0f, m_d.m_v.y - cs*15.0f);
   }

   //angle = ((PI*2)/24) * 3;
   psur->Line(m_d.m_v.x, m_d.m_v.y, m_d.m_v.x + 10.5f, m_d.m_v.y - 7.5f);

}

void Timer::RenderBlueprint(Sur *psur, const bool solid)
{
}


#pragma region Physics

void Timer::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (isUI)
   {
      // FIXME implement UI picking
   }
}

void Timer::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
}

#pragma endregion


#pragma region Rendering

void Timer::RenderSetup(RenderDevice *device)
{
}

void Timer::UpdateAnimation(const float diff_time_msec)
{
}

void Timer::Render(const unsigned int renderMask)
{
}

void Timer::RenderRelease()
{
}

#pragma endregion


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
   writer.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   writer.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
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
         case FID(TMON): m_d.m_tdr.m_TimerEnabled = reader.AsBool(); break;
         case FID(TMIN): m_d.m_tdr.m_TimerInterval = reader.AsInt(); break;
         case FID(NAME): m_wzName = reader.AsWideString(); break;
         case FID(BGLS): m_desktopBackdrop = reader.AsBool(); break;
         default: LoadSharedEditableField(tag, reader); break;
         }
         return true;
      });
}
