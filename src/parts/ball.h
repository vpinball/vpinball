// license:GPLv3+

#pragma once

#include "ui/resource.h"
#include "physics/hitable.h"
#include "physics/hitball.h"
#include "renderer/Renderable.h"
#include "parts/timer.h"

class HitBall;

// Helper class used for projecting sphere points, which is then used to compensate for projection stretch if anti-ball-stretch is enabled
class AntiStretchHelper final
{
public:
   // The number of points matters: 12 points are not enough, 35 and more seems to give good results
   static constexpr int npts = 35;
   float m_stretchFitPoints[3 * npts];

   AntiStretchHelper()
   {
      const double a = 4.0 * M_PI / npts, d = sqrt(a);
      const int nTheta = (int)round(M_PI / d);
      const double dTheta = M_PI / nTheta, dPhi = a / dTheta;
      for (int pos = 0, j = 0; j < nTheta; j++)
      {
         const double theta = ((double)j + 0.5) * M_PI / (double)nTheta;
         const int nPhi = (int)round(2.0 * M_PI * sin(theta) / dPhi);
         for (int i = 0; i < nPhi; i++)
         {
            const double phi = (double)i * (2.0 * M_PI) / (double)nPhi;
            m_stretchFitPoints[pos++] = (float)(sin(theta) * cos(phi));
            m_stretchFitPoints[pos++] = (float)(sin(theta) * sin(phi));
            m_stretchFitPoints[pos++] = (float)cos(theta);
         }
         // npts = pos / 3;
      }
   }

   bool computeProjBounds(const Matrix3D &mvp, const float x, const float y, const float z, const float radius, float& xMin, float& xMax, float& yMin, float& yMax) const
   {
      for (int i = 0; i < AntiStretchHelper::npts * 3; i += 3)
      {
         const float px = x + radius * m_stretchFitPoints[i];
         const float py = y + radius * m_stretchFitPoints[i + 1];
         const float pz = z + radius * m_stretchFitPoints[i + 2];
         float xp = mvp._11 * px + mvp._21 * py + mvp._31 * pz + mvp._41;
         float yp = mvp._12 * px + mvp._22 * py + mvp._32 * pz + mvp._42;
         const float wp = mvp._14 * px + mvp._24 * py + mvp._34 * pz + mvp._44;
         if (wp <= 1e-3f)
            return true;
         xp /= wp;
         yp /= wp;
         xMin = min(xMin, xp);
         xMax = max(xMax, xp);
         yMin = min(yMin, yp);
         yMax = max(yMax, yp);
      }
      return false;
   }
};

class BallData final : public BaseProperty
{
public:
   TimerDataRoot m_tdr;
   // Vertex3Ds m_pos; implemented in HitBall to avoid duplication
   // float m_radius; implemented in HitBall to avoid duplication
   // float m_mass; implemented in HitBall to avoid duplication
   bool m_forceReflection;

   bool m_useTableRenderSettings;
   bool m_decalMode;
   string m_imageDecal;
   float m_bulb_intensity_scale; // to dampen/increase contribution of the bulb lights (locally/by script)
   float m_playfieldReflectionStrength;
   COLORREF m_color;
   bool m_pinballEnvSphericalMapping;
};

class Ball :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IBall, &IID_IBall, &LIBID_VPinballLib>,
   //public CComObjectRoot,
   public CComCoClass<Ball, &CLSID_Ball>,
   public EventProxy<Ball, &DIID_IBallEvents>,
   public IConnectionPointContainerImpl<Ball>,
   public IProvideClassInfo2Impl<&CLSID_Ball, &DIID_IBallEvents, &LIBID_VPinballLib>,

   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS *const pdispparams) override;
#endif
   Ball();
   ~Ball();

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_COM_MAP(Ball)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IBall)

      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)

      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)

      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   STANDARD_EDITABLE_DECLARES(Ball, eItemBall, BALL, 1)

   BEGIN_CONNECTION_POINT_MAP(Ball)
      CONNECTION_POINT_ENTRY(DIID_IBallEvents)
   END_CONNECTION_POINT_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_BALL)

   // ISelect implementation
   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D &pv) final;

   // IEditable implementation
   void RenderBlueprint(Sur *psur, const bool solid) final;
   void WriteRegDefaults() final;

   // IHitable implementation
   ItemTypeEnum HitableGetItemType() const final { return eItemBall; }

   // IBall implementation
   STDMETHOD(get_FrontDecal)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_FrontDecal)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_DecalMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DecalMode)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_VelZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_VelZ)(/*[in]*/ float newVal);
   STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Z)(/*[in]*/ float newVal);
   STDMETHOD(get_VelY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_VelY)(/*[in]*/ float newVal);
   STDMETHOD(get_VelX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_VelX)(/*[in]*/ float newVal);
   STDMETHOD(get_AngVelZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngVelZ)(/*[out, retval]*/ float newVal) override { return S_OK; } // No-op to not break tables.
   STDMETHOD(get_AngVelY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngVelY)(/*[out, retval]*/ float newVal) override { return S_OK; } // No-op to not break tables.
   STDMETHOD(get_AngVelX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngVelX)(/*[out, retval]*/ float newVal) override { return S_OK; } // No-op to not break tables.
   STDMETHOD(get_AngMomZ)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngMomZ)(/*[in]*/ float newVal);
   STDMETHOD(get_AngMomY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngMomY)(/*[in]*/ float newVal);
   STDMETHOD(get_AngMomX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngMomX)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Radius)(/*[in]*/ float newVal);
   STDMETHOD(get_Mass)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Mass)(/*[in]*/ float newVal);
   STDMETHOD(get_ID)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_ID)(/*[in]*/ int newVal);
   STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_BulbIntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_BulbIntensityScale)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_PlayfieldReflectionScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_PlayfieldReflectionScale)(/*[in]*/ float newVal);
   STDMETHOD(get_ForceReflection)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ForceReflection)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);

   static void ResetBallIDCounter() { m_nextBallID = 0; }

   BallData m_d;
   HitBall m_hitBall;

   static const AntiStretchHelper m_ash;

private:
   static unsigned int m_nextBallID; // increased for each ball created to have an unique ID for scripts for each ball
   static unsigned int GetNextBallID();

   const unsigned int m_id; // unique ID for each ball
   PinTable *m_ptable = nullptr;
   RenderDevice *m_rd = nullptr;
   Texture *m_pinballEnv = nullptr;
   Texture *m_pinballDecal = nullptr;
   bool m_antiStretch = false;
};
