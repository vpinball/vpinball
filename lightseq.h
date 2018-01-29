// LightSeq.h: Definition of the LightSeq class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_)
#define AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// LightSeq

class LightSeqData
{
public:
   Vertex2D 		m_v;
   Vertex2D		m_vCenter;
   WCHAR 			m_wzCollection[MAXNAMEBUFFER];	// collection to use
   long			m_updateinterval;
   TimerDataRoot 	m_tdr;
};

typedef struct {
   SequencerState 	Animation;
   long 			TailLength;
   long 			Repeat;
   long 			Pause;
   long			UpdateRate;
} _LightSeqQueueData;

typedef struct {
   int					Head;
   int					Tail;
   _LightSeqQueueData	Data[LIGHTSEQQUEUESIZE];
} _LightSeqQueue;

enum {
   eSeqNull,
   eSeqBlink,
   eSeqRandom,
   eSeqLine,
   eSeqCircle,
   eSeqRadar
};

typedef struct {
   int		type;				// type of tracer processing required
   int		delay;
   int		length;
   int		frameCount;
   float	x;					// current grid coordinates
   float	stepX;				// step to next grid coordinate for the next frame
   float	processStepX;		// step to next grid coordinate for processing this frame
   float	y;
   float	stepY;
   float	processStepY;
   float	radius;
   float	stepRadius;
   float	processRadius;
   float	angle;
   float	stepAngle;
} _tracer;

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
public:
   LightSeq();
   ~LightSeq();

   //HRESULT Init(PinTable *ptable, float x, float y);

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

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   void RenderOutline(Sur * const psur);
   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   virtual void RenderBlueprint(Sur *psur, const bool solid=false);
   virtual ItemTypeEnum HitableGetItemType() { return eItemLightSeq; }

   void WriteRegDefaults();

   STANDARD_EDITABLE_DECLARES(LightSeq, eItemLightSeq, LIGHTSEQ, 3)

      //DECLARE_NOT_AGGREGATABLE(LightSeq)
      // Remove the comment from the line above if you don't want your object to
      // support aggregation.

      DECLARE_REGISTRY_RESOURCEID(IDR_LIGHT_SEQ)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

private:
   U32				m_timeNextUpdate;
   int				m_blinkSequence;
   float			m_GridXCenter;
   float			m_GridYCenter;
   int				m_lightSeqGridHeight;
   int				m_lightSeqGridWidth;
   int				m_GridXCenterAdjust;
   int				m_GridYCenterAdjust;
   _tracer			m_th1, m_th2, m_tt1, m_tt2;
   Collection		*m_pcollection;
   short			*m_pgridData;
   _LightSeqQueue	m_queue;
   SequencerState	m_playAnimation;
   long			m_tailLength;
   long			m_pauseValue;
   long			m_replayCount;
   long			m_updateRate;
   bool			m_playInProgress;
   bool			m_pauseInProgress;

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

   PinTable 		*m_ptable;
   LightSeqData 	m_d;

   LightSeqAnimObject m_lightseqanim;

   void		Animate();

private:
   void		SetupTracers(const SequencerState Animation, long TailLength, long Repeat, long Pause);
   bool		ProcessTracer(_tracer * const pTracer, const LightState State);
   void		SetAllLightsToState(const LightState State);
   void 		SetElementToState(const int index, const LightState State);
   LightState	GetElementState(const int index);
   bool		VerifyAndSetGridElement(const int x, const int y, const LightState State);
};

#endif // !defined(AFX_LIGHTSEQ_H__5EC2D0B7_3868_4CCC_81EC_A4653460DF7E__INCLUDED_)
