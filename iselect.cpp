#include "StdAfx.h"

// BUG - this table must be kept in sync with the ISelectable ItemTypeEnum table
int rgTypeStringIndex[] = {
	IDS_TB_WALL, //eItemSurface,
	IDS_TB_FLIPPER, //eItemFlipper,
	IDS_TB_TIMER, //eItemTimer,
	IDS_TB_PLUNGER, //eItemPlunger,
	IDS_TB_TEXTBOX, //eItemTextbox,
	IDS_TB_BUMPER, //eItemBumper,
	IDS_TB_TRIGGER, //eItemTrigger,
	IDS_TB_LIGHT, //eItemLight,
	IDS_TB_KICKER, //eItemKicker,
	IDS_TB_DECAL, //eItemDecal,
	IDS_TB_GATE, //eItemGate,
	IDS_TB_SPINNER, //eItemSpinner,
	IDS_TB_RAMP, //eItemRamp,
	IDS_TABLE, //eItemTable,
	IDS_TB_LIGHT, //eItemLightCenter,
	IDS_CONTROLPOINT, //eItemDragPoint,
	IDS_COLLECTION, //eItemCollection,
    IDS_TB_DISPREEL, //eItemDispReel,
	IDS_TB_LIGHTSEQ, //eItemLightSeq,
	IDS_TB_PRIMITIVE, //eItemPrimitive	
	IDS_TB_LIGHTSEQ, //light seq center
	IDS_TB_COMCONTROL, //eItemComControl
    };

ISelect::ISelect()
	{
	m_fDragging = fFalse;
	m_fMarkedForUndo = fFalse;
	m_selectstate = eNotSelected;

	m_fLocked = fFalse;

	m_menuid = -1;
	}

void ISelect::SetObjectPos()
	{
	g_pvp->ClearObjectPosCur();
	}

void ISelect::OnLButtonDown(int x, int y)
	{
	m_fDragging = fTrue;
	m_fMarkedForUndo = fFalse; // So we will be marked when and if we are dragged
	m_ptLast.x = x;
	m_ptLast.y = y;

	SetCapture(GetPTable()->m_hwnd);

	SetObjectPos();
	}

void ISelect::OnLButtonUp(int x, int y)
	{
	m_fDragging = fFalse;

	ReleaseCapture();

	if (m_fMarkedForUndo)
		{
		m_fMarkedForUndo = fFalse;
		GetIEditable()->EndUndo();
		}
	}

void ISelect::OnRButtonDown(int x, int y, HWND hwnd)
	{
	}

void ISelect::OnRButtonUp(int x, int y)
	{
	}

void ISelect::OnMouseMove(int x, int y)
	{
	PinTable * const ptable = GetPTable();
	const float inv_zoom = 1.0f/ptable->m_zoom;

	if ((x == m_ptLast.x) && (y == m_ptLast.y))
		{
		return;
		}

	if (m_fDragging && !GetIEditable()->GetISelect()->m_fLocked) // For drag points, follow the lock of the parent
		{
		if (!m_fMarkedForUndo)
			{
			m_fMarkedForUndo = fTrue;
			GetIEditable()->BeginUndo();
			GetIEditable()->MarkForUndo();
			}
		MoveOffset((x - m_ptLast.x)*inv_zoom, (y - m_ptLast.y)*inv_zoom);
		m_ptLast.x = x;
		m_ptLast.y = y;
		SetObjectPos();
		}
	}

void ISelect::MoveOffset(const float dx, const float dy)
	{
	// Implement in child class to enable dragging
	}

void ISelect::EditMenu(HMENU hmenu)
	{
	}

void ISelect::DoCommand(int icmd, int x, int y)
	{
	IEditable *piedit = GetIEditable();

	switch (icmd)
		{
		case ID_DRAWINFRONT:
			GetPTable()->m_vedit.RemoveElement(piedit);
			GetPTable()->m_vedit.AddElement(piedit);
			GetPTable()->SetDirtyDraw();
			break;
		case ID_DRAWINBACK:
			GetPTable()->m_vedit.RemoveElement(piedit);
			GetPTable()->m_vedit.InsertElementAt(piedit, 0);
			GetPTable()->SetDirtyDraw();
			break;
		case ID_SETASDEFAULT:
			piedit->WriteRegDefaults();
			break;
		case ID_LOCK:
			GetIEditable()->BeginUndo();
			GetIEditable()->MarkForUndo();
			m_fLocked = !m_fLocked;
			GetIEditable()->EndUndo();
			GetPTable()->SetDirtyDraw();
			break;
      case ID_WALLMENU_HIDE:
         GetPTable()->m_vedit.RemoveElement(piedit);
         GetPTable()->hiddenObjects.AddElement(piedit);
         GetPTable()->SetDirtyDraw();
         break;
      case ID_WALLMENU_UNHIDEALL:
         for( int i=GetPTable()->hiddenObjects.Size()-1;i>=0;i-- )
         {
            piedit = GetPTable()->hiddenObjects.ElementAt(i);
            GetPTable()->m_vedit.AddElement(piedit);
         }
         GetPTable()->hiddenObjects.RemoveAllElements();
         GetPTable()->SetDirtyDraw();
         break;
		/*default:
			psel->DoCommand(command, x, y);
			break;*/
		}
	}

#define COLOR_LOCKED RGB(160,160,160)
//GetSysColor(COLOR_GRAYTEXT)

void ISelect::SetSelectFormat(Sur *psur)
	{
	//psur->SetFillColor(RGB(150,200,255));
	//psur->SetFillColor(RGB(128,0,0));

	DWORD color;

	if (m_fLocked)
		{
		//color = GetSysColor(COLOR_GRAYTEXT);
		color = COLOR_LOCKED;
		}
	else
		{
		color = GetSysColor(COLOR_HIGHLIGHT);
		}

	psur->SetBorderColor(color, false, 4);
	psur->SetLineColor(color, false, 4);
	}

void ISelect::SetMultiSelectFormat(Sur *psur)
	{
	DWORD color;

	if (m_fLocked)
		{
		color = COLOR_LOCKED;
		}
	else
		{
		color = GetSysColor(COLOR_HIGHLIGHT);
		}

	psur->SetBorderColor(color, false, 3);
	psur->SetLineColor(color, false, 3);
	}

void ISelect::SetLockedFormat(Sur *psur)
	{
	psur->SetBorderColor(COLOR_LOCKED, false, 1);
	psur->SetLineColor(COLOR_LOCKED, false, 1);
	}

void ISelect::FlipY(Vertex2D * const pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex2D vCenter;
	GetCenter(&vCenter);
	const float delta = vCenter.y - pvCenter->y;
	vCenter.y -= delta*2;
	PutCenter(&vCenter);
	}

void ISelect::FlipX(Vertex2D * const pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex2D vCenter;
	GetCenter(&vCenter);
	const float delta = vCenter.x - pvCenter->x;
	vCenter.x -= delta*2;
	PutCenter(&vCenter);
	}

void ISelect::Rotate(float ang, Vertex2D *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex2D vCenter;
	GetCenter(&vCenter);

	const float sn = sinf(ANGTORAD(ang));
	const float cs = cosf(ANGTORAD(ang));

	const float dx = vCenter.x - pvCenter->x;
	const float dy = vCenter.y - pvCenter->y;

	vCenter.x = pvCenter->x + cs*dx - sn*dy;
	vCenter.y = pvCenter->y + cs*dy + sn*dx;
	PutCenter(&vCenter);
	}

void ISelect::Scale(float scalex, float scaley, Vertex2D *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex2D vCenter;
	GetCenter(&vCenter);

	const float dx = vCenter.x - pvCenter->x;
	const float dy = vCenter.y - pvCenter->y;

	vCenter.x = pvCenter->x + dx*scalex;
	vCenter.y = pvCenter->y + dy*scaley;
	PutCenter(&vCenter);
	}

void ISelect::Translate(Vertex2D *pvOffset)
	{
	GetIEditable()->MarkForUndo();

	Vertex2D vCenter;
	GetCenter(&vCenter);

	vCenter.x += pvOffset->x;
	vCenter.y += pvOffset->y;
	PutCenter(&vCenter);
	}

void ISelect::GetCenter(Vertex2D * const pv) const
	{
	}

void ISelect::PutCenter(const Vertex2D * const pv)
	{
	}

HRESULT ISelect::GetTypeName(BSTR *pVal)
	{
	WCHAR wzName[128];

	LocalString ls(rgTypeStringIndex[GetItemType()]);

	MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzName, 128);

	*pVal = SysAllocString(wzName);

	return S_OK;
	}
BOOL ISelect::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(LOCK))
		{
		pbr->GetBool(&m_fLocked);
		}

	return fTrue;
	}

HRESULT ISelect::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

	bw.WriteBool(FID(LOCK), m_fLocked);

	return S_OK;
	}
