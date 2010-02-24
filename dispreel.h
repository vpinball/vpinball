// DispReel.h: Definition of the DispReel class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_)
#define AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

#define MAX_REELS           32

/////////////////////////////////////////////////////////////////////////////
// DispReel

// add data in this class is persisted with the table
class DispReelData
{
public:
    Vertex      m_v1, m_v2;             // position on map (top right corner)
    BOOL        m_fTransparent;         // is the background transparent
    char        m_szImage[MAXTOKEN];    // image to use for the decals.
    ReelType    m_reeltype;
	BOOL		m_fUseImageGrid;
    long		m_imagesPerGridRow;
    float       m_reelcount;            // number of individual reel in the set
    float       m_width, m_height;      // size of each reel
    float       m_reelspacing;          // spacing between each reel and the boarders
    float       m_motorsteps;           // steps (or frames) to move each reel each frame
	float		m_digitrange;			// max number of digits per reel (usually 9)

    char        m_szSound[MAXTOKEN];    // sound to play for each turn of a digit
    BOOL        m_fShading;             // apply alpha shading to the reel (darken the Y axis edges)
    long        m_updateinterval;       // time in ms between each animation update

    COLORREF    m_backcolor;            // colour of the background
    COLORREF    m_reelcolor;            // colour of the reels (valid if m_reeltype = ReelText)
    COLORREF    m_fontcolor;            // colour of the text on the reels (valid if m_reeltype = ReelText)
    char        szfont[MAXSTRING];

    TimerDataRoot m_tdr;                // timer information
};

typedef struct {
    RECT    position;           // position within the object frame (includes rendering size)
    int     currentValue;       // current digit value
    int     motorPulses;        // number of motor pulses received for this reel (can be negative)
    int		motorStepCount;     // when equal to zero then at a whole letter
    float   motorCalcStep;      // calculated steping rate of motor
    float   motorOffset;        // frame value of motor (where to display the reel)
} _reelInfo;

class ATL_NO_VTABLE DispReel :
	public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IDispReel, &IID_IDispReel, &LIBID_VBATESTLib>,
#ifdef VBA
    public CApcProjectItem<DispReel>,
#endif
	//public ISupportErrorInfo,
	//public CComObjectRoot,
    public CComCoClass<DispReel,&CLSID_DispReel>,
    public EventProxy<DispReel, &DIID_IDispReelEvents>,
    public IConnectionPointContainerImpl<DispReel>,
    public IProvideClassInfo2Impl<&CLSID_DispReel, &DIID_IDispReelEvents, &LIBID_VBATESTLib>,
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

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemDispReel)

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(float dx, float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex *pv);
	virtual void PutCenter(Vertex *pv);

DECLARE_REGISTRY_RESOURCEID(IDR_DispReel)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    void        RenderText();
    bool        RenderAnimation();

    PinTable    *m_ptable;
    ObjFrame    *m_pobjframe;      // overall object frame (box) which contains the boarder and reels

    DispReelData m_d;

    IFont       *m_pIFont;

    DispReelUpdater *m_ptu;
    
    int      m_renderwidth, m_renderheight;     // size of each reel (rendered)

private:
    // rendering information (after scaling to render resolution)
    IFont       *m_pIFontPlay;     // Our font, scaled to match play window resolution
    Vector<ObjFrame>    m_vreelframe;     // the generated reel frame which contains the individual reel graphics

    int         m_reeldigitwidth;  // size of the individual reel digits (in bitmap form)
    int         m_reeldigitheight;
    int         m_timenextupdate;
    bool        m_fforceupdate;
	COLORREF	m_rgbImageTransparent;

    _reelInfo   ReelInfo[MAX_REELS];

// IDispReel
public:
    // properties
	STDMETHOD(get_IsTransparent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_IsTransparent)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Type)(/*[out, retval]*/ ReelType *pVal);
	STDMETHOD(put_Type)(/*[in]*/ ReelType newVal);
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
    STDMETHOD(get_IsShading)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_IsShading)(/*[in]*/ VARIANT_BOOL newVal);
    STDMETHOD(get_Steps)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_Steps)(/*[in]*/ float newVal);
	STDMETHOD(get_Font)(/*[out, retval]*/ IFontDisp **pVal);
	STDMETHOD(put_Font)(/*[in]*/ IFontDisp *newVal);
	STDMETHOD(putref_Font)(IFontDisp* pFont);
	STDMETHOD(get_FontColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_FontColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_ReelColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_ReelColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Range)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Range)(/*[in]*/ float newVal);
    STDMETHOD(get_UpdateInterval)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_UpdateInterval)(/*[in]*/ long newVal);
    STDMETHOD(get_UseImageGrid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_UseImageGrid)(/*[in]*/ VARIANT_BOOL newVal);
    STDMETHOD(get_ImagesPerGridRow)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_ImagesPerGridRow)(/*[in]*/ long newVal);
    // methods
    STDMETHOD(ResetToZero)(void);
    STDMETHOD(AddValue)(/*[in]*/ long Value);
	STDMETHOD(SetValue)(/*[in]*/ long Value);
    STDMETHOD(SpinReel)(/*[in]*/ long ReelNumber, /*[in]*/ long PulseCount);

	void    UpdateObjFrame(void);

private:
    float   getBoxWidth(void);
    float   getBoxHeight(void);
    
};

#endif // !defined(AFX_DISPREEL_H__1052EB33_4F53_460B_AAB8_09D3C517F225__INCLUDED_)

