// license:GPLv3+

#include "core/stdafx.h"


PartGroup *PartGroup::CopyForPlay() const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(PartGroup)
   return dst;
}

HRESULT PartGroup::Init(const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   SetDefaults(fromMouseClick);
   m_d.m_v.x = x;
   m_d.m_v.y = y;
   return S_OK;
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
#define LinkProp(field, prop) field = fromMouseClick ? g_app->m_settings.GetDefaultPropsPartGroup_##prop() : Settings::GetDefaultPropsPartGroup_##prop##_Default()
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
#undef LinkProp
}

void PartGroup::WriteRegDefaults()
{
#define LinkProp(field, prop) g_app->m_settings.SetDefaultPropsPartGroup_##prop(field, false)
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
#undef LinkProp
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

unsigned int PartGroup::GetPlayerModeVisibilityMask() const
{
   if (GetPartGroup() != nullptr)
      return m_d.m_playerModeVisibilityMask & GetPartGroup()->GetPlayerModeVisibilityMask();
   return m_d.m_playerModeVisibilityMask;
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
   bw.WriteInt(FID(PMSK), static_cast<int>(m_d.m_playerModeVisibilityMask));
   bw.WriteInt(FID(SPRF), static_cast<int>(m_d.m_spaceReference));
   ISelect::SaveData(pstm, hcrypthash);
   bw.WriteTag(FID(ENDB));
   return S_OK;
}

HRESULT PartGroup::Load(IObjectReader& reader)
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
         case FID(BGLS): m_backglass = reader.AsBool(); break;
         case FID(PMSK): m_d.m_playerModeVisibilityMask = reader.AsUInt(); break;
         case FID(SPRF): m_d.m_spaceReference = static_cast<PartGroupData::SpaceReference>(reader.AsInt()); break;
         default: ISelect::LoadToken(tag, reader); break;
         }
         return true;
      });
   return S_OK;
}

#pragma endregion
