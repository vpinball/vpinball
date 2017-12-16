// Ball.h : Declaration of the CBall
#pragma once
#ifndef __BALL_H_
#define __BALL_H_

#include "resource.h"       // main symbols

class Ball;

/////////////////////////////////////////////////////////////////////////////
// CBall
class BallEx :
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<BallEx, &CLSID_Ball>,
   public IDispatchImpl<IBall, &IID_IBall, &LIBID_VPinballLib>,
   public IFireEvents,
   public IDebugCommands
{
public:
   BallEx();
   ~BallEx();

   DECLARE_REGISTRY_RESOURCEID(IDR_BALL)

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_COM_MAP(BallEx)
      COM_INTERFACE_ENTRY(IBall)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   // IBall
public:
   STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
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
   STDMETHOD(put_AngVelZ)(/*[in]*/ float newVal);
   STDMETHOD(get_AngVelY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngVelY)(/*[in]*/ float newVal);
   STDMETHOD(get_AngVelX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngVelX)(/*[in]*/ float newVal);
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
   STDMETHOD(get_UserValue)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_UserValue)(VARIANT *newVal);
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

   virtual void FireGroupEvent(int dispid) {}
   virtual IDispatch *GetDispatch() { return ((IDispatch *) this); }
   virtual IDebugCommands *GetDebugCommands() { return (IDebugCommands *) this; }

   // IDebugCommands
   virtual void GetDebugCommands(std::vector<int> & pvids, std::vector<int> & pvcommandid);
   virtual void RunDebugCommand(int id);

   Ball *m_pball;
   VARIANT m_uservalue;
};

#endif //__BALL_H_
