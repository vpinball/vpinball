// DispReel.h: Definition of the DispReel class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_)
#define AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// DispReel

// add data in this class is persisted with the table
class DispReelData : public BaseProperty
{
public:
   Vertex2D    m_v1, m_v2;          // position on map (top right corner)
   int         m_imagesPerGridRow;
   int         m_reelcount;         // number of individual reel in the set
   float       m_width, m_height;   // size of each reel
   float       m_reelspacing;       // spacing between each reel and the boarders
   int         m_motorsteps;        // steps (or frames) to move each reel each frame
   int         m_digitrange;        // max number of digits per reel (usually 9)

   char        m_szSound[MAXTOKEN]; // sound to play for each turn of a digit
   int         m_updateinterval;    // time in ms between each animation update

   COLORREF    m_backcolor;         // colour of the background

   TimerDataRoot m_tdr;             // timer information
   bool        m_transparent;      // is the background transparent
   bool        m_useImageGrid;
};

class DispReel :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IDispReel, &IID_IDispReel, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   //public CComObjectRoot,
   public CComCoClass<DispReel, &CLSID_DispReel>,
   public EventProxy<DispReel, &DIID_IDispReelEvents>,
   public IConnectionPointContainerImpl<DispReel>,
   public IProvideClassInfo2Impl<&CLSID_DispReel, &DIID_IDispReelEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public IScriptable,
   public IFireEvents,
   public Hitable,
   public IPerPropertyBrowsing     // Ability to fill in dropdown(s) in property browser
{
public:
   DispReel();
   virtual ~DispReel();
   BEGIN_COM_MAP(DispReel)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IDispReel)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(DispReel)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(DispReel)
      CONNECTION_POINT_ENTRY(DIID_IDispReelEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(DispReel, eItemDispReel, DISPREEL, 2)

   virtual void GetDialogPanes(vector<PropertyPane*> &pvproppane);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);

   virtual ItemTypeEnum HitableGetItemType() const { return eItemDispReel; }

   virtual void WriteRegDefaults();

   DECLARE_REGISTRY_RESOURCEID(IDR_DISP_REEL)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   int     GetImagesPerGridRow() { return m_d.m_imagesPerGridRow; }
   void    SetImagesPerGridRow(int amount) { m_d.m_imagesPerGridRow = max(1, amount); }
   int     GetReels() { return m_d.m_reelcount; }
   void    SetReels(int reels)
   {
       m_d.m_reelcount = min(max(1, reels), MAX_REELS); // must have at least 1 reel and a max of MAX_REELS
       m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();
       m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();
   }
   int     GetRange() { return m_d.m_digitrange; }
   void    SetRange(int newRange)
   {
       m_d.m_digitrange = max(0, newRange);                        // must have at least 1 digit (0 is a digit)
       if (m_d.m_digitrange > 512 - 1) m_d.m_digitrange = 512 - 1; // and a max of 512 (0->511) //!! 512 requested by highrise
   }
   float   GetX() { return m_d.m_v1.x; }
   void    SetX(float x)
   {
       const float delta = x - m_d.m_v1.x;
       m_d.m_v1.x += delta;
       m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();
   }
   float   GetY() { return m_d.m_v1.y; }
   void    SetY(float y)
   {
       const float delta = y - m_d.m_v1.y;
       m_d.m_v1.y += delta;
       m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();
   }
   float   GetWidth() { return m_d.m_width; }
   void    SetWidth(float width)
   {
       m_d.m_width = max(0.0f, width);
       m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();

   }

   float   GetHeight() { return m_d.m_height; }
   void    SetHeight(float height)
   {
       m_d.m_height = max(0.0f, height);
       m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();
   }
   float   GetSpacing() { return m_d.m_reelspacing; }
   void    SetSpacing(float newSpace) 
   {
       m_d.m_reelspacing = max(0.0f, newSpace);
       m_d.m_v2.x = m_d.m_v1.x + getBoxWidth();
       m_d.m_v2.y = m_d.m_v1.y + getBoxHeight();
   }
   int     GetMotorSteps() { return m_d.m_motorsteps; }
   void    SetMotorSteps(int steps)
   {
       m_d.m_motorsteps = max(1, steps); // must have at least 1 step)
   }
   int     GetUpdateInterval() { return m_d.m_updateinterval; }
   void    SetUpdateInterval(int interval)
   {
       m_d.m_updateinterval = max((int)5, interval);
   }

   void    Animate();
   
   DispReelAnimObject m_dispreelanim;

   DispReelData m_d;

private:
   float   getBoxWidth() const;
   float   getBoxHeight() const;

   PinTable    *m_ptable;

   float       m_renderwidth, m_renderheight;     // size of each reel (rendered)

   struct ReelInfo
   {
      int     currentValue;       // current digit value
      int     motorPulses;        // number of motor pulses received for this reel (can be negative)
      int     motorStepCount;     // when equal to zero then at a whole letter
      float   motorCalcStep;      // calculated steping rate of motor
      float   motorOffset;        // frame value of motor (where to display the reel)
   };
   ReelInfo    m_reelInfo[MAX_REELS];

   float       m_reeldigitwidth;  // size of the individual reel digits (in bitmap form)
   float       m_reeldigitheight;
   U32         m_timeNextUpdate;

   struct TexCoordRect
   {
      float u_min, u_max;
      float v_min, v_max;
   };
   std::vector<TexCoordRect> m_digitTexCoords;

   // IDispReel
public:
   // properties
   STDMETHOD(get_IsTransparent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsTransparent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Reels)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Reels)(/*[in]*/ float newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Width)(/*[in]*/ float newVal);
   STDMETHOD(get_Spacing)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Spacing)(/*[in]*/ float newVal);
   STDMETHOD(get_BackColor)(/*[out, retval]*/ OLE_COLOR *pVal);
   STDMETHOD(put_BackColor)(/*[in]*/ OLE_COLOR newVal);
   STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Sound)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Sound)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Steps)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Steps)(/*[in]*/ float newVal);
   STDMETHOD(get_Range)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Range)(/*[in]*/ float newVal);
   STDMETHOD(get_UpdateInterval)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_UpdateInterval)(/*[in]*/ long newVal);
   STDMETHOD(get_UseImageGrid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_UseImageGrid)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ImagesPerGridRow)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_ImagesPerGridRow)(/*[in]*/ long newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   // methods
   STDMETHOD(ResetToZero)();
   STDMETHOD(AddValue)(/*[in]*/ long Value);
   STDMETHOD(SetValue)(/*[in]*/ long Value);
   STDMETHOD(SpinReel)(/*[in]*/ long ReelNumber, /*[in]*/ long PulseCount);
};

#endif // !defined(AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_)
