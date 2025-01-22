// license:GPLv3+

// Definition of the Textbox class

#pragma once

#include "ui/resource.h"

#ifdef __STANDALONE__
#include <SDL3_ttf/SDL_ttf.h>
#endif

class TextboxData final
{
public:
   Vertex2D m_v1, m_v2;
   COLORREF m_backcolor;
   COLORREF m_fontcolor;
   float m_intensity_scale;
   string m_sztext;
   TimerDataRoot m_tdr;
   TextAlignment m_talign;
   bool m_transparent;
   bool m_visible;
   bool m_isDMD;
};

class Textbox :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<ITextbox, &IID_ITextbox, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   //public CComObjectRoot,
   public CComCoClass<Textbox, &CLSID_Textbox>,
   public EventProxy<Textbox, &DIID_ITextboxEvents>,
   public IConnectionPointContainerImpl<Textbox>,
   public IProvideClassInfo2Impl<&CLSID_Textbox, &DIID_ITextboxEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public IScriptable,
   public IFireEvents,
   public Hitable
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
   Textbox();
   virtual ~Textbox();

   BEGIN_COM_MAP(Textbox)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ITextbox)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Textbox) 
   // Remove the comment from the line above if you don't want your object to 
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Textbox)
      CONNECTION_POINT_ENTRY(DIID_ITextboxEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Textbox, eItemTextbox, TEXTBOX, 2)

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   // Multi-object manipulation
   Vertex2D GetCenter() const final { return m_d.m_v1; }
   void PutCenter(const Vertex2D& pv) final;
   ItemTypeEnum HitableGetItemType() const final { return eItemTextbox; }

   void WriteRegDefaults() final;

   DECLARE_REGISTRY_RESOURCEID(IDR_TEXTBOX)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   char *GetFontName();
   HFONT GetFont();

   IFont *m_pIFont = nullptr;
#ifdef __STANDALONE__
   bool m_fontItalic;
   bool m_fontUnderline;
   bool m_fontStrikeThrough;
   bool m_fontBold;
   float m_fontSize;
   string m_szFontName;
#endif

   TextboxData m_d;

private:
   PinTable *m_ptable = nullptr;
   
   RenderDevice *m_rd = nullptr;
   bool m_textureDirty = true;
   BaseTexture *m_texture = nullptr;
   IFont *m_pIFontPlay = nullptr; // Our font, scaled to match play window resolution

#ifdef __STANDALONE__
   TTF_Font* LoadFont();
#endif

public:
   // ITextbox
   STDMETHOD(get_IsTransparent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsTransparent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DMD)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Alignment)(/*[out, retval]*/ TextAlignment *pVal);
   STDMETHOD(put_Alignment)(/*[in]*/ TextAlignment newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_IntensityScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_IntensityScale)(/*[in]*/ float newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Width)(/*[in]*/ float newVal);
   STDMETHOD(get_Font)(/*[out, retval]*/ IFontDisp **pVal);
   STDMETHOD(put_Font)(/*[in]*/ IFontDisp *newVal);
   STDMETHOD(putref_Font)(IFontDisp* pFont);
   STDMETHOD(get_Text)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Text)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_FontColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_FontColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_BackColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_BackColor)(/*[in]*/ OLE_COLOR newVal);
};
