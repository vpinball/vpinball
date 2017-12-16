#include "stdafx.h"
#include "vpinball_i.h"

BallEx::BallEx()
{
   m_pball = NULL;
}

BallEx::~BallEx()
{
}

void BallEx::GetDebugCommands(std::vector<int> & pvids, std::vector<int> & pvcommandid)
{
   pvids.push_back(IDS_MAKEACTIVEBALL);
   pvcommandid.push_back(0);
}

void BallEx::RunDebugCommand(int id)
{
   switch (id)
   {
   case 0:
      g_pplayer->m_pactiveballDebug = m_pball;
      break;
   }
}

#define CHECKSTALEBALL if (!m_pball) return E_POINTER;

STDMETHODIMP BallEx::get_X(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_pos.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_X(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_pos.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Y(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_pos.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_Y(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_pos.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_vel.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_vel.x = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_VelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_vel.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_vel.y = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_Z(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_pos.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_Z(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_pos.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_vel.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_vel.z = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularvelocity.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngVelX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularvelocity.x = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularvelocity.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngVelY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularvelocity.y = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularvelocity.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngVelZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularvelocity.z = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.x = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.y = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.z = newVal;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_Color(OLE_COLOR *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_color;

   return S_OK;
}

STDMETHODIMP BallEx::put_Color(OLE_COLOR newVal)
{
   CHECKSTALEBALL

   m_pball->m_color = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_pball->m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP BallEx::put_Image(BSTR newVal)
{
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_pball->m_szImage, 32, NULL, NULL);

   m_pball->m_pinballEnv = g_pplayer->m_ptable->GetImage(m_pball->m_szImage);
   m_pball->RenderSetup(); // nowadays does not do anything, but call it in case this changes in the future

   return S_OK;
}

HRESULT BallEx::get_UserValue(VARIANT *pVal)
{
   VariantClear(pVal);
   return VariantCopy(pVal, &m_uservalue);
}

HRESULT BallEx::put_UserValue(VARIANT *newVal)
{
   VariantInit(&m_uservalue);
   VariantClear(&m_uservalue);
   return VariantCopy(&m_uservalue, newVal);
}

STDMETHODIMP BallEx::get_FrontDecal(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_pball->m_szImageFront, -1, wz, 32);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP BallEx::put_FrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   Texture * const tex = g_pplayer->m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   strcpy_s(m_pball->m_szImageFront, szImage);

   m_pball->m_pinballDecal = tex;

   return S_OK;
}

STDMETHODIMP BallEx::get_DecalMode(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_pball->m_decalMode);

   return S_OK;
}

STDMETHODIMP BallEx::put_DecalMode(VARIANT_BOOL newVal)
{
   m_pball->m_decalMode = VBTOF(newVal);

   return S_OK;
}

STDMETHODIMP BallEx::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Ball");
   return S_OK;
}

STDMETHODIMP BallEx::put_Name(BSTR newVal)
{
   return S_OK;
}

STDMETHODIMP BallEx::get_Mass(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_mass;

   return S_OK;
}

STDMETHODIMP BallEx::put_Mass(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_mass = newVal;
   m_pball->m_invMass = 1.0f / newVal;

   m_pball->m_inertia = (float)(2.0 / 5.0) * m_pball->m_radius*m_pball->m_radius * m_pball->m_mass;

   return S_OK;
}

STDMETHODIMP BallEx::get_ID(int *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_id;

   return S_OK;
}

STDMETHODIMP BallEx::put_ID(int newVal)
{
   CHECKSTALEBALL

   m_pball->m_id = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Radius(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_radius;

   return S_OK;
}

STDMETHODIMP BallEx::put_Radius(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_radius = newVal;
   m_pball->m_inertia = (float)(2.0 / 5.0) * m_pball->m_radius*m_pball->m_radius * m_pball->m_mass;

   m_pball->CalcHitRect();

   return S_OK;
}

STDMETHODIMP BallEx::get_BulbIntensityScale(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_bulb_intensity_scale;

   return S_OK;
}

STDMETHODIMP BallEx::put_BulbIntensityScale(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_bulb_intensity_scale = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_pball->m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP BallEx::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_pball->m_reflectionEnabled = VBTOF(newVal);

   return S_OK;
}

STDMETHODIMP BallEx::get_PlayfieldReflectionScale(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_playfieldReflectionStrength;

   return S_OK;
}

STDMETHODIMP BallEx::put_PlayfieldReflectionScale(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_playfieldReflectionStrength = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_ForceReflection(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_pball->m_forceReflection);

    return S_OK;
}

STDMETHODIMP BallEx::put_ForceReflection(VARIANT_BOOL newVal)
{
    m_pball->m_forceReflection = VBTOF(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::get_Visible(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_pball->m_visible);

    return S_OK;
}

STDMETHODIMP BallEx::put_Visible(VARIANT_BOOL newVal)
{
    m_pball->m_visible = VBTOF(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::DestroyBall(int *pVal)
{
   int cnt = 0;
   if (g_pplayer)
   {
      ++cnt;
      Ball * b = g_pplayer->m_pactiveball;
      g_pplayer->m_pactiveball = NULL;
      g_pplayer->DestroyBall(b); // clear ActiveBall
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}
