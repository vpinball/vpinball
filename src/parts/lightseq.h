// LightSeq.h: Definition of the LightSeq class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_)
#define AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// LightSeq

class LightSeqData final
{
public:
   Vertex2D      m_v;
   Vertex2D      m_vCenter;
   std::basic_string<WCHAR> m_wzCollection;
   int           m_updateinterval;
   TimerDataRoot m_tdr;
};

struct LightSeqQueueData {
   SequencerState Animation;
   int            TailLength;
   int            Repeat;
   int            Pause;
   int            UpdateRate;
};

struct LightSeqQueue {
   int               Head;
   int               Tail;
   LightSeqQueueData Data[LIGHTSEQQUEUESIZE];
};

enum {
   eSeqNull,
   eSeqBlink,
   eSeqRandom,
   eSeqLine,
   eSeqCircle,
   eSeqRadar
};

struct _tracer {
   int   type;         // type of tracer processing required
   int   delay;
   int   length;
   int   frameCount;
   float x;            // current grid coordinates
   float stepX;        // step to next grid coordinate for the next frame
   float processStepX; // step to next grid coordinate for processing this frame
   float y;
   float stepY;
   float processStepY;
   float radius;
   float stepRadius;
   bool  processRadiusNeg;
   float angle;
   float stepAngle;
};

class LightSeq :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<ILightSeq, &IID_ILightSeq, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   //public CComObjectRoot,
   public CComCoClass<LightSeq, &CLSID_LightSeq>,
   public IConnectionPointContainerImpl<LightSeq>,
   public IProvideClassInfo2Impl<&CLSID_LightSeq, &DIID_ILightSeqEvents, &LIBID_VPinballLib>,
   public EventProxy<LightSeq, &DIID_ILightSeqEvents>,
   public ISelect,
   public IEditable,
   public IScriptable,
   public IFireEvents,
   public Hitable,
   public IPerPropertyBrowsing     // Ability to fill in dropdown(s) in property browser
   //public EditableImpl<LightSeq>
{
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
public:
   LightSeq();
   ~LightSeq();

   //HRESULT Init(PinTable * const ptable, const float x, const float y);

   BEGIN_COM_MAP(LightSeq)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ILightSeq)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(LightSeq)
      CONNECTION_POINT_ENTRY(DIID_ILightSeqEvents)
   END_CONNECTION_POINT_MAP()

   void RenderOutline(Sur * const psur);

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   // Multi-object manipulation
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D& pv) final;

   void RenderBlueprint(Sur *psur, const bool solid) final;
   ItemTypeEnum HitableGetItemType() const final { return eItemLightSeq; }

   void WriteRegDefaults() final;

   STANDARD_EDITABLE_DECLARES(LightSeq, eItemLightSeq, LIGHTSEQ, 3)

   //DECLARE_NOT_AGGREGATABLE(LightSeq)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   DECLARE_REGISTRY_RESOURCEID(IDR_LIGHT_SEQ)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

private:
   U32            m_timeNextUpdate;
   int            m_blinkSequence;
   float          m_GridXCenter;
   float          m_GridYCenter;
   int            m_lightSeqGridHeight;
   int            m_lightSeqGridWidth;
   int            m_GridXCenterAdjust;
   int            m_GridYCenterAdjust;
   _tracer        m_th1, m_th2, m_tt1, m_tt2;
   Collection     *m_pcollection;
   short          *m_pgridData;
   LightSeqQueue  m_queue;
   SequencerState m_playAnimation;
   int            m_tailLength;
   int            m_pauseValue;
   int            m_replayCount;
   int            m_updateRate;
   bool           m_playInProgress;
   bool           m_pauseInProgress;

   // ILightSeq
public:
   STDMETHOD(get_Collection)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Collection)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_CenterX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_CenterX)(/*[in]*/ float newVal);
   STDMETHOD(get_CenterY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_CenterY)(/*[in]*/ float newVal);
   STDMETHOD(get_UpdateInterval)(/*[out, retval]*/ long *pVal);
   STDMETHOD(put_UpdateInterval)(/*[in]*/ long newVal);
   // methods
   STDMETHOD(Play)(/*[in]*/ SequencerState Animation, /*[in]*/ long TailLength, /*[in]*/ long Repeat, /*[in]*/ long Pause);
   STDMETHOD(StopPlay)();

   float    GetX() const { return m_d.m_vCenter.x; }
   void     SetX(const float value)
   {
       if ((value < 0.f) || (value >= (float)EDITOR_BG_WIDTH))
           return;

       m_d.m_vCenter.x = value;
       // set the center point of the grid for effects which start from the center
       m_GridXCenter = floorf(m_d.m_vCenter.x * (float)(1.0 / LIGHTSEQGRIDSCALE));
       m_GridXCenterAdjust = abs(m_lightSeqGridWidth / 2 - (int)m_GridXCenter);
   }
   float    GetY() const { return m_d.m_vCenter.y; }
   void     SetY(const float value)
   {
       if ((value < 0.f) || (value >= (float)(2 * EDITOR_BG_WIDTH)))
           return;

       m_d.m_vCenter.y = value;
       // set the center point of the grid for effects which start from the center
       m_GridYCenter = floorf(m_d.m_vCenter.y * (float)(1.0 / LIGHTSEQGRIDSCALE));
       m_GridYCenterAdjust = abs(m_lightSeqGridHeight / 2 - (int)m_GridYCenter);
   }

   int      GetUpdateInterval() const { return m_d.m_updateinterval; }
   void     SetUpdateInterval(const int value) { m_d.m_updateinterval = max(1, value); }

   LightSeq *CopyForPlay(PinTable *live_table);

   LightSeqData m_d;

private:
   PinTable *m_ptable;

   void     SetupTracers(const SequencerState Animation, int TailLength, int Repeat, int Pause);
   bool     ProcessTracer(_tracer * const pTracer, const LightState State);
   void     SetAllLightsToState(const LightState State);
   void     SetElementToState(const int index, const LightState State);
   LightState GetElementState(const int index) const;
   bool     VerifyAndSetGridElement(const int x, const int y, const LightState State);
};

#endif // !defined(AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_)
