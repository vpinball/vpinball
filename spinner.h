// Spinner.h: Definition of the Spinner class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_)
#define AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_

#include "resource.h"       // main symbols

class SpinnerData : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_length;
   float m_rotation;
   TimerDataRoot m_tdr;
   float m_height;
   float m_damping;
   float m_angleMax;
   float m_angleMin;
   char m_szSurface[MAXTOKEN];
   bool m_showBracket;
};

class HitSpinner;

/////////////////////////////////////////////////////////////////////////////
// Spinner

class Spinner :
   public IDispatchImpl<ISpinner, &IID_ISpinner, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Spinner, &CLSID_Spinner>,
   public EventProxy<Spinner, &DIID_ISpinnerEvents>,
   public IConnectionPointContainerImpl<Spinner>,
   public IProvideClassInfo2Impl<&CLSID_Spinner, &DIID_ISpinnerEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Spinner();
   virtual ~Spinner();
   BEGIN_COM_MAP(Spinner)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ISpinner)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Spinner)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Spinner)
      CONNECTION_POINT_ENTRY(DIID_ISpinnerEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Spinner, eItemSpinner, SPINNER, 1)

   DECLARE_REGISTRY_RESOURCEID(IDR_SPINNER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);
   virtual void SetDefaultPhysics(bool fromMouseClick);

   virtual unsigned long long GetMaterialID() const { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual unsigned long long GetImageID() const { return (unsigned long long)(m_ptable->GetImage(m_d.m_szImage)); }
   virtual ItemTypeEnum HitableGetItemType() const { return eItemSpinner; }
   virtual void ExportMesh(ObjLoader& loader);

   virtual void WriteRegDefaults();
   virtual void UpdateStatusBarInfo();

   float GetAngleMax() const;
   void  SetAngleMax(const float angle);
   float GetAngleMin() const;
   void  SetAngleMin(const float angle);

   SpinnerData m_d;

   HitSpinner *m_phitspinner;

private:
   void UpdatePlate(Vertex3D_NoTex2 * const vertBuffer);

   PinTable *m_ptable;

   VertexBuffer *m_bracketVertexBuffer;
   IndexBuffer *m_bracketIndexBuffer;
   VertexBuffer *m_plateVertexBuffer;
   IndexBuffer *m_plateIndexBuffer;
   Matrix3D m_fullMatrix;

   float m_posZ;

   float m_vertexBuffer_spinneranimangle;

   // ISpinner
public:
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Damping)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Damping)(/*[in]*/ float newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
   STDMETHOD(get_Length)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Length)(/*[in]*/ float newVal);

   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowBracket)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowBracket)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_AngleMax)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngleMax)(/*[in]*/ float newVal);
   STDMETHOD(get_AngleMin)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AngleMin)(/*[in]*/ float newVal);
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHODIMP get_CurrentAngle(float *pVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_)
