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
**	RenderAnimation()
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
    vertexBuffer = NULL;
}


// Destructor
//
DispReel::~DispReel()
{
    m_pIFont->Release();
    if( vertexBuffer )
    {
        vertexBuffer->release();
        vertexBuffer=0;
    }
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
		int len = lstrlen(&tmp[0]) + 1;
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
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
	else
		m_d.m_tdr.m_fTimerEnabled = 0;
	
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
			int len = lstrlen(&tmp[0]) + 1;
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

	SetRegValue("DefaultProps\\EMReel","ReelType",REG_DWORD,&m_d.m_reeltype,4);
	SetRegValue("DefaultProps\\EMReel","Image", REG_SZ, &m_d.m_szImage,lstrlen(m_d.m_szImage));
	SetRegValue("DefaultProps\\EMReel","Sound", REG_SZ, &m_d.m_szSound,lstrlen(m_d.m_szSound));
	SetRegValueInt("DefaultProps\\Decal","UseImageGrid",m_d.m_fUseImageGrid);
	SetRegValueInt("DefaultProps\\Decal","ImagesPerRow",m_d.m_imagesPerGridRow);
	SetRegValueInt("DefaultProps\\Decal","Transparent",m_d.m_fTransparent);
	SetRegValueInt("DefaultProps\\Decal","ReelCount",m_d.m_reelcount);
	SetRegValueFloat("DefaultProps\\EMReel","Width", m_d.m_width);
	SetRegValueFloat("DefaultProps\\EMReel","Height", m_d.m_height);
	SetRegValueFloat("DefaultProps\\EMReel","ReelSpacing", m_d.m_reelspacing);
	SetRegValueFloat("DefaultProps\\EMReel","MotorSteps", m_d.m_motorsteps);
	SetRegValueInt("DefaultProps\\Decal","DigitRange",m_d.m_digitrange);
	SetRegValueInt("DefaultProps\\Decal","UpdateInterval",m_d.m_updateinterval);
	SetRegValue("DefaultProps\\EMReel","BackColor",REG_DWORD,&m_d.m_backcolor,4);
	SetRegValue("DefaultProps\\EMReel","FontColor",REG_DWORD,&m_d.m_fontcolor,4);
	SetRegValue("DefaultProps\\EMReel","ReelColor",REG_DWORD,&m_d.m_reelcolor,4);
	SetRegValueInt("DefaultProps\\EMReel","TimerEnabled",m_d.m_tdr.m_fTimerEnabled);
	SetRegValueInt("DefaultProps\\EMReel","TimerInterval",m_d.m_tdr.m_TimerInterval);

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
		
		const float fTmp = (float)(fd.cySize.int64 / 10000.0);
		sprintf_s(strTmp, 40, "%f", fTmp);
		SetRegValue("DefaultProps\\EMReel","FontSize", REG_SZ, &strTmp,lstrlen(strTmp));
		size_t charCnt = wcslen(fd.lpstrName) + 1;
		WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, (int)charCnt, strTmp, (int)(2*charCnt), NULL, NULL);
		SetRegValue("DefaultProps\\EMReel","FontName", REG_SZ, &strTmp,lstrlen(strTmp));
		const int weight = fd.sWeight;
		const int charset = fd.sCharset;
		SetRegValueInt("DefaultProps\\EMReel","FontWeight",weight);
		SetRegValueInt("DefaultProps\\EMReel","FontCharSet",charset);
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

	for (size_t i=0; i<sizeof(arr)/sizeof(arr[0]); ++i)
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
void DispReel::PreRender(Sur * const psur)
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
        const float x = m_d.m_v1.x + fi*(m_d.m_width + m_d.m_reelspacing) + m_d.m_reelspacing;
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
void DispReel::Render(Sur * const psur)
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
        const float x = m_d.m_v1.x + fi*(m_d.m_width + m_d.m_reelspacing) + m_d.m_reelspacing;
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
void DispReel::GetTimers(Vector<HitTimer> * const pvht)
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
void DispReel::GetHitShapes(Vector<HitObject> * const pvho)
{
    m_ptu = new DispReelUpdater(this);

	// HACK - adding object directly to screen update list.  Someday make hit objects and screenupdaters seperate objects
	g_pplayer->m_vscreenupdate.AddElement(&m_ptu->m_dispreelanim);
}

void DispReel::GetHitShapesDebug(Vector<HitObject> * const pvho)
	{
	}

// This method is called as the game exits..
// it cleans up any allocated memory used by the instance of the object
//
void DispReel::EndPlay()
{
	if (m_ptu)
	{
		delete m_ptu;
		m_ptu = NULL;
	}

   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }

	IEditable::EndPlay();
}

void DispReel::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();
    if (!GetPTable()->GetEMReelsEnabled())
        return;

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

    // Set up the reel strip (either using bitmaps or fonts)
    if (m_d.m_reeltype == ReelImage)
    {
        // get a pointer to the image specified in the object
        Texture * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

        if (!pin)
            return;

        Material mat;
        mat.setColor( 1.0f, 1.0f, 1.0f, 1.0f );
        pd3dDevice->SetMaterial(mat);

        pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);

        // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
        pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_MIRROR);

        pin->CreateAlphaChannel();
        pin->Set( ePictureTexture );

        ppin3d->SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);

        pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
        pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, 0xe0);
        pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);

        ppin3d->DisableLightMap();

        Vertex3D_NoTex2 rgv3D[4];

        for (int i = 0; i < m_d.m_reelcount; ++i)
        {
            SetVerticesForReel(i, ReelInfo[i].currentValue, rgv3D);
            pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_NOTEX2_VERTEX, rgv3D, 4);
        }

        // reset render state

        pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);

        pd3dDevice->SetTexture(ePictureTexture, NULL);
        ppin3d->SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
        pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
        pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
        pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_WRAP);
    }
    else
    {
        // TODO: ReelText not supported yet
    }

    pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
}

void DispReel::RenderSetup(RenderDevice* pd3dDevice)
{
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    //if ( vertexBuffer==NULL )
    //    pd3dDevice->CreateVertexBuffer( 4, 0, MY_D3DTRANSFORMED_NOTEX2_VERTEX, &vertexBuffer );

    // get the render sizes of the objects (reels and frame)
    m_renderwidth  = max(0.0f, (m_d.m_width / (float)EDITOR_BG_WIDTH) * ppin3d->m_dwRenderWidth);
    m_renderheight = max(0.0f, (m_d.m_height / (float)EDITOR_BG_HEIGHT) * ppin3d->m_dwRenderHeight);
    const float m_renderspacingx = max(0.0f, (m_d.m_reelspacing / (float)EDITOR_BG_WIDTH) * ppin3d->m_dwRenderWidth);
    const float m_renderspacingy = max(0.0f, (m_d.m_reelspacing / (float)EDITOR_BG_HEIGHT)  * ppin3d->m_dwRenderHeight);

    // set up all the reel positions within the object frame
    const float x0 = (m_d.m_v1.x / (float)EDITOR_BG_WIDTH) * ppin3d->m_dwRenderWidth;
    const float y0 = (m_d.m_v1.y / (float)EDITOR_BG_HEIGHT) * ppin3d->m_dwRenderHeight;
    float x1 = x0 + m_renderspacingx;

    for (int i=0; i<m_d.m_reelcount; ++i)
    {
        ReelInfo[i].position.left	= x1;
        ReelInfo[i].position.right	= x1 + m_renderwidth;
        ReelInfo[i].position.top	= y0 + m_renderspacingy;
        ReelInfo[i].position.bottom	= y0 + m_renderspacingy + m_renderheight;

        ReelInfo[i].currentValue	= 0;
        ReelInfo[i].motorPulses		= 0;
        ReelInfo[i].motorStepCount	= 0;
        ReelInfo[i].motorCalcStep	= 0;
        ReelInfo[i].motorOffset		= 0;

        // move to the next reel
        x1 += m_renderspacingx + m_renderwidth;
    }

    // Set up the reel strip (either using bitmaps or fonts)
    if (m_d.m_reeltype == ReelImage)
    {
        // get a pointer to the image specified in the object
        Texture * const pin = m_ptable->GetImage(m_d.m_szImage); // pointer to image information from the image manager

        if (!pin)
            return;

        int	GridCols, GridRows;

        // get the number of images per row of the image
        if (m_d.m_fUseImageGrid)
        {
            GridCols = m_d.m_imagesPerGridRow;
            if (GridCols != 0) // best to be safe
            {
                GridRows = (m_d.m_digitrange+1) / GridCols;
                if ( (GridRows * GridCols) < (m_d.m_digitrange+1) )
                    ++GridRows;
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
        if ( GridCols!=0 && GridRows!=0 )
        {
            // get the size of the individual reel digits (if m_digitrange is wrong we can forget the rest)
            m_reeldigitwidth  = (float)pin->m_width / (float)GridCols;
            m_reeldigitheight = (float)pin->m_height / (float)GridRows;
        }
        else
        {
            ShowError("DispReel: GridCols/GridRows are zero!");
        }

        pin->CreateAlphaChannel();

        const float ratiox = (float)m_reeldigitwidth  / (float)pin->m_width;
        const float ratioy = (float)m_reeldigitheight / (float)pin->m_height;

        int gr = 0;
        int gc = 0;

        m_digitTexCoords.resize(m_d.m_digitrange + 1);

        for (int i=0; i<=m_d.m_digitrange; ++i)
        {
            m_digitTexCoords[i].u_min = (float)gc * ratiox;
            m_digitTexCoords[i].v_min = (float)gr * ratioy;
            m_digitTexCoords[i].u_max = m_digitTexCoords[i].u_min + ratiox;
            m_digitTexCoords[i].v_max = m_digitTexCoords[i].v_min + ratioy;

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
    }
    else
    {
        // TODO: ReelText not supported yet
    }

    m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;
    m_fforceupdate = false;
}

void DispReel::RenderStatic(RenderDevice* pd3dDevice)
{
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

    if (g_pplayer->m_time_msec >= m_timenextupdate)
	{
        m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;

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
		           m_ptable->PlaySound(mySound, 0, 1.0f, 0.f, 0.f, 0, 0, 1);
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
            m_fforceupdate = false;
            rc = true;
		}
	}

	return rc;
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


void DispReel::GetCenter(Vertex2D * const pv) const
{
	*pv = m_d.m_v1;
}


void DispReel::PutCenter(const Vertex2D * const pv)
{
	m_d.m_v1 = *pv;

	m_d.m_v2.x = pv->x + getBoxWidth();
	m_d.m_v2.y = pv->y + getBoxHeight();

	m_ptable->SetDirtyDraw();
}


HRESULT DispReel::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

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

	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
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
		m_timenextupdate = g_pplayer->m_time_msec + m_d.m_updateinterval;
    
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
	m_timenextupdate = g_pplayer->m_time_msec;

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


void DispReel::SetVerticesForReel(int reelNum, int digit, Vertex3D_NoTex2 * v)
{
    v[0].x = v[3].x = (float)ReelInfo[reelNum].position.left;
    v[1].x = v[2].x = (float)ReelInfo[reelNum].position.right;

    v[0].y = v[1].y = (float)ReelInfo[reelNum].position.top;
    v[2].y = v[3].y = (float)ReelInfo[reelNum].position.bottom;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;
    v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.0f;

    v[0].tu = v[3].tu = m_digitTexCoords[digit].u_min;
    v[0].tv = v[1].tv = m_digitTexCoords[digit].v_min;

    v[1].tu = v[2].tu = m_digitTexCoords[digit].u_max;
    v[2].tv = v[3].tv = m_digitTexCoords[digit].v_max;

    v[0].color = v[1].color = v[2].color = v[3].color = 0xffffffff;
}

