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
	PinTable *ptable = GetPTable();
	const float zoom = ptable->m_zoom;

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
		MoveOffset((x - m_ptLast.x)/zoom, (y - m_ptLast.y)/zoom);
		m_ptLast.x = x;
		m_ptLast.y = y;
		SetObjectPos();
		}
	}

void ISelect::MoveOffset(float dx, float dy)
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
		case ID_LOCK:
			GetIEditable()->BeginUndo();
			GetIEditable()->MarkForUndo();
			m_fLocked = m_fLocked ? fFalse : fTrue;
			GetIEditable()->EndUndo();
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

	psur->SetBorderColor(color, fFalse, 4);
	psur->SetLineColor(color, fFalse, 4);
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

	psur->SetBorderColor(color, fFalse, 3);
	psur->SetLineColor(color, fFalse, 3);
	}

void ISelect::SetLockedFormat(Sur *psur)
	{
	psur->SetBorderColor(COLOR_LOCKED, fFalse, 1);
	psur->SetLineColor(COLOR_LOCKED, fFalse, 1);
	}

void ISelect::FlipY(Vertex *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex vCenter;
	GetCenter(&vCenter);
	float delta = vCenter.y - pvCenter->y;
	vCenter.y -= delta*2;
	PutCenter(&vCenter);
	}

void ISelect::FlipX(Vertex *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex vCenter;
	GetCenter(&vCenter);
	float delta = vCenter.x - pvCenter->x;
	vCenter.x -= delta*2;
	PutCenter(&vCenter);
	}

void ISelect::Rotate(float ang, Vertex *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex vCenter;
	GetCenter(&vCenter);

	const float sn = sinf(ANGTORAD(ang));
	const float cs = cosf(ANGTORAD(ang));

	float dx = vCenter.x - pvCenter->x;
	float dy = vCenter.y - pvCenter->y;
	const float temp = dx;
	dx = cs*dx - sn*dy;
	dy = cs*dy + sn*temp;
	vCenter.x = pvCenter->x + dx;
	vCenter.y = pvCenter->y + dy;
	PutCenter(&vCenter);
	}

void ISelect::Scale(float scalex, float scaley, Vertex *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	Vertex vCenter;
	GetCenter(&vCenter);

	float dx = vCenter.x - pvCenter->x;
	float dy = vCenter.y - pvCenter->y;

	dx *= scalex;
	dy *= scaley;

	vCenter.x = pvCenter->x + dx;
	vCenter.y = pvCenter->y + dy;
	PutCenter(&vCenter);
	}

void ISelect::Translate(Vertex *pvOffset)
	{
	GetIEditable()->MarkForUndo();

	Vertex vCenter;
	GetCenter(&vCenter);

	vCenter.x += pvOffset->x;
	vCenter.y += pvOffset->y;
	PutCenter(&vCenter);
	}

void ISelect::GetCenter(Vertex *pv)
	{
	}

void ISelect::PutCenter(Vertex *pv)
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
