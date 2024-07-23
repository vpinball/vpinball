// Decal.h: interface for the Decal class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_)
#define AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_

class DecalData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_width, m_height;
   float m_rotation;
   string m_szSurface;
   DecalType m_decaltype;
   string m_sztext;
   SizingType m_sizingtype;
   COLORREF m_color;
   bool m_verticalText;
};

class Decal :
   public IDispatchImpl<IDecal, &IID_IDecal, &LIBID_VPinballLib>,
   public CComObjectRoot,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
public:
   Decal();
   virtual ~Decal();

   BEGIN_COM_MAP(Decal)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IDecal)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
   END_COM_MAP()

   STANDARD_NOSCRIPT_EDITABLE_DECLARES(Decal, eItemDecal, DECAL, VIEW_PLAYFIELD | VIEW_BACKGLASS)

   void MoveOffset(const float dx, const float dy) final { m_d.m_vCenter.x += dx; m_d.m_vCenter.y += dy; }
   void SetObjectPos() final;
   // Multi-object manipulation
   Vertex2D GetCenter() const final { return m_d.m_vCenter; }
   void PutCenter(const Vertex2D& pv) final { m_d.m_vCenter = pv; }
   float GetDepth(const Vertex3Ds &viewDir) const final;
   void Rotate(const float ang, const Vertex2D &pvCenter, const bool useElementCenter) final;

   STDMETHOD(get_Name)(BSTR *pVal) final { return E_FAIL; }
   char *GetFontName();
   HFONT GetFont();

   void WriteRegDefaults() final;

   ItemTypeEnum HitableGetItemType() const final { return eItemDecal; }

   void EnsureSize();

   Decal *CopyForPlay(PinTable *live_table);

   DecalData m_d;
   IFont *m_pIFont = nullptr;

private:
   void GetTextSize(int * const px, int * const py);

   PinTable *m_ptable = nullptr;

   RenderDevice *m_rd = nullptr;
   MeshBuffer *m_meshBuffer = nullptr;
   BaseTexture *m_textImg = nullptr;
   float m_leading = 0.0f, m_descent = 0.0f;
   float m_realwidth = 0.0f, m_realheight = 0.0f;

   Vertex3Ds m_boundingSphereCenter;
   void UpdateBounds();

   // IDecal
public:
   STDMETHOD(get_HasVerticalText)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasVerticalText)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Font)(/*[out, retval]*/ IFontDisp **pVal);
   STDMETHOD(putref_Font)(/*[in]*/ IFontDisp *newVal);
   STDMETHOD(get_FontColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_FontColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_SizingType)(/*[out, retval]*/ SizingType *pVal);
   STDMETHOD(put_SizingType)(/*[in]*/ SizingType newVal);
   STDMETHOD(get_Text)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Text)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Type)(/*[out, retval]*/ DecalType *pVal);
   STDMETHOD(put_Type)(/*[in]*/ DecalType newVal);
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Width)(/*[in]*/ float newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
};

#endif // !defined(AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_)
