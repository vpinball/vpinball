#include "stdafx.h"
#ifndef __STANDALONE__
#include "vpinball.h"
#else
#include "vpinball_standalone.h"
#endif

BallEx::BallEx()
{
   m_pball = nullptr;
}

BallEx::~BallEx()
{
}

void BallEx::GetDebugCommands(vector<int> & pvids, vector<int> & pvcommandid)
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

   *pVal = m_pball->m_d.m_pos.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_X(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Y(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_pos.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_Y(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Z(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_pos.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_Z(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.x / m_pball->Inertia();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.y / m_pball->Inertia();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.z / m_pball->Inertia();

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
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_pball->m_image.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP BallEx::put_Image(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_pball->m_image = buf;

   m_pball->m_pinballEnv = g_pplayer->m_ptable->GetImage(m_pball->m_image);
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
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_pball->m_imageDecal.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP BallEx::put_FrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   Texture * const tex = g_pplayer->m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   m_pball->m_imageDecal = szImage;
   m_pball->m_pinballDecal = tex;

   return S_OK;
}

STDMETHODIMP BallEx::get_DecalMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pball->m_decalMode);

   return S_OK;
}

STDMETHODIMP BallEx::put_DecalMode(VARIANT_BOOL newVal)
{
   m_pball->m_decalMode = VBTOb(newVal);

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

   *pVal = m_pball->m_d.m_mass;

   return S_OK;
}

STDMETHODIMP BallEx::put_Mass(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_mass = newVal;

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

   *pVal = m_pball->m_d.m_radius;

   return S_OK;
}

STDMETHODIMP BallEx::put_Radius(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_radius = newVal;

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
   *pVal = FTOVB(m_pball->m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP BallEx::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_pball->m_reflectionEnabled = VBTOb(newVal);

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
    *pVal = FTOVB(m_pball->m_forceReflection);

    return S_OK;
}

STDMETHODIMP BallEx::put_ForceReflection(VARIANT_BOOL newVal)
{
    m_pball->m_forceReflection = VBTOb(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::get_Visible(VARIANT_BOOL *pVal)
{
    *pVal = FTOVB(m_pball->m_visible);

    return S_OK;
}

STDMETHODIMP BallEx::put_Visible(VARIANT_BOOL newVal)
{
    m_pball->m_visible = VBTOb(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::DestroyBall(int *pVal)
{
   int cnt = 0;
   if (g_pplayer)
   {
      ++cnt;
      Ball * const b = g_pplayer->m_pactiveball;
      g_pplayer->m_pactiveball = nullptr;
      g_pplayer->DestroyBall(b); // clear ActiveBall
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}
