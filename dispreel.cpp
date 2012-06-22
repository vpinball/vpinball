/******************************************************************************
**
** Module name   : DispReel.c
** Module type   : Component source file
** Compiler(s)   : Microsoft Visual C++
** Environment(s): Intel 386 and above
** Author        : Chris Leathley
**
** Description:
**
** This file provides the functionality for the Electro Mechanical Reel Display
**
** Contents:
**  DispReel()
**  ~DispReel()
**  Init()
**  SetDefaults()
**  InterfaceSupportsErrorInfo()
**
**  PreRender()
**  Render()
**
**	GetTimers()
**	GetHitShapes()
**	EndPlay()
**	RenderStatic()
**	RenderMovers()
**	RenderAnimation()
**	RenderText()
**
**	SetObjectPos()
**	MoveOffset()
**	GetCenter()
**	PutCenter()
**	SaveData()
**	InitLoad()
**	LoadToken()
**	InitPostLoad()
**
**	get_BackColor()					Access methods for properties (from VBS & Editor)
**	put_BackColor()
**	get_Reels()
**	put_Reels()
**  get_Width()
**	put_Width()
**  get_Height()
**	put_Height()
**  get_X()
**	put_X()
**  get_Y()
**	put_X()
**	get_IsTransparent()
**	put_IsTransparent
**	get_Image()
**	put_Image()
**	get_Spacing()
**	put_Spacing()
**	get_Sound()
**	put_Sound()
**	get_Steps()
**	put_Steps()
**	get_IsShading()
**	put_IsShading()
**	get_Type()
**	put_Type()
**	get_Font()
**	put_Font()
**	putref_Font()
**	get_FontColor()
**	put_FontColor()
**	get_ReelColor()
**	put_ReelColor()
**	get_Range()
**	put_Range()
**	get_UpdateInterval()
**	put_UpdateInterval()
**
**	AddValue()						Object control Methods (from VBS)
**	ResetToZero()
**	SpinReel()
**	SetValue()
**
**	getBoxWidth()					Private functions
**	getBoxHeight()
**	UpdateObjFrame()
**
** REVISION HISTORY:
** -----------------
**
** v0.0  15/11/01  CL   Created
** v0.1	 28/11/01  CL   Released to Randy
** v0.2	  1/12/01  CL   Changed ReelImage to use a horizontal strip to
**						allow creation of vertical strips greater than
**						1000 pixels (you cannot import a bitmap > 1000)
**
******************************************************************************/

#include "stdafx.h"

const char REEL_NUMBER_TEXT[] = "01234567890";


// Constructor
//
DispReel::DispReel()
{
	m_pIFont = NULL;
	m_ptu = NULL;
}


// Destructor
//
DispReel::~DispReel()
{
	m_pIFont->Release();
}


// This function is called when ever a new instance of this object is created
// (along with the constructor (above))
//
HRESULT DispReel::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
	
	m_ptable = ptable;

	SetDefaults(fromMouseClick);

	m_d.m_v1.x = x;
	m_d.m_v1.y = y;
	m_d.m_v2.x = x+getBoxWidth();
	m_d.m_v2.y = y+getBoxHeight();

	m_pobjframe = NULL;
    //m_preelframe = NULL;

	HRESULT hr;
	float fTmp;
	int iTmp;

	FONTDESC fd;
	fd.cbSizeofstruct = sizeof(FONTDESC);
	
	char tmp[256];
	hr = GetRegString("DefaultProps\\EMReel","FontName", tmp, 256);
	if (hr != S_OK)
		fd.lpstrName = L"Times New Roman";
	else
	{
		unsigned int len = strlen(&tmp[0]);
		fd.lpstrName = (LPOLESTR) malloc(len*sizeof(WCHAR));
		UNICODE_FROM_ANSI(fd.lpstrName, &tmp[0], len); 
		fd.lpstrName[len] = 0;
	}
	
	hr = GetRegStringAsFloat("DefaultProps\\EMReel","FontSize", &fTmp);
	fd.cySize.int64 = (hr == S_OK) ? (LONGLONG)(fTmp * 10000.0) : 260000;
	
	hr = GetRegInt("DefaultProps\\EMReel", "FontWeight", &iTmp);
	fd.sWeight = (hr == S_OK) ? iTmp : FW_BOLD;

	hr = GetRegInt("DefaultProps\\EMReel", "FontCharSet", &iTmp);
	fd.sCharset = (hr == S_OK) ? iTmp : 0;

    hr = GetRegInt("DefaultProps\\EMReel", "FontItalic", &iTmp);
	if (hr == S_OK)
		fd.fItalic = iTmp == 0 ? false : true;
	else
		fd.fItalic = 0;

	hr = GetRegInt("DefaultProps\\EMReel", "FontUnderline", &iTmp);
	if (hr == S_OK)
		fd.fUnderline = iTmp == 0 ? false : true;
	else
		fd.fUnderline = 0;
		
	hr = GetRegInt("DefaultProps\\EMReel", "FontStrikeThrough", &iTmp);
	if (hr == S_OK)
		fd.fStrikethrough = iTmp == 0 ? false : true;
	else
		fd.fStrikethrough = 0;

	OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);

    return InitVBA(fTrue, 0, NULL);
}



// set the defaults for the objects persistent data (m_d.*) in case this
// is a new instance of this object or there is a backwards compatability
// issue (old version of object doesn't contain all the needed fields)
//
void DispReel::SetDefaults(bool fromMouseClick)
{
    // object is only available on the backglass
	m_fBackglass = fTrue;

    // set all the Data defaults
	HRESULT hr;
	float fTmp;
	int iTmp;

	hr = GetRegInt("DefaultProps\\EMReel","ReelType", &iTmp);
	m_d.m_reeltype = (hr == S_OK) && fromMouseClick ? (enum ReelType)iTmp : ReelText;

	hr = GetRegString("DefaultProps\\Ramp","Image", m_d.m_szImage, MAXTOKEN);
	if ((hr != S_OK) || !fromMouseClick)
		m_d.m_szImage[0] = 0;
    
	hr = GetRegString("DefaultProps\\Ramp","Sound", m_d.m_szSound, MAXTOKEN);
	if ((hr != S_OK) || !fromMouseClick)
		m_d.m_szSound[0] = 0;

	hr = GetRegInt("DefaultProps\\EMReel","UseImageGrid", &iTmp);
	if ((hr == S_OK)&& fromMouseClick)
		m_d.m_fUseImageGrid = iTmp == 0 ? false : true;
	else
		m_d.m_fUseImageGrid = fFalse;

    hr = GetRegInt("DefaultProps\\EMReel","ImagesPerRow", &iTmp);
	m_d.m_imagesPerGridRow = (hr == S_OK) && fromMouseClick ? iTmp : 1;

	hr = GetRegInt("DefaultProps\\EMReel","Transparent", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fTransparent = iTmp == 0 ? false : true;
	else
		m_d.m_fTransparent = fFalse;
    
	hr = GetRegInt("DefaultProps\\EMReel","ReelCount", &iTmp);
	m_d.m_reelcount = (hr == S_OK) && fromMouseClick ? iTmp : 5;

    hr = GetRegStringAsFloat("DefaultProps\\EMReel","Width", &fTmp);
	m_d.m_width = (hr == S_OK) && fromMouseClick ? fTmp : 30.0f;
    
	hr = GetRegStringAsFloat("DefaultProps\\EMReel","Height", &fTmp);
	m_d.m_height = (hr == S_OK) && fromMouseClick ? fTmp : 40.0f;
    
	hr = GetRegStringAsFloat("DefaultProps\\EMReel","ReelSpacing", &fTmp);
	m_d.m_reelspacing = (hr == S_OK) && fromMouseClick ? fTmp : 4.0f;
    
	hr = GetRegStringAsFloat("DefaultProps\\EMReel","MotorSteps", &fTmp);
	m_d.m_motorsteps = (hr == S_OK) && fromMouseClick ? fTmp : 2.0f;

	hr = GetRegInt("DefaultProps\\EMReel","DigitRange", &iTmp);
	m_d.m_digitrange = (hr == S_OK) && fromMouseClick ? iTmp : 9;
    
	hr = GetRegInt("DefaultProps\\EMReel","Shading", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fShading = iTmp == 0 ? false : true;
	else
		m_d.m_fShading = fFalse;
    
	hr = GetRegInt("DefaultProps\\EMReel","UpdateInterval", &iTmp);
	m_d.m_updateinterval = (hr == S_OK) && fromMouseClick ? iTmp : 50;

    hr = GetRegInt("DefaultProps\\EMReel","BackColor", &iTmp);
	m_d.m_backcolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(64,64,64);
    
	hr = GetRegInt("DefaultProps\\EMReel","FontColor", &iTmp);
	m_d.m_fontcolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(0,0,0);
    
	hr = GetRegInt("DefaultProps\\EMReel","ReelColor", &iTmp);
	m_d.m_reelcolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(255,255,255);

	hr = GetRegInt("DefaultProps\\EMReel","TimerEnabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false:true;
	else
		m_d.m_tdr.m_fTimerEnabled = false;
	
	hr = GetRegInt("DefaultProps\\EMReel","TimerInterval", &iTmp);
	m_d.m_tdr.m_TimerInterval = (hr == S_OK) && fromMouseClick ? iTmp : 100;

		if (!m_pIFont)
		{
		FONTDESC fd;
		fd.cbSizeofstruct = sizeof(FONTDESC);
		
		hr = GetRegStringAsFloat("DefaultProps\\EMReel","FontSize", &fTmp);
		fd.cySize.int64 = (hr == S_OK) ? (LONGLONG)(fTmp * 10000.0) : 142500;

		char tmp[256];
		hr = GetRegString("DefaultProps\\EMReel","FontName", tmp, 256);
		if ((hr != S_OK) || !fromMouseClick)
			fd.lpstrName = L"Arial Black";
		else
		{
			unsigned int len = strlen(&tmp[0]);
			fd.lpstrName = (LPOLESTR) malloc(len*sizeof(WCHAR));
			UNICODE_FROM_ANSI(fd.lpstrName, &tmp[0], len); 
			fd.lpstrName[len] = 0;
		}

		hr = GetRegInt("DefaultProps\\EMReel", "FontWeight", &iTmp);
		fd.sWeight = (hr == S_OK) && fromMouseClick ? iTmp : FW_NORMAL;
	
		hr = GetRegInt("DefaultProps\\EMReel", "FontCharSet", &iTmp);
		fd.sCharset = (hr == S_OK) && fromMouseClick ? iTmp : 0;
		
		hr = GetRegInt("DefaultProps\\EMReel", "FontItalic", &iTmp);
		if ((hr == S_OK) && fromMouseClick)
			fd.fItalic = iTmp == 0 ? false : true;
		else
			fd.fItalic = 0;

		hr = GetRegInt("DefaultProps\\EMReel", "FontUnderline", &iTmp);
		if ((hr == S_OK) && fromMouseClick)
			fd.fUnderline = iTmp == 0 ? false : true;
		else
			fd.fUnderline = 0;
		
		hr = GetRegInt("DefaultProps\\EMReel", "FontStrikeThrough", &iTmp);
		if ((hr == S_OK) && fromMouseClick)
			fd.fStrikethrough = iTmp == 0 ? false : true;
		else
			fd.fStrikethrough = 0;
		
		OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
		}
	}

void DispReel::WriteRegDefaults()
	{
	char strTmp[MAXTOKEN];
	float fTmp;

	SetRegValue("DefaultProps\\EMReel","ReelType",REG_DWORD,&m_d.m_reeltype,4);
	SetRegValue("DefaultProps\\EMReel","Image", REG_SZ, &m_d.m_szImage,strlen(m_d.m_szImage));
	SetRegValue("DefaultProps\\EMReel","Sound", REG_SZ, &m_d.m_szSound,strlen(m_d.m_szSound));
	SetRegValue("DefaultProps\\Decal","UseImageGrid",REG_DWORD,&m_d.m_fUseImageGrid,4);
	SetRegValue("DefaultProps\\Decal","ImagesPerRow",REG_DWORD,&m_d.m_imagesPerGridRow ,4);
	SetRegValue("DefaultProps\\Decal","Transparent",REG_DWORD,&m_d.m_fTransparent,4);
	SetRegValue("DefaultProps\\Decal","ReelCount",REG_DWORD,&m_d.m_reelcount ,4);
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_width);
	SetRegValue("DefaultProps\\EMReel","Width", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_height);
	SetRegValue("DefaultProps\\EMReel","Height", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_reelspacing);
	SetRegValue("DefaultProps\\EMReel","ReelSPacing", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_motorsteps);
	SetRegValue("DefaultProps\\EMReel","MotorSteps", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\Decal","DigitRange",REG_DWORD,&m_d.m_digitrange,4);
	SetRegValue("DefaultProps\\Decal","Shading",REG_DWORD,&m_d.m_fShading,4);
	SetRegValue("DefaultProps\\Decal","UpdateInterval",REG_DWORD,&m_d.m_updateinterval,4);
	SetRegValue("DefaultProps\\EMReel","BackColor",REG_DWORD,&m_d.m_backcolor,4);
	SetRegValue("DefaultProps\\EMReel","FontColor",REG_DWORD,&m_d.m_fontcolor,4);
	SetRegValue("DefaultProps\\EMReel","ReelColor",REG_DWORD,&m_d.m_reelcolor,4);
	SetRegValue("DefaultProps\\EMReel","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\EMReel","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);

	if (m_pIFont)
		{
		FONTDESC fd;
		fd.cbSizeofstruct = sizeof(FONTDESC);
		m_pIFont->get_Size(&fd.cySize); 
		m_pIFont->get_Name(&fd.lpstrName); 
		m_pIFont->get_Weight(&fd.sWeight); 
		m_pIFont->get_Charset(&fd.sCharset); 
		m_pIFont->get_Italic(&fd.fItalic);
		m_pIFont->get_Underline(&fd.fUnderline); 
		m_pIFont->get_Strikethrough(&fd.fStrikethrough); 
		
		fTmp = (float)(fd.cySize.int64 / 10000.0);
		sprintf_s(&strTmp[0], 40, "%f", fTmp);
		SetRegValue("DefaultProps\\EMReel","FontSize", REG_SZ, &strTmp,strlen(strTmp));
		int charCnt = wcslen(fd.lpstrName) +1;
		WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, charCnt, strTmp, 2*charCnt, NULL, NULL);
		SetRegValue("DefaultProps\\EMReel","FontName", REG_SZ, &strTmp,strlen(strTmp));
		SetRegValue("DefaultProps\\EMReel","FontWeight",REG_DWORD,&fd.sWeight,4);
		SetRegValue("DefaultProps\\EMReel","FontCharSet",REG_DWORD,&fd.sCharset,4);
		SetRegValue("DefaultProps\\EMReel","FontItalic",REG_DWORD,&fd.fItalic,4);
		SetRegValue("DefaultProps\\EMReel","FontUnderline",REG_DWORD,&fd.fUnderline,4);
		SetRegValue("DefaultProps\\EMReel","FontStrikeThrough",REG_DWORD,&fd.fStrikethrough,4);
		}
	}


STDMETHODIMP DispReel::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
        &IID_IDispReel,
	};

	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); ++i)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}



// this function draws the shape of the object with a solid fill
// only used in the editor and not the game
//
// this is called before the grid lines are drawn on the map
//
void DispReel::PreRender(Sur *psur)
{
    psur->SetBorderColor(-1,false,0);
	psur->SetFillColor(m_d.m_backcolor);
	psur->SetObject(this);

    // draw background box
    psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);

    // draw n reels in the box (in blue)
    psur->SetFillColor(m_d.m_reelcolor);
    for (int i=0; i<m_d.m_reelcount; ++i)
    {
        // set up top corner point
		const float fi = (float)i;
        const float x = m_d.m_v1.x + fi*m_d.m_width + fi*m_d.m_reelspacing + m_d.m_reelspacing;
        const float y = m_d.m_v1.y + m_d.m_reelspacing;
		const float x2 = x+m_d.m_width;
		const float y2 = y+m_d.m_height;

		// set up points (clockwise)
		const Vertex2D rgv[4] = {Vertex2D(x,y),Vertex2D(x2,y),Vertex2D(x2,y2),Vertex2D(x,y2)};			
        psur->Polygon(rgv, 4);
    }
}



// this function draws the shape of the object with a black outline (no solid fill)
// only used in the editor and not the game
//
// this is called after the grid lines have been drawn on the map.  draws a solid
// outline over the grid lines
//
void DispReel::Render(Sur *psur)
{
	if( !GetPTable()->GetEMReelsEnabled() ) return;

	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);
	psur->SetObject(NULL);

    // draw background box
	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);

    // draw n reels in the box
    for (int i=0; i<m_d.m_reelcount; ++i)
    {
        // set up top corner point
		const float fi = (float)i;
        const float x = m_d.m_v1.x + fi*m_d.m_width + fi*m_d.m_reelspacing + m_d.m_reelspacing;
        const float y = m_d.m_v1.y + m_d.m_reelspacing;
		const float x2 = x+m_d.m_width;
		const float y2 = y+m_d.m_height;

        // set up points (clockwise)
		const Vertex2D rgv[4] = {Vertex2D(x,y),Vertex2D(x2,y),Vertex2D(x2,y2),Vertex2D(x,y2)};
        psur->Polygon(rgv, 4);
    }
}



// Registers the timer with the game call which then makes a call back when the interval
// has expired.
//
// for this sort of object (reel driver) it is basically not really required but hey, somebody
// might use it..
//
void DispReel::GetTimers(Vector<HitTimer> *pvht)
{
	IEditable::BeginPlay();

	HitTimer * const pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
    {
        pvht->AddElement(pht);
    }
}



// This function is supposed to return the hit shapes for the object but since it is
// off screen we use it to register the screen updater in the game engine.. this means
// that Check3d (and Draw3d) are called in the updater class.
//
void DispReel::GetHitShapes(Vector<HitObject> *pvho)
{
    m_ptu = new DispReelUpdater(this);

	m_ptu->m_dispreelanim.m_znear = 0;
	m_ptu->m_dispreelanim.m_zfar = 0;

	// HACK - adding object directly to screen update list.  Someday make hit objects and screenupdaters seperate objects
	g_pplayer->m_vscreenupdate.AddElement(&m_ptu->m_dispreelanim);
}

void DispReel::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}


// This method is called as the game exits..
// it cleans up any allocated memory used by the instance of the object
//
void DispReel::EndPlay()
{
    // free up object frame (buffer)
    if (m_pobjframe != NULL)    // Failed Player case
	{
		delete m_pobjframe;
		m_pobjframe = NULL;
	}

    // free up reel buffer
    if (m_vreelframe.Size() != 0)   // Failed Player case
	{
		while (m_vreelframe.Size() != 0)
			{
			delete m_vreelframe.ElementAt(0);
			m_vreelframe.RemoveElementAt(0);
			}
	}

	if (m_ptu)
	{
		delete m_ptu;
		m_ptu = NULL;
	}

	IEditable::EndPlay();
}

void DispReel::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
{

}
void DispReel::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
{

}


void DispReel::RenderMoversFromCache(Pin3D *ppin3d)
	{
	RenderMovers(ppin3d->m_pd3dDevice);
	/*m_pobjframe = new ObjFrame();
	ppin3d->ReadObjFrameFromCacheFile(m_pobjframe);*/
	}
	
// This function is called during the redering process // Old comments from RenderStatic
// (before the game starts, but after play has been pressed)
//
// it is designed to generate any static information/graphics for the table
// to use
//
// This function makes up a strip graphic from either the specified bitmap or
// generates one from the font specification..
//
// It then works out the correct scaling of all the items depending on the rendering
// screen size
//	
// This function is called during the redering process
// (before the game starts, but after play has been pressed)
//
// it is designed to generate any 3d animation as the 3D map has been set up
// to use in the process
//

WORD rgiDispReel[4] = {0,1,2,3};

void DispReel::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
{
    // set any defaults for the game rendering
    m_timenextupdate = g_pplayer->m_timeCur + m_d.m_updateinterval;
    m_fforceupdate = false;

    m_pobjframe = GetPTable()->GetEMReelsEnabled() ? (new ObjFrame()) : NULL;

	if (m_pobjframe == NULL)
        return;

    // get information about the table player (size sizes, resolution, etc..)
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	// get the render sizes of the objects (reels and frame)
    m_renderwidth    = max(0, (int)((m_d.m_width * (float)(1.0/1000.0)) * ppin3d->m_dwRenderWidth));
    m_renderheight   = max(0, (int)((m_d.m_height * (float)(1.0/750.0)) * ppin3d->m_dwRenderHeight));
    const int m_renderspacingx = max(0, (int)((m_d.m_reelspacing * (float)(1.0/1000.0)) * ppin3d->m_dwRenderWidth));
    const int m_renderspacingy = max(0, (int)((m_d.m_reelspacing * (float)(1.0/750.0))  * ppin3d->m_dwRenderHeight));

    // get the size of the object frame (size of entire reel set and border)
	m_pobjframe->rc.left = (int)((m_d.m_v1.x * (float)(1.0/1000.0)) * ppin3d->m_dwRenderWidth);
	m_pobjframe->rc.top = (int)((m_d.m_v1.y * (float)(1.0/750.0)) * ppin3d->m_dwRenderHeight);
	// i cant use v2 as it really doesn't scale properly.
	m_pobjframe->rc.right = m_pobjframe->rc.left + m_d.m_reelcount * (m_renderwidth+m_renderspacingx) + m_renderspacingx;
	m_pobjframe->rc.bottom = m_pobjframe->rc.top + m_renderheight + (2 * m_renderspacingy);

    // set the boundarys of the object frame (used for clipping I assume)
	m_ptu->m_dispreelanim.m_rcBounds = m_pobjframe->rc;

    // set up all the reel positions within the object frame
    int x1 = m_renderspacingx;
    
	for (int i=0; i<m_d.m_reelcount; ++i)
    {
        ReelInfo[i].position.left	= x1/* + m_pobjframe->rc.left*/;
        ReelInfo[i].position.right	= x1 + m_renderwidth/* + m_pobjframe->rc.left*/;
        ReelInfo[i].position.top	= m_renderspacingy/* + m_pobjframe->rc.top*/;
        ReelInfo[i].position.bottom	= m_renderspacingy + m_renderheight/* + m_pobjframe->rc.top*/;

        ReelInfo[i].currentValue	= 0;
        ReelInfo[i].motorPulses		= 0;
        ReelInfo[i].motorStepCount	= 0;
        ReelInfo[i].motorCalcStep	= 0;
        ReelInfo[i].motorOffset		= 0;

        // move to the next reel
        x1 += m_renderspacingx+m_renderwidth;
    }

    // Set up the reel strip (either using bitmaps or fonts)
    if (m_d.m_reeltype == ReelImage)
    {
		// get a pointer to the image specified in the object
		PinImage * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

        // was there a valid image (if not then m_preelframe->pdds remains NULL and void)
        if (pin)
        {
            //HDC hdcImage;
			//HDC hdcReelFrame;
			int	GridCols, GridRows;

			// get the number of images per row of the image
			if (m_d.m_fUseImageGrid)
			{
				GridCols = m_d.m_imagesPerGridRow;
				if (GridCols != 0) // best to be safe
				{
					GridRows = (m_d.m_digitrange+1) / GridCols;
					if ( (GridRows * GridCols) < (m_d.m_digitrange+1) )
					{
						++GridRows;
					}
				}
				else
				{
					GridRows = 1;
				}
			}
			else
			{
				GridCols = m_d.m_digitrange+1;
				GridRows = 1;
			}

			// save the color to use in any transparent blitting
			m_rgbImageTransparent = pin->m_rgbTransparent;

            // get the size of the individual reel digits (if m_digitrange is wrong we can forget the rest)
            m_reeldigitwidth  = (float)pin->m_width / GridCols;
            m_reeldigitheight = (float)pin->m_height / GridRows;

            // work out the size of the reel image strip (adds room for an extra digit at the end)
            //const int width  = m_reeldigitwidth;
            //const int height = (m_reeldigitheight * (m_d.m_digitrange+1)) + m_reeldigitheight;
            // allocate some memory for this strip
			for (int i=0; i<=m_d.m_digitrange; ++i)
				{
				ObjFrame * const pobjframe = new ObjFrame();
				if (pobjframe == NULL)
					{
					return;
					}
				pobjframe->pdds	= NULL;
				m_vreelframe.AddElement(pobjframe);				
				}
            
            // now make the reel image strip..  It has to use the BitBlt function and not the direct draw
			// blt function (ie. m_preelframe->pdds->Blt) to copy the bitmaps as this function coverts the
			// destination bitmap to the same colour depth of the game (all images in the image manager
			// are stored as 32bit).

			// from this point on we can use the Direct Draw Blt function as it handles and scaling and
			// is much faster

			// get the HDC of the source image
			//pin->m_pdsBuffer->GetDC(&hdcImage);
			// get the HDC of the reel frame object
			//m_preelframe->pdds->GetDC(&hdcReelFrame);

			// Render images and collect them
			
			// New rendering stuff
			ppin3d->SetMaterial(1.0f, 1.0f, 1.0f, 0.5f);
				
			pin->EnsureMaxTextureCoordinates();
				
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

			// Set texture to mirror, so the alpha state of the texture blends correctly to the outside
			pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_MIRROR);
				
			pin->EnsureColorKey();
			pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);

			//pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
			//pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			ppin3d->SetColorKeyEnabled(fFalse);
			ppin3d->SetFiltersLinear();
			ppin3d->SetAlphaEnabled(fTrue);
				
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0xe0);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

			ppin3d->EnableLightMap(fFalse, -1);
			
			//

			Vertex3D rgv3D[4];
			for (int l=0; l<4; ++l)
				rgv3D[l].z = 1.0f;//height + 0.2f;

			rgv3D[0].x = 0;//(float)rectSrc.left;
			rgv3D[0].y = 0;//(float)rectSrc.top;
			rgv3D[0].tu = 0;
			rgv3D[0].tv = 0;

			rgv3D[1].x = m_d.m_width;//(((double)m_renderwidth)/ppin3d->m_dwRenderWidth)*1000;// m_d.m_width;//(float)rectSrc.right;
			rgv3D[1].y = 0;//(float)rectSrc.top;
			rgv3D[1].tu = pin->m_maxtu;
			rgv3D[1].tv = 0;

			rgv3D[2].x = rgv3D[1].x;//m_d.m_width;//(float)rectSrc.right;
			rgv3D[2].y = m_d.m_height;//(float)rectSrc.bottom;
			rgv3D[2].tu = pin->m_maxtu;
			rgv3D[2].tv = pin->m_maxtv;

			rgv3D[3].x = 0;//(float)rectSrc.left;
			rgv3D[3].y = m_d.m_height;//(float)rectSrc.bottom;
			rgv3D[3].tu = 0;
			rgv3D[3].tv = pin->m_maxtv;
			
			SetHUDVertices(rgv3D, 4);
			
			{
			D3DMATERIAL7 mtrl;
			pd3dDevice->GetMaterial(&mtrl);
			SetDiffuseFromMaterial(rgv3D, 4, &mtrl);
			}
			
			//ppin3d->ExpandExtents(&m_preelframe->rc, rgv3D, NULL, NULL, 4, m_fBackglass);

			RECT rectSrc;
			rectSrc.left = 0;
			rectSrc.top = 0;
			rectSrc.right = m_renderwidth;
			rectSrc.bottom = m_renderheight;

			// Reset color key in back buffer
			// this is usually not done since the buffer
			// should be clear from the last object,
			// but for caching, this object will draw
			// when others don't, so be safe.
			DDBLTFX ddbltfx;
			ddbltfx.dwSize = sizeof(DDBLTFX);
			ddbltfx.dwFillColor = m_rgbImageTransparent;
			ppin3d->m_pddsBackBuffer->Blt(&rectSrc, NULL,
					&rectSrc, DDBLT_COLORFILL, &ddbltfx);

			const float ratiox = (float)m_reeldigitwidth  * pin->m_maxtu / (float)pin->m_width;
			const float ratioy = (float)m_reeldigitheight * pin->m_maxtv / (float)pin->m_height;

			int gr = 0;
			int gc = 0;

			for (int i=0; i<=m_d.m_digitrange; ++i)
			{
				rgv3D[0].tu = rgv3D[3].tu = (float)gc * ratiox;
				rgv3D[0].tv = rgv3D[1].tv = (float)gr * ratioy;
			
				rgv3D[1].tu = rgv3D[2].tu = rgv3D[0].tu + ratiox;
				rgv3D[2].tv = rgv3D[3].tv = rgv3D[0].tv + ratioy;
			
				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_VERTEX,
												  rgv3D, 4,
												  rgiDispReel, 4, NULL);

				RECT rectDst;
				rectDst.left = 0;
				rectDst.top = 0;//m_renderheight*i;
				rectDst.right = m_renderwidth;
				rectDst.bottom = rectDst.top + m_renderheight;

				m_vreelframe.ElementAt(i)->pdds = ppin3d->CreateOffscreenWithCustomTransparency(/*m_reeldigitwidth*/m_renderwidth, /*m_reeldigitheight*/m_renderheight, m_rgbImageTransparent);
				m_vreelframe.ElementAt(i)->pdds->Blt(&rectDst, ppin3d->m_pddsBackBuffer, &rectSrc, 0, NULL);

				// Reset color key in back buffer
				ppin3d->m_pddsBackBuffer->Blt(&rectSrc, NULL,
						&rectSrc, DDBLT_COLORFILL, &ddbltfx);
				/*BitBlt(hdcReelFrame,			// handle to destination device context
						0,						// x-coordinate of destination rectangle's upper-left corner
						i*m_reeldigitheight,	// y-coordinate of destination rectangle's upper-left corner
						m_reeldigitwidth,		// width of destination rectangle
						m_reeldigitheight,		// height of destination rectangle
						hdcImage,				// handle to source device context
						gc*m_reeldigitwidth,	// x-coordinate of source rectangle's upper-left corner
						gr*m_reeldigitheight,	// y-coordinate of source rectangle's upper-left corner
						SRCCOPY);				// raster operation code
				*/
				
				/*StretchBlt(hdcReelFrame,		// handle to destination device context
						0,						// x-coordinate of destination rectangle's upper-left corner
						i*m_renderheight,		// y-coordinate of destination rectangle's upper-left corner
						m_renderwidth,			// width of destination rectangle
						m_renderheight,			// height of destination rectangle
						hdcImage,				// handle to source device context
						gc*m_reeldigitwidth,	// x-coordinate of source rectangle's upper-left corner
						gr*m_reeldigitheight,	// y-coordinate of source rectangle's upper-left corner
						m_reeldigitwidth,
						m_reeldigitheight,
						SRCCOPY);				// raster operation code
				*/
				
				++gc;
				if (gc >= GridCols)
				{
					gc = 0;
					++gr;
				}
				
				if (i == m_d.m_digitrange)
				{
					// Go back and draw the first picture at the end of the strip
					gc = 0;
					gr = 0;
				}
			}

			// now copy the first digit graphic onto the end on the reel object frame
			// this means the bitmap starts and ends with the same graphic
			/*BitBlt(hdcReelFrame,
					0,
					m_reeldigitheight * (m_d.m_digitrange+1),	// start after the last number
					m_reeldigitwidth,
					m_reeldigitheight,
					hdcImage,
					0,
					0,
					SRCCOPY);*/

			//m_preelframe->pdds->ReleaseDC(hdcReelFrame);
			//pin->m_pdsBuffer->ReleaseDC(hdcImage);
			
		pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
        
        // reset device 
        
        pd3dDevice->SetTexture(ePictureTexture, NULL);
		pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

		pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
        }
    }
    else    /* generate a strip of numbers using font rendering */
	{
        RECT	rcOut;
        HFONT   hFont;

        // text reels are purely 0-9 and nothing else
        m_d.m_digitrange = 9;

        // make a clone of the specified font
        m_pIFont->Clone(&m_pIFontPlay);
        // scale the font (either up or down) to suit the screen resolution
        CY size;
        m_pIFontPlay->get_Size(&size);
        size.int64 = size.int64 / 1000 * ppin3d->m_dwRenderWidth;
        m_pIFontPlay->put_Size(size);

        m_pIFontPlay->get_hFont(&hFont);
	    HDC hdc = GetDC(NULL);
        SelectObject(hdc, hFont);
        SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

        // work out the maximum width and height for the selected font
        int maxwidth = m_renderwidth;
        int maxheight = m_renderheight;
        const int length = lstrlen(REEL_NUMBER_TEXT);
        for (int i=0; i<length; ++i)
        {
            rcOut.left = 0;
            rcOut.top = 0;
            rcOut.right = maxwidth;
            rcOut.bottom = maxwidth;
            DrawText(hdc, &REEL_NUMBER_TEXT[i], 1, &rcOut, DT_NOCLIP | DT_NOPREFIX | DT_CALCRECT);
            maxwidth = max(maxwidth, rcOut.right);
            maxheight = max(maxheight, rcOut.bottom);
        }
        ReleaseDC(NULL, hdc);

        // set the size of the individual reel digits
        m_reeldigitwidth  = (float)maxwidth;
        m_reeldigitheight = (float)maxheight;

		for (int i=0; i < length; ++i)
			{
			ObjFrame * const pobjframe = new ObjFrame();
			if (pobjframe == NULL)
				return;
			pobjframe->pdds	= NULL;
			m_vreelframe.AddElement(pobjframe);			
			}

        
		SetTextColor(hdc, m_d.m_fontcolor);		// set the font colour
		SetBkMode(hdc, TRANSPARENT);
		for (int i=0; i < length; ++i)
        {
			// allocate some memory for this strip
			m_vreelframe.ElementAt(i)->pdds = ppin3d->CreateOffscreen(maxwidth, maxheight);
			// fill the strip with the reel colour
			m_vreelframe.ElementAt(i)->pdds->GetDC(&hdc);
			HBRUSH hbrush = CreateSolidBrush(m_d.m_reelcolor);
			HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
			PatBlt(hdc, 0, 0, maxwidth, maxheight, PATCOPY);
			SelectObject(hdc, hbrushold);
			DeleteObject(hbrush);
			
			// set the font plotting parameters
			SelectObject(hdc, hFont);
			SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
        
            rcOut.left = 0;
            rcOut.top = 0;//i * maxheight;
            rcOut.right = maxwidth;
            rcOut.bottom = rcOut.top + maxheight;
            DrawText(hdc, &REEL_NUMBER_TEXT[i], 1, &rcOut, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
            m_vreelframe.ElementAt(i)->pdds->ReleaseDC(hdc);
		}
		m_pIFontPlay->Release();
		
		// For text, try to pick a color that won't be used (hack)
		m_rgbImageTransparent = 0xff;
		if ((m_d.m_fontcolor & 0xffffff) == m_rgbImageTransparent)
			{
			m_rgbImageTransparent = 0xffff;
			}
			
		if ((m_d.m_reelcolor & 0xffffff) == m_rgbImageTransparent)
			{
			m_rgbImageTransparent = 0xff00;
			}
			
		if ((m_d.m_fontcolor & 0xffffff) == m_rgbImageTransparent)
			{
			m_rgbImageTransparent = 0xff0000;
			}
	}
	
	// allocate the memory for this object (returns with a LPDIRECTDRAWSURFACE7)
	m_pobjframe->pdds = ppin3d->CreateOffscreenWithCustomTransparency(m_pobjframe->rc.right - m_pobjframe->rc.left, m_pobjframe->rc.bottom - m_pobjframe->rc.top, m_rgbImageTransparent);
	
	//ppin3d->WriteObjFrameToCacheFile(m_pobjframe);
}



// This function is called during Check3D.  It basically check to see if the update
// interval has expired and if so handles the rolling of the reels according to the
// number of motor steps queued up for each reel
//
// if a screen update is required it returns true..
//
bool DispReel::RenderAnimation()
{
    bool    rc = false;
    OLECHAR mySound[256];

    if (g_pplayer->m_timeCur >= m_timenextupdate)
	{
        m_timenextupdate = g_pplayer->m_timeCur + m_d.m_updateinterval;

        // work out the roll over values
        const int OverflowValue = m_d.m_digitrange;
        const int AdjustValue   = OverflowValue+1;

		const float step = (float)m_reeldigitheight / m_d.m_motorsteps;

        // start at the last reel and work forwards (right to left)
        for (int i=m_d.m_reelcount-1; i>=0; i--)
        {
            // if the motor has stoped, and there are still motor steps then start another one
            if ((ReelInfo[i].motorPulses != 0) && (ReelInfo[i].motorStepCount == 0))
            {
                // get the number of steps (or increments) needed to move the reel
                ReelInfo[i].motorStepCount = (int)m_d.m_motorsteps;
				ReelInfo[i].motorCalcStep = (ReelInfo[i].motorPulses > 0) ? step : -step;
				ReelInfo[i].motorOffset = 0;

                // play the sound (if any) for each click of the reel
				if (m_d.m_szSound[0] != 0)
				{
	               MultiByteToWideChar(CP_ACP, 0, m_d.m_szSound, -1, mySound, 32);
		           m_ptable->PlaySound(mySound, 0, 1.0f);
				}
            }

            // is the reel in the process of moving??
            if (ReelInfo[i].motorStepCount != 0)
            {
                ReelInfo[i].motorOffset += ReelInfo[i].motorCalcStep;
                ReelInfo[i].motorStepCount--;
                // have re reached the end of the step
                if (ReelInfo[i].motorStepCount <= 0)
                {
					ReelInfo[i].motorStepCount = 0;      // best to be safe (paranoid)
					ReelInfo[i].motorOffset = 0;

					if (ReelInfo[i].motorPulses < 0)
					{
						ReelInfo[i].motorPulses++;
					    ReelInfo[i].currentValue--;
				        if (ReelInfo[i].currentValue < 0)
						{
		                    ReelInfo[i].currentValue += AdjustValue;
			                // if not the first reel then decrement the next reel by 1
					        if (i != 0)
						    {
			                    ReelInfo[i-1].motorPulses--;
							}
						}
					}
					else
					{
						ReelInfo[i].motorPulses--;
					    ReelInfo[i].currentValue++;
				        if (ReelInfo[i].currentValue > OverflowValue)
						{
		                    ReelInfo[i].currentValue -= AdjustValue;
			                // if not the first reel then increment the next reel
							// along by 1 (just like a car odometer)
					        if (i != 0)
						    {
			                    ReelInfo[i-1].motorPulses++;
							}
						}
					}
				}
				// there is a change in the animation, redraw the frame
				rc = true;
            }
        }

        // if there is a change or we are forced to update, then do so..
        if (rc || m_fforceupdate)
        {
			// redraw the reels (and boarder) into the objectframe
			RenderText();
            m_fforceupdate = false;
            rc = true;
		}
	}

	return rc;
}



// This function gets calls just before the game starts to draw the first instance of
// the object on the screen.  it is not called after that.  Check3D and Draw3D handle
// any dynamic updates.
//
void DispReel::RenderText()
{
	//Pin3D	* const ppin3d = &g_pplayer->m_pin3d;

    // update the object frame (or in this case, draw it for the first time)
    UpdateObjFrame();

    // copy the object frame onto the back buffer
	if( GetPTable()->GetEMReelsEnabled() )
	{
	    m_ptu->m_dispreelanim.Draw3D(NULL);
	}
    //ppin3d->m_pddsBackBuffer->BltFast(&m_pobjframe->rc, m_pobjframe->pdds, NULL, DDBLTFAST_SRCCOLORKEY/*DDBLTFAST_WAIT*/, NULL);
}



void DispReel::SetObjectPos()
{
	g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
}



void DispReel::MoveOffset(const float dx, const float dy)
{
	m_d.m_v1.x += dx;
	m_d.m_v1.y += dy;

	m_d.m_v2.x += dx;
	m_d.m_v2.y += dy;

	m_ptable->SetDirtyDraw();
}



void DispReel::GetCenter(Vertex2D *pv)
{
	*pv = m_d.m_v1;
}



void DispReel::PutCenter(Vertex2D *pv)
{
	m_d.m_v1 = *pv;

	m_d.m_v2.x = pv->x + getBoxWidth();
	m_d.m_v2.y = pv->y + getBoxHeight();

	m_ptable->SetDirtyDraw();
}



HRESULT DispReel::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
	bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
    bw.WriteInt(FID(TYPE), m_d.m_reeltype);
	bw.WriteInt(FID(CLRB), m_d.m_backcolor);
	bw.WriteInt(FID(CLRF), m_d.m_fontcolor);
    bw.WriteInt(FID(CLRR),  m_d.m_reelcolor);
    bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteBool(FID(TRNS), m_d.m_fTransparent);
	bw.WriteString(FID(IMAG), m_d.m_szImage);
    bw.WriteString(FID(SOUN), m_d.m_szSound);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteFloat(FID(WDTH), m_d.m_width);
	bw.WriteFloat(FID(HIGH), m_d.m_height);
	const float reel = (float)m_d.m_reelcount;
    bw.WriteFloat(FID(RCNT), reel);
    bw.WriteFloat(FID(RSPC), m_d.m_reelspacing);
    bw.WriteFloat(FID(MSTP), m_d.m_motorsteps);
    bw.WriteBool(FID(SHAD), m_d.m_fShading);
	const float dig = (float)m_d.m_digitrange;
    bw.WriteFloat(FID(RANG), dig);
    bw.WriteInt(FID(UPTM), m_d.m_updateinterval);
    bw.WriteBool(FID(UGRD), m_d.m_fUseImageGrid);
    bw.WriteInt(FID(GIPR), m_d.m_imagesPerGridRow);

	bw.WriteTag(FID(FONT));
	IPersistStream * ips;
	m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
	ips->Save(pstm, TRUE);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey); //add BDS2

	bw.WriteTag(FID(ENDB));

	return S_OK;
}



HRESULT DispReel::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	SetDefaults(false);
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	ULONG read = 0;
	HRESULT hr = S_OK;

	m_ptable = ptable;

	DWORD dwID;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

    if(FAILED(hr = pstm->Read(&m_d, sizeof(DispReelData), &read)))
		return hr;

	FONTDESC fd;
	fd.cbSizeofstruct = sizeof(FONTDESC);
	fd.lpstrName = L"Times New Roman";
	fd.cySize.int64 = 260000;
	//fd.cySize.Lo = 0;
	fd.sWeight = FW_BOLD;
	fd.sCharset = 0;
    fd.fItalic = 0;
	fd.fUnderline = 0;
	fd.fStrikethrough = 0;
	OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);

	IPersistStream * ips;
	m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

	ips->Load(pstm);

	//ApcProjectItem.Register(ptable->ApcProject, GetDispatch(), dwID);
	*pid = dwID;

	return hr;
#endif
}



BOOL DispReel::LoadToken(int id, BiffReader *pbr)
{
	if (id == FID(PIID))
	{
		pbr->GetInt((int *)pbr->m_pdata);
	}
	else if (id == FID(VER1))
		{
			pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D));
		}
	else if (id == FID(VER2))
		{
			pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D));
		}
	else if (id == FID(WDTH))
		{
			pbr->GetFloat(&m_d.m_width);
		}
	else if (id == FID(HIGH))
		{
			pbr->GetFloat(&m_d.m_height);
		}
	else if (id == FID(CLRB))
		{
			pbr->GetInt(&m_d.m_backcolor);
		}
	else if (id == FID(CLRF))
		{
			pbr->GetInt(&m_d.m_fontcolor);
		}
    else if (id == FID(CLRR))
		{
			pbr->GetInt(&m_d.m_reelcolor);
		}
	else if (id == FID(TMON))
		{
			pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
			pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(NAME))
		{
			pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(TRNS))
		{
			pbr->GetBool(&m_d.m_fTransparent);
		}
	else if (id == FID(IMAG))
		{
			pbr->GetString(m_d.m_szImage);
		}
    else if (id == FID(RCNT))
		{
			float reel;
			pbr->GetFloat(&reel);
			m_d.m_reelcount = (int)reel;
		}
    else if (id == FID(RSPC))
		{
			pbr->GetFloat(&m_d.m_reelspacing);
		}
    else if (id == FID(MSTP))
		{
			pbr->GetFloat(&m_d.m_motorsteps);
		}
    else if (id == FID(SOUN))
		{
			pbr->GetString(m_d.m_szSound);
		}
    else if (id == FID(SHAD))
		{
			pbr->GetBool(&m_d.m_fShading);
		}
	else if (id == FID(TYPE))
		{
			pbr->GetInt(&m_d.m_reeltype);
		}
	else if (id == FID(UGRD))
		{
			pbr->GetBool(&m_d.m_fUseImageGrid);
		}
	else if (id == FID(GIPR))
		{
   			pbr->GetInt(&m_d.m_imagesPerGridRow);
		}
	else if (id == FID(RANG))
		{
			float dig;
            pbr->GetFloat(&dig);
			m_d.m_digitrange = (int)dig;
		}
    else if (id == FID(UPTM))
		{
            pbr->GetInt(&m_d.m_updateinterval);
		}
	else if (id == FID(FONT))
		{
		if (!m_pIFont)
			{
				FONTDESC fd;
				fd.cbSizeofstruct = sizeof(FONTDESC);
				fd.lpstrName = L"Times New Roman";
				fd.cySize.int64 = 260000;
				//fd.cySize.Lo = 0;
				fd.sWeight = FW_BOLD;
				fd.sCharset = 0;
				fd.fItalic = 0;
				fd.fUnderline = 0;
				fd.fStrikethrough = 0;
				OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
			}

		IPersistStream * ips;
		m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

		ips->Load(pbr->m_pistream);
	}
	else
		{
		ISelect::LoadToken(id, pbr);
		}
	return fTrue;
}



HRESULT DispReel::InitPostLoad()
{
	m_pobjframe = NULL;

	return S_OK;
}

void DispReel::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPDISPREEL_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPDISPREEL_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPDISPREEL_STATE, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

// These methods provide the interface to the object through both the editor
// and the script for a of the object properties
//
STDMETHODIMP DispReel::get_BackColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_backcolor;

	return S_OK;
}

STDMETHODIMP DispReel::put_BackColor(OLE_COLOR newVal)
{
	STARTUNDO
	m_d.m_backcolor = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Reels(float *pVal)
{
    *pVal = (float)m_d.m_reelcount;

    return S_OK;
}

STDMETHODIMP DispReel::put_Reels(float newVal)
{
	STARTUNDO

	m_d.m_reelcount = min(max(1, (int)newVal), MAX_REELS); // must have at least 1 reel and a max of MAX_REELS
	m_d.m_v2.x = m_d.m_v1.x+getBoxWidth();
	m_d.m_v2.y = m_d.m_v1.y+getBoxHeight();

	STOPUNDO

    return S_OK;
}

STDMETHODIMP DispReel::get_Width(float *pVal)
{
	*pVal = m_d.m_width;

	return S_OK;
}

STDMETHODIMP DispReel::put_Width(float newVal)
{
	STARTUNDO
    
	m_d.m_width = max(0.0f, newVal);
	m_d.m_v2.x = m_d.m_v1.x+getBoxWidth();
	
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Height(float *pVal)
{
	*pVal = m_d.m_height;

	return S_OK;
}

STDMETHODIMP DispReel::put_Height(float newVal)
{
	STARTUNDO
    
	m_d.m_height = max(0.0f, newVal);
	m_d.m_v2.y = m_d.m_v1.y+getBoxHeight();
	
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_X(float *pVal)
{
	*pVal = m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP DispReel::put_X(float newVal)
{
	STARTUNDO
	
	const float delta = newVal - m_d.m_v1.x;
	m_d.m_v1.x += delta;
	m_d.m_v2.x = m_d.m_v1.x+getBoxWidth();
	
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Y(float *pVal)
{
	*pVal = m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP DispReel::put_Y(float newVal)
{
	STARTUNDO

	const float delta = newVal - m_d.m_v1.y;
	m_d.m_v1.y += delta;
	m_d.m_v2.y = m_d.m_v1.y+getBoxHeight();

	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_IsTransparent(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fTransparent);

	return S_OK;
}

STDMETHODIMP DispReel::put_IsTransparent(VARIANT_BOOL newVal)
{
	STARTUNDO
	m_d.m_fTransparent = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Image(BSTR *pVal)
{
	OLECHAR wz[512];
    MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP DispReel::put_Image(BSTR newVal)
{
	STARTUNDO
	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Spacing(float *pVal)
{
    *pVal = m_d.m_reelspacing;
    return S_OK;
}

STDMETHODIMP DispReel::put_Spacing(float newVal)
{
	STARTUNDO

	m_d.m_reelspacing = max(0.0f, newVal);
	m_d.m_v2.x = m_d.m_v1.x+getBoxWidth();
	m_d.m_v2.y = m_d.m_v1.y+getBoxHeight();

	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Sound(BSTR *pVal)
{
	OLECHAR wz[512];
    MultiByteToWideChar(CP_ACP, 0, m_d.m_szSound, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP DispReel::put_Sound(BSTR newVal)
{
	STARTUNDO
    WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSound, 32, NULL, NULL);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Steps(float *pVal)
{
    *pVal = floorf(m_d.m_motorsteps);

    return S_OK;
}

STDMETHODIMP DispReel::put_Steps(float newVal)
{
	STARTUNDO
    m_d.m_motorsteps = max(1.0f,floorf(newVal));	// must have at least 1 step
	STOPUNDO

    return S_OK;
}

STDMETHODIMP DispReel::get_IsShading(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fShading);

	return S_OK;
}

STDMETHODIMP DispReel::put_IsShading(VARIANT_BOOL newVal)
{
	STARTUNDO
    m_d.m_fShading = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Type(ReelType *pVal)
{
    *pVal = m_d.m_reeltype;

	return S_OK;
}

STDMETHODIMP DispReel::put_Type(ReelType newVal)
{
	STARTUNDO
    m_d.m_reeltype = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Font(IFontDisp **pVal)
{
	m_pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);

	return S_OK;
}

STDMETHODIMP DispReel::put_Font(IFontDisp *newVal)
{
	// Does anybody use this way of setting the font?  Need to add to idl file.
	return S_OK;
}

STDMETHODIMP DispReel::putref_Font(IFontDisp* pFont)
{
	//We know that our own property browser gives us the same pointer

	//m_pIFont->Release();
	//pFont->QueryInterface(IID_IFont, (void **)&m_pIFont);

	SetDirtyDraw();
    return S_OK;
}

STDMETHODIMP DispReel::get_FontColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_fontcolor;

	return S_OK;
}

STDMETHODIMP DispReel::put_FontColor(OLE_COLOR newVal)
{
	STARTUNDO
	m_d.m_fontcolor = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_ReelColor(OLE_COLOR *pVal)
{
    *pVal = m_d.m_reelcolor;

	return S_OK;
}

STDMETHODIMP DispReel::put_ReelColor(OLE_COLOR newVal)
{
	STARTUNDO
    m_d.m_reelcolor = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP DispReel::get_Range(float *pVal)
{
    *pVal = (float)m_d.m_digitrange;

    return S_OK;
}

STDMETHODIMP DispReel::put_Range(float newVal)
{
	STARTUNDO

	m_d.m_digitrange = (int)max(0.0f,floorf(newVal));        // must have at least 1 digit (0 is a digit)
    if (m_d.m_digitrange > 512-1) m_d.m_digitrange = 512-1;  // and a max of 512 (0->511) //!! 512 requested by highrise

	STOPUNDO
	
	return S_OK;
}

STDMETHODIMP DispReel::get_UpdateInterval(long *pVal)
{
    *pVal = m_d.m_updateinterval;

    return S_OK;
}

STDMETHODIMP DispReel::put_UpdateInterval(long newVal)
{
	STARTUNDO
    
	m_d.m_updateinterval = max(5, newVal);
	if (g_pplayer)
		m_timenextupdate = g_pplayer->m_timeCur + m_d.m_updateinterval;
    
	STOPUNDO
	
	return S_OK;
}

STDMETHODIMP DispReel::get_UseImageGrid(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fUseImageGrid);

	return S_OK;
}

STDMETHODIMP DispReel::put_UseImageGrid(VARIANT_BOOL newVal)
{
	STARTUNDO
    m_d.m_fUseImageGrid = VBTOF(newVal);
	STOPUNDO

	return S_OK;
}
STDMETHODIMP DispReel::get_ImagesPerGridRow(long *pVal)
{
    *pVal = m_d.m_imagesPerGridRow;

    return S_OK;
}

STDMETHODIMP DispReel::put_ImagesPerGridRow(long newVal)
{
	STARTUNDO
    m_d.m_imagesPerGridRow = max(1, newVal);
	STOPUNDO
	
	return S_OK;
}


// function Methods available for the scripters.
//
STDMETHODIMP DispReel::AddValue(long Value)
{
	const bool bNegative = (Value < 0);

	// ensure a positive number
	long val = labs(Value);

	// get the base of this reel
	const long valbase = m_d.m_digitrange+1;

	// start at the right most reel and move left
	int i = m_d.m_reelcount-1;
	while ( (val != 0) && (i >= 0) )
	{
		const int digitValue = val % valbase;
		// remove the value for this reel from the overall number
		val /= valbase;

		if (bNegative)
			ReelInfo[i].motorPulses -= digitValue;
		else
			ReelInfo[i].motorPulses += digitValue;		
		
		// move to next reel
		i--;
	}

	return S_OK;
}


STDMETHODIMP DispReel::SetValue(long Value)
{
	// ensure a positive number
	long val = labs(Value);

	// get the base of this reel
	const long valbase = m_d.m_digitrange+1;

    // reset the motor
    for (int l=0; l<m_d.m_reelcount; ++l)
    {
		ReelInfo[l].currentValue	= 0;
        ReelInfo[l].motorPulses 	= 0;
        ReelInfo[l].motorStepCount	= 0;
        ReelInfo[l].motorCalcStep	= 0;
        ReelInfo[l].motorOffset		= 0;
	}

    // set the reel values (startint at the right most reel and move left)
	int i = m_d.m_reelcount-1;
	while ( (val != 0) && (i >= 0) )
	{
		const int digitValue = val % valbase;
		// remove the value for this reel from the overall number
		val /= valbase;
        ReelInfo[i].currentValue = digitValue;		
		// move to next reel
		i--;
    }

    // force a immediate screen update
    m_fforceupdate = true;
	m_timenextupdate = g_pplayer->m_timeCur;

    return S_OK;
}


STDMETHODIMP DispReel::ResetToZero()
{
    int carry = 0;
    const int overflowValue = m_d.m_digitrange+1;

    // work for the last reel to the first one
    for (int i=m_d.m_reelcount-1; i>=0; i--)
    {
		const int adjust = overflowValue - carry - ReelInfo[i].currentValue;
        carry = 0;

		if (adjust != overflowValue)
		{
			// overwrite the pulse count with the adjust value
			ReelInfo[i].motorPulses = adjust;
			// as this reel returns to zero it will roll over the next reel along
			carry = 1;
		}
    }

	return S_OK;
}


STDMETHODIMP DispReel::SpinReel(long ReelNumber, long PulseCount)
{
	if ( (ReelNumber >= 1) && (ReelNumber <= m_d.m_reelcount) )
	{
		const int reel = ReelNumber-1;
		ReelInfo[reel].motorPulses += PulseCount;
		return S_OK;
	}
	else
		return E_FAIL;
}


// Private functions
//
float DispReel::getBoxWidth() const
{
    const float width =  (float)m_d.m_reelcount * m_d.m_width
					   + (float)m_d.m_reelcount * m_d.m_reelspacing
					   + m_d.m_reelspacing;	// spacing also includes edges
    return width;
}


float DispReel::getBoxHeight() const
{
    const float height = m_d.m_height
					   + m_d.m_reelspacing + m_d.m_reelspacing; // spacing also includes edges

    return height;
}


// This function renders the in-game object which is then drawn onto the screen (in Draw3D)
//
// It firsts draws the object box (either solid or transparent) and draws the reels within it
//
void DispReel::UpdateObjFrame()
{
    RECT    reelstriprc;
	Pin3D	* const ppin3d = &g_pplayer->m_pin3d;
	DDBLTFX	bltFx;
	DWORD	flags;

	if( !GetPTable()->GetEMReelsEnabled() ) return;

	// is the background box transparent?
#if 1
    if (m_d.m_fTransparent)
    {
        // yes, then copy the current backgrount into the object frame
        m_pobjframe->pdds->Blt(NULL, ppin3d->m_pddsStatic, &m_pobjframe->rc, DDBLT_WAIT, NULL);
    }
    else
    {
        // nope, fill the box with the background colour
        HDC hdc;
        m_pobjframe->pdds->GetDC(&hdc);
        HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
        HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
        PatBlt(hdc, 0, 0, m_pobjframe->rc.right - m_pobjframe->rc.left, m_pobjframe->rc.bottom - m_pobjframe->rc.top, PATCOPY);
        SelectObject(hdc, hbrushold);
        DeleteObject(hbrush);
        m_pobjframe->pdds->ReleaseDC(hdc);
    }
#endif

    // render the reels onto the screen (providing the reel generation worked)

    if (m_pobjframe->pdds && (m_vreelframe.Size() > 0))
    {
		ZeroMemory(&bltFx, sizeof(bltFx));
		bltFx.dwSize = sizeof(bltFx);
		flags = DDBLTFAST_WAIT;

		if (m_d.m_reeltype == ReelImage)
		{
			bltFx.ddckSrcColorkey.dwColorSpaceLowValue  = m_rgbImageTransparent;
			bltFx.ddckSrcColorkey.dwColorSpaceHighValue = m_rgbImageTransparent;
			flags |= DDBLT_KEYSRCOVERRIDE;
		}

        reelstriprc.left  = 0;
	    reelstriprc.right = m_renderwidth;//m_reeldigitwidth;
        for (int i=0; i<m_d.m_reelcount; ++i)
        {
            reelstriprc.top = /*(ReelInfo[i].currentValue * m_renderheight) +*/ (int)(ReelInfo[i].motorOffset);
			if (reelstriprc.top < 0)
			{
				reelstriprc.top += m_renderheight/*m_reeldigitheight*/ * (m_d.m_digitrange+1);
			}
			reelstriprc.bottom = /*reelstriprc.top +*/ m_renderheight/*m_reeldigitheight*/;

    		// Set the color key for this bitmap (black)

#if 0
            m_pobjframe->pdds->Blt(&ReelInfo[i].position,   // destination rectangle
                                   m_preelframe->pdds,      // source image (LPDIRECTDRAWSURFACE7)
                                   &reelstriprc,            // source rectangle
                                   flags,
                                   &bltFx);            // no blit effects ;-(
#else
			m_pobjframe->pdds->BltFast(ReelInfo[i].position.left, ReelInfo[i].position.top, m_vreelframe.ElementAt(ReelInfo[i].currentValue)->pdds,
						&reelstriprc, DDBLTFAST_NOCOLORKEY/*DDBLTFAST_SRCCOLORKEY*/);
#endif

			if (ReelInfo[i].motorOffset != 0.0f)
				{
				const int nextval = (ReelInfo[i].currentValue + 1) % m_vreelframe.Size();
				const int top = ReelInfo[i].position.top + (reelstriprc.bottom - reelstriprc.top);
				reelstriprc.top = 0;
				reelstriprc.bottom = (int)ReelInfo[i].motorOffset;
				
				m_pobjframe->pdds->BltFast(ReelInfo[i].position.left, top, m_vreelframe.ElementAt(nextval)->pdds,
						&reelstriprc, DDBLTFAST_NOCOLORKEY/*DDBLTFAST_SRCCOLORKEY*/);
				}
        }
    }
    // objframe is now upto date
}
