#include "stdafx.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

Timer *Timer::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Timer, live_table)
   return dst;
}

HRESULT Timer::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_v.x = x;
   m_d.m_v.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr); //ApcProjectItem.Define(ptable->ApcProject, GetDispatch(),
   //axTypeHostProjectItem/*axTypeHostClass*/, L"Timer", nullptr);
}

void Timer::SetDefaults(const bool fromMouseClick)
{
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsTimer, "TimerEnabled"s, true) : true;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsTimer, "TimerInterval"s, 100) : 100;
}

void Timer::WriteRegDefaults()
{
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsTimer, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsTimer, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
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

void Timer::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(m_phittimer);
}

#pragma region Physics

void Timer::GetHitShapes(vector<HitObject*> &pvho)
{
   m_phittimer = nullptr;
}

void Timer::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Timer::EndPlay()
{
   IEditable::EndPlay();
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

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Timer::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_tdr.m_TimerEnabled);

   return S_OK;
}

STDMETHODIMP Timer::put_Enabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   const bool val = VBTOb(newVal);

   if (val != m_d.m_tdr.m_TimerEnabled && m_phittimer)
   {
       // to avoid problems with timers dis/enabling themselves, store all the changes in a list
       bool found = false;
       for (size_t i = 0; i < g_pplayer->m_changed_vht.size(); ++i)
           if (g_pplayer->m_changed_vht[i].m_timer == m_phittimer)
           {
               g_pplayer->m_changed_vht[i].m_enabled = val;
               found = true;
               break;
           }

       if (!found)
       {
         TimerOnOff too;
         too.m_enabled = val;
         too.m_timer = m_phittimer;
         g_pplayer->m_changed_vht.push_back(too);
       }

       if (val)
           m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
       else
           m_phittimer->m_nextfire = 0xFFFFFFFF; // fakes the disabling of the timer, until it will be catched by the cleanup via m_changed_vht
   }

   m_d.m_tdr.m_TimerEnabled = val;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Timer::get_Interval(long *pVal)
{
   *pVal = m_d.m_tdr.m_TimerInterval;

   return S_OK;
}

STDMETHODIMP Timer::put_Interval(long newVal)
{
   STARTUNDO

   m_d.m_tdr.m_TimerInterval = newVal;

   if (m_phittimer)
   {
      m_phittimer->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : max(-2l, newVal);
      m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
   }

   STOPUNDO

   return S_OK;
}

HRESULT Timer::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_v);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), m_wzName);

   bw.WriteBool(FID(BGLS), m_backglass);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Timer::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Timer::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_v); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(BGLS): pbr->GetBool(m_backglass); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Timer::InitPostLoad()
{
   return S_OK;
}
