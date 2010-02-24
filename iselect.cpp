#include "stdafx.h"
#include "main.h"

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

/*int rgTypeStringIndexPlural[] = {
	IDS_TB_WALLS, //eItemSurface,
	IDS_TB_FLIPPERS, //eItemFlipper,
	IDS_TB_TIMERS, //eItemTimer,
	IDS_TB_PLUNGERS, //eItemPlunger,
	IDS_TB_TEXTBOXES, //eItemTextbox,
	IDS_TB_BUMPERS, //eItemBumper,
	IDS_TB_TRIGGERS, //eItemTrigger,
	IDS_TB_LIGHTS, //eItemLight,
	IDS_TB_KICKERS, //eItemKicker,
	IDS_TB_DECALS, //eItemDecal,
	IDS_TB_GATES, //eItemGate,
	IDS_TB_SPINNERS, //eItemSpinner,
	IDS_TB_RAMPS, //eItemRamp,
	IDS_TABLE, //eItemTable,
	IDS_TB_LIGHTS, //eItemLightCenter,
	IDS_CONTROLPOINTS, //eItemDragPoint,
	IDS_COLLECTIONS, //eItemCollection,
	};*/

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
	/*HMENU hmenumain;
	HMENU hmenu;
	POINT pt;
	int icmd;

	if (m_menuid == -1 && GetIEditable() == NULL)
		{
		return;
		}

	pt.x = x;
	pt.y = y;
	ClientToScreen(hwnd, &pt);

	if (m_menuid != -1)
		{
		hmenumain = LoadMenu(g_hinst, MAKEINTRESOURCE(m_menuid));

		hmenu = GetSubMenu(hmenumain, 0);
		}
	else
		{
		hmenu = CreatePopupMenu();
		}

	EditMenu(hmenu);

	if (GetIEditable() != NULL)
		{
		if (GetMenuItemCount(hmenu) > 0)
			{
			AppendMenu(hmenu, MF_SEPARATOR, -1, "");
			}
		AppendMenu(hmenu, MF_STRING, ID_DRAWINFRONT, "Draw In &Front");
		AppendMenu(hmenu, MF_STRING, ID_DRAWINBACK, "Draw In &Back");
		}

	icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD,
			pt.x, pt.y, hwnd, NULL);

	if (icmd != 0)
		{
		switch (icmd)
			{
			case ID_DRAWINFRONT:
				GetPTable()->m_vedit.RemoveElement(GetIEditable());
				GetPTable()->m_vedit.AddElement(GetIEditable());
				GetPTable()->SetDirtyDraw();
				break;
			case ID_DRAWINBACK:
				GetPTable()->m_vedit.RemoveElement(GetIEditable());
				GetPTable()->m_vedit.InsertElementAt(GetIEditable(), 0);
				GetPTable()->SetDirtyDraw();
				break;
			default:
				DoCommand(icmd, x, y);
				break;
			}
		}

	DestroyMenu(hmenu);

	if (m_menuid != -1)
		{
		DestroyMenu(hmenumain);
		}

	return;*/
	}

void ISelect::OnRButtonUp(int x, int y)
	{
	}

void ISelect::OnMouseMove(int x, int y)
	{
	PinTable *ptable = GetPTable();
	float zoom = ptable->m_zoom;

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

	float dx,dy;
	float sn,cs;
	float temp;

	Vertex vCenter;
	GetCenter(&vCenter);

	sn = (float)sin(ANGTORAD(ang));
	cs = (float)cos(ANGTORAD(ang));

	dx = vCenter.x - pvCenter->x;
	dy = vCenter.y - pvCenter->y;
	temp = dx;
	dx = cs*dx - sn*dy;
	dy = cs*dy + sn*temp;
	vCenter.x = pvCenter->x + dx;
	vCenter.y = pvCenter->y + dy;
	PutCenter(&vCenter);
	}

void ISelect::Scale(float scalex, float scaley, Vertex *pvCenter)
	{
	GetIEditable()->MarkForUndo();

	float dx,dy;

	Vertex vCenter;
	GetCenter(&vCenter);

	dx = vCenter.x - pvCenter->x;
	dy = vCenter.y - pvCenter->y;

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
