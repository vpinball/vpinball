// license:GPLv3+

#include "core/stdafx.h"


PartGroup::PartGroup()
{
}

PartGroup::~PartGroup()
{
}

PartGroup *PartGroup::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(PartGroup, live_table)
   return dst;
}

HRESULT PartGroup::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_v.x = x;
   m_d.m_v.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

STDMETHODIMP PartGroup::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IPartGroup,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}


#pragma region Editable

void PartGroup::SetDefaults(const bool fromMouseClick)
{
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsPartGroup, "TimerEnabled"s, true) : true;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsPartGroup, "TimerInterval"s, 100) : 100;
}

void PartGroup::WriteRegDefaults()
{
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsPartGroup, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsPartGroup, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
}

void PartGroup::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
}

void PartGroup::MoveOffset(const float dx, const float dy)
{
   m_d.m_v.x += dx;
   m_d.m_v.y += dy;
}

Vertex2D PartGroup::GetCenter() const
{
   return m_d.m_v;
}

void PartGroup::PutCenter(const Vertex2D& pv)
{
   m_d.m_v = pv;
}

void PartGroup::UIRenderPass1(Sur * const psur)
{
}

void PartGroup::UIRenderPass2(Sur * const psur)
{
}

void PartGroup::RenderBlueprint(Sur *psur, const bool solid)
{
}

#pragma endregion


#pragma region Physics

void PartGroup::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
}

void PartGroup::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
}

#pragma endregion


#pragma region Rendering

unsigned int PartGroup::GetVisibilityMask() const
{
   if (GetPartGroup() != nullptr)
      return m_d.m_visibilityMask & GetPartGroup()->GetVisibilityMask();
   return m_d.m_visibilityMask;
}

PartGroupData::SpaceReference PartGroup::GetReferenceSpace() const
{
   if (m_d.m_spaceReference != PartGroupData::SpaceReference::SR_INHERIT)
      return m_d.m_spaceReference;
   if (GetPartGroup() != nullptr)
      return GetPartGroup()->GetReferenceSpace();
   return PartGroupData::SpaceReference::SR_PLAYFIELD;
}

void PartGroup::RenderSetup(RenderDevice *device)
{
}

void PartGroup::UpdateAnimation(const float diff_time_msec)
{
}

void PartGroup::Render(const unsigned int renderMask)
{
}

void PartGroup::RenderRelease()
{
}

#pragma endregion


#pragma region Serialization

HRESULT PartGroup::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);
   // Default properties
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteVector2(FID(VCEN), m_d.m_v);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteBool(FID(BGLS), m_backglass);
   // PartGroup properties
   bw.WriteInt(FID(VMSK), static_cast<int>(m_d.m_visibilityMask));
   bw.WriteInt(FID(SPRF), static_cast<int>(m_d.m_spaceReference));
   ISelect::SaveData(pstm, hcrypthash);
   bw.WriteTag(FID(ENDB));
   return S_OK;
}

HRESULT PartGroup::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);
   m_ptable = ptable;
   br.Load();
   return S_OK;
}

bool PartGroup::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   // Default properties
   case FID(PIID): pbr->GetInt(static_cast<int*>(pbr->m_pdata)); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_v); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(BGLS): pbr->GetBool(m_backglass); break;
   // PartGroup properties
   case FID(VMSK): pbr->GetInt(&m_d.m_visibilityMask); break;
   case FID(SPRF): pbr->GetInt(&m_d.m_spaceReference); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT PartGroup::InitPostLoad()
{
   return S_OK;
}

#pragma endregion
