// Textbox.cpp : Implementation of CVBATestApp and DLL registration.

#include "StdAfx.h"

/////////////////////////////////////////////////////////////////////////////
//

Textbox::Textbox()
	{
	m_pIFont = NULL;
	m_ptu = NULL;
	}

Textbox::~Textbox()
	{
	m_pIFont->Release();
	}

HRESULT Textbox::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
	{
	m_ptable = ptable;

	HRESULT hr;
	float fTmp, width, height;

	hr = GetRegStringAsFloat("DefaultProps\\TextBox","Width", &fTmp);
	if (hr == S_OK)
		width = fTmp;
	else
		width = 100.0f;

	hr = GetRegStringAsFloat("DefaultProps\\TextBox","Height", &fTmp);
	if (hr == S_OK)
		height = fTmp;
	else
		height = 50.0f;

	m_d.m_v1.x = x;
	m_d.m_v1.y = y;
	m_d.m_v2.x = x+width;
	m_d.m_v2.y = y+height;

	m_pobjframe = NULL;

	SetDefaults(fromMouseClick);

	return InitVBA(fTrue, 0, NULL);//ApcProjectItem.Define(ptable->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, L"Textbox", NULL);
	}

void Textbox::SetDefaults(bool fromMouseClick)
	{
	float fTmp;
	//Textbox is always located on backdrop
	m_fBackglass = fTrue;

	HRESULT hr;
	int iTmp;

	hr = GetRegInt("DefaultProps\\TextBox","BackColor", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_backcolor = iTmp;
	else
		m_d.m_backcolor = RGB(0,0,0);
    
	hr = GetRegInt("DefaultProps\\TextBox","FontColor", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fontcolor = iTmp;
	else
		m_d.m_fontcolor = RGB(255,255,255);

	hr = GetRegInt("DefaultProps\\TextBox","TimerEnabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
	else
		m_d.m_tdr.m_fTimerEnabled = false;
	
	hr = GetRegInt("DefaultProps\\TextBox","TimerInterval", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_TimerInterval = iTmp;
	else
		m_d.m_tdr.m_TimerInterval = 100;

	hr = GetRegInt("DefaultProps\\TextBox","TextAlignment", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_talign = (enum TextAlignment)iTmp;
	else	
		m_d.m_talign = TextAlignRight;

	hr = GetRegInt("DefaultProps\\TextBox","Transparent", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fTransparent = iTmp == 0 ? false : true;
	else	
		m_d.m_fTransparent = fFalse;

	FONTDESC fd;

	fd.cbSizeofstruct = sizeof(FONTDESC);

	hr = GetRegStringAsFloat("DefaultProps\\TextBox","FontSize", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.cySize.int64 = (LONGLONG)(fTmp * 10000.0);
	else	
		fd.cySize.int64 = 142500;

	char tmp[256];
	hr = GetRegString("DefaultProps\\TextBox","FontName", tmp, 256);
	if ((hr != S_OK) || !fromMouseClick)
		fd.lpstrName = L"Arial";
	else
	{
		unsigned int len = strlen(&tmp[0]);
		fd.lpstrName = (LPOLESTR) malloc(len*sizeof(WCHAR));
		UNICODE_FROM_ANSI(fd.lpstrName, &tmp[0], len); 
		fd.lpstrName[len] = 0;
	}

	hr = GetRegInt("DefaultProps\\TextBox", "FontWeight", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.sWeight = (SHORT)iTmp;
	else
		fd.sWeight = FW_NORMAL;

	hr = GetRegInt("DefaultProps\\TextBox", "FontCharSet", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.sCharset = (SHORT)iTmp;
	else
		fd.sCharset = 0;
	
	hr = GetRegInt("DefaultProps\\TextBox", "FontItalic", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.fItalic = iTmp == 0 ? false : true;
	else
		fd.fItalic = 0;

	hr = GetRegInt("DefaultProps\\TextBox", "FontUnderline", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.fUnderline = iTmp == 0 ? false : true;
	else
		fd.fUnderline = 0;
	
	hr = GetRegInt("DefaultProps\\TextBox", "FontStrikeThrough", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		fd.fStrikethrough = iTmp == 0 ? false : true;
	else
		fd.fStrikethrough = 0;

	hr = GetRegString("DefaultProps\\TextBox","Text", m_d.sztext, MAXSTRING);
	if ((hr != S_OK) || !fromMouseClick)
		lstrcpy(m_d.sztext,"0");

	OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
	}

void Textbox::WriteRegDefaults()
	{
	char strTmp[40];
	float fTmp;
	
	SetRegValue("DefaultProps\\TextBox","BackColor", REG_DWORD, &m_d.m_backcolor, 4);
	SetRegValue("DefaultProps\\TextBox","FontColor", REG_DWORD, &m_d.m_fontcolor, 4);
	SetRegValue("DefaultProps\\TextBox","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\TextBox","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
	SetRegValue("DefaultProps\\TextBox","FontColor", REG_DWORD, &m_d.m_fontcolor, 4);
	SetRegValue("DefaultProps\\TextBox","Transparent",REG_DWORD,&m_d.m_fTransparent,4);

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
	sprintf_s(strTmp, 40, "%f", fTmp);
	SetRegValue("DefaultProps\\TextBox","FontSize", REG_SZ, &strTmp,strlen(strTmp));
	int charCnt = wcslen(fd.lpstrName) +1;
	WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, charCnt, strTmp, 2*charCnt, NULL, NULL);
	SetRegValue("DefaultProps\\TextBox","FontName", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\TextBox","FontWeight",REG_DWORD,&fd.sWeight,4);
	SetRegValue("DefaultProps\\TextBox","FontCharSet",REG_DWORD,&fd.sCharset,4);
	SetRegValue("DefaultProps\\TextBox","FontItalic",REG_DWORD,&fd.fItalic,4);
	SetRegValue("DefaultProps\\TextBox","FontUnderline",REG_DWORD,&fd.fUnderline,4);
	SetRegValue("DefaultProps\\TextBox","FontStrikeThrough",REG_DWORD,&fd.fStrikethrough,4);
	SetRegValue("DefaultProps\\TextBox","Text", REG_SZ, &m_d.sztext,strlen(m_d.sztext));
	}
STDMETHODIMP Textbox::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ITextbox,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

void Textbox::PreRender(Sur * const psur)
	{
	psur->SetBorderColor(-1,false,0);
	psur->SetFillColor(m_d.m_backcolor);
	psur->SetObject(this);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
	}

void Textbox::Render(Sur * const psur)
	{
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);
	psur->SetObject(NULL);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
	}

void Textbox::GetTimers(Vector<HitTimer> * const pvht)
	{
	IEditable::BeginPlay();

	HitTimer *pht;
	pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
		{
		pvht->AddElement(pht);
		}
	}

void Textbox::GetHitShapes(Vector<HitObject> * const pvho)
	{
	m_ptu = new TextboxUpdater(this);

	m_ptu->m_textboxanim.m_znear = 0;
	m_ptu->m_textboxanim.m_zfar = 0;

	// HACK - adding object directly to screen update list.  Someday make hit objects and screenupdaters seperate objects
	g_pplayer->m_vscreenupdate.AddElement(&m_ptu->m_textboxanim);
	}

void Textbox::GetHitShapesDebug(Vector<HitObject> * const pvho)
	{
	}

void Textbox::EndPlay()
	{

	if (m_pobjframe) // Failed Player case
		{
		delete m_pobjframe;
		m_pobjframe = NULL;

		m_pIFontPlay->Release();
		}

	if (m_ptu)
		{
		delete m_ptu;
		m_ptu = NULL;
		}

	IEditable::EndPlay();
	}

void Textbox::PostRenderStatic(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Textbox::RenderStatic(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}
	
void Textbox::RenderMovers(const LPDIRECT3DDEVICE7 pd3dDevice)
	{
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;

	m_pobjframe = new ObjFrame();

	Vertex3D rgv3D[2];
	rgv3D[0].x = m_d.m_v1.x;
	rgv3D[0].y = m_d.m_v1.y;
	rgv3D[1].x = m_d.m_v2.x;
	rgv3D[1].y = m_d.m_v2.y;

	SetHUDVertices(rgv3D, 2);

	m_pobjframe->rc.left = (int)min(rgv3D[0].x, rgv3D[1].x);
	m_pobjframe->rc.top = (int)min(rgv3D[0].y, rgv3D[1].y);
	m_pobjframe->rc.right = (int)max(rgv3D[0].x, rgv3D[1].x);
	m_pobjframe->rc.bottom = (int)max(rgv3D[0].y, rgv3D[1].y);

	m_ptu->m_textboxanim.m_rcBounds = m_pobjframe->rc;

	m_pobjframe->pdds = ppin3d->CreateOffscreen(m_pobjframe->rc.right - m_pobjframe->rc.left, m_pobjframe->rc.bottom - m_pobjframe->rc.top);

	m_pIFont->Clone(&m_pIFontPlay);

	CY size;
	m_pIFontPlay->get_Size(&size);
	// I choose 912 because that was the original playing size I tested with,
	// and this way I don't have to change my tables
	size.int64 = size.int64 / 912 * ppin3d->m_dwRenderWidth;
	m_pIFontPlay->put_Size(size);
	}

void Textbox::RenderText()
	{
	//DDBLTFX ddbfx;
	Pin3D *const ppin3d = &g_pplayer->m_pin3d;

	HDC hdc;

	if (m_d.m_fTransparent)
		{
		m_pobjframe->pdds->Blt(NULL, ppin3d->m_pddsStatic, &m_pobjframe->rc, DDBLT_WAIT, NULL);
		m_pobjframe->pdds->GetDC(&hdc);
		}
	else
		{
		m_pobjframe->pdds->GetDC(&hdc);

		HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
		HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);

		PatBlt(hdc, 0, 0, m_pobjframe->rc.right - m_pobjframe->rc.left, m_pobjframe->rc.bottom - m_pobjframe->rc.top, PATCOPY);

		SelectObject(hdc, hbrushold);

		DeleteObject(hbrush);
		}

	HFONT hFont;
	m_pIFontPlay->get_hFont(&hFont);
	if (g_pplayer->m_frotate)
		{
		LOGFONT lf;
		GetObject(hFont, sizeof(LOGFONT), &lf);
		lf.lfOrientation = 900;
		lf.lfEscapement = 900;
		HFONT hfontNew = CreateFontIndirect(&lf);
		SelectObject(hdc, hfontNew);
		}
	else
		{
		SelectObject(hdc, hFont);
		}

	SetTextColor(hdc, m_d.m_fontcolor);

	SetBkMode(hdc, TRANSPARENT);

	//RECT rc = {4,4,m_pobjframe->rc.right - m_pobjframe->rc.left - 8, m_pobjframe->rc.bottom - m_pobjframe->rc.top - 8};

	//SetTextAlign(hdc, TA_RIGHT);

	SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

	if (m_d.sztext)
		{
		//ExtTextOut(hdc, m_pobjframe->rc.right - m_pobjframe->rc.left - 4, 4, 0, NULL, m_d.sztext, lstrlen(m_d.sztext), NULL);

		int alignment;
		switch (m_d.m_talign)
			{
			case TextAlignLeft:
				alignment = DT_LEFT;
				break;

			default:
			case TextAlignCenter:
				alignment = DT_CENTER;
				break;

			case TextAlignRight:
				alignment = DT_RIGHT;
				break;
			}

		int border = (4 * ppin3d->m_dwRenderWidth) / 1000;
		RECT rcOut;

		rcOut.left = border;
		rcOut.top = border;
		rcOut.right = m_pobjframe->rc.right - m_pobjframe->rc.left - border * 2;
		rcOut.bottom = m_pobjframe->rc.bottom - m_pobjframe->rc.top - border * 2;

		DrawText(hdc, m_d.sztext, lstrlen(m_d.sztext), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
		}

	m_pobjframe->pdds->ReleaseDC(hdc);

	ppin3d->m_pddsBackBuffer->Blt(&m_pobjframe->rc, m_pobjframe->pdds, NULL, DDBLT_WAIT, NULL);

	UpdateRect *pur = new UpdateRect();
	pur->m_rcupdate = m_pobjframe->rc;
	pur->m_fSeeThrough = fFalse;
	g_pplayer->m_vupdaterect.AddElement(pur);
	}

void Textbox::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
	}

void Textbox::MoveOffset(const float dx, const float dy)
	{
	m_d.m_v1.x += dx;
	m_d.m_v1.y += dy;

	m_d.m_v2.x += dx;
	m_d.m_v2.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Textbox::GetCenter(Vertex2D * const pv) const
{
	*pv = m_d.m_v1;
}

void Textbox::PutCenter(const Vertex2D * const pv)
{
	m_d.m_v2.x = pv->x + m_d.m_v2.x - m_d.m_v1.x;
	m_d.m_v2.y = pv->y + m_d.m_v2.y - m_d.m_v1.y;

	m_d.m_v1 = *pv;

	m_ptable->SetDirtyDraw();
}

STDMETHODIMP Textbox::get_BackColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_backcolor;

	return S_OK;
}

STDMETHODIMP Textbox::put_BackColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_backcolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_FontColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_fontcolor;

	return S_OK;
}

STDMETHODIMP Textbox::put_FontColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_fontcolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Text(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, (char *)m_d.sztext, -1, wz, 512);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Textbox::put_Text(BSTR newVal)
{
	if (lstrlenW(newVal) < 512)
		{
		STARTUNDO

		WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.sztext, 512, NULL, NULL);

		if (m_pobjframe)
			{
			RenderText();
			}

		STOPUNDO
		}

	return S_OK;
}

HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
	bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
	bw.WriteInt(FID(CLRB), m_d.m_backcolor);
	bw.WriteInt(FID(CLRF), m_d.m_fontcolor);
	bw.WriteString(FID(TEXT), m_d.sztext);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(ALGN), m_d.m_talign);
	bw.WriteBool(FID(TRNS), m_d.m_fTransparent);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(FONT));
	IPersistStream * ips;
	m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
	HRESULT hr;
	hr = ips->Save(pstm, TRUE);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Textbox::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
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

	if(FAILED(hr = pstm->Read(&m_d, sizeof(TextboxData), &read)))
		return hr;

	FONTDESC fd;
	fd.cbSizeofstruct = sizeof(FONTDESC);
	fd.lpstrName = L"Arial";
	fd.cySize.int64 = 142500;
	//fd.cySize.Lo = 0;
	fd.sWeight = FW_NORMAL;
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

BOOL Textbox::LoadToken(int id, BiffReader *pbr)
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
	else if (id == FID(CLRB))
		{
		pbr->GetInt(&m_d.m_backcolor);
//		if (!(m_d.m_backcolor & MINBLACKMASK)) {m_d.m_backcolor |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(CLRF))
		{
		pbr->GetInt(&m_d.m_fontcolor);
//		if (!(m_d.m_fontcolor & MINBLACKMASK)) {m_d.m_fontcolor |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(TEXT))
		{
		pbr->GetString(m_d.sztext);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(ALGN))
		{
		pbr->GetInt(&m_d.m_talign);
		}
	else if (id == FID(TRNS))
		{
		pbr->GetBool(&m_d.m_fTransparent);
		}
	else if (id == FID(FONT))
		{
		if (!m_pIFont)
			{
			FONTDESC fd;
			fd.cbSizeofstruct = sizeof(FONTDESC);
			fd.lpstrName = L"Arial";
			fd.cySize.int64 = 142500;
			//fd.cySize.Lo = 0;
			fd.sWeight = FW_NORMAL;
			fd.sCharset = 0;
			fd.fItalic = 0;
			fd.fUnderline = 0;
			fd.fStrikethrough = 0;
			OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
			}

		IPersistStream * ips;
		m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

		HRESULT hr;
		hr = ips->Load(pbr->m_pistream);
		hr = 9;
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Textbox::InitPostLoad()
	{
	m_pobjframe = NULL;

	return S_OK;
	}

STDMETHODIMP Textbox::get_Font(IFontDisp **pVal)
{
	m_pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);

	return S_OK;
}

STDMETHODIMP Textbox::put_Font(IFontDisp *newVal)
{
	// Does anybody use this way of setting the font?  Need to add to idl file.
	return S_OK;
}

STDMETHODIMP Textbox::putref_Font(IFontDisp* pFont)
	{
	//We know that our own property browser gives us the same pointer

	//m_pIFont->Release();
	//pFont->QueryInterface(IID_IFont, (void **)&m_pIFont);

	SetDirtyDraw();
	return S_OK;
	}

STDMETHODIMP Textbox::get_Width(float *pVal)
{
	*pVal = m_d.m_v2.x - m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP Textbox::put_Width(float newVal)
{
	STARTUNDO

	m_d.m_v2.x = m_d.m_v1.x + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Height(float *pVal)
{
	*pVal = m_d.m_v2.y - m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP Textbox::put_Height(float newVal)
{
	STARTUNDO

	m_d.m_v2.y = m_d.m_v1.y + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_X(float *pVal)
{
	*pVal = m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP Textbox::put_X(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.x;

	m_d.m_v1.x += delta;
	m_d.m_v2.x += delta;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Y(float *pVal)
{
	*pVal = m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP Textbox::put_Y(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.y;

	m_d.m_v1.y += delta;
	m_d.m_v2.y += delta;

	STOPUNDO

	return S_OK;
}

void Textbox::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_STATE, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Textbox::get_Alignment(TextAlignment *pVal)
{
	*pVal = m_d.m_talign;

	return S_OK;
}

STDMETHODIMP Textbox::put_Alignment(TextAlignment newVal)
{
	STARTUNDO

	m_d.m_talign = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_IsTransparent(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fTransparent);

	return S_OK;
}

STDMETHODIMP Textbox::put_IsTransparent(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fTransparent = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}
