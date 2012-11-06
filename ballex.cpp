#include "stdafx.h"
#include "VBATest.h"

BallEx::BallEx()
	{
	m_pball = NULL;
	}

BallEx::~BallEx()
	{
	}

void BallEx::GetDebugCommands(VectorInt<int> *pvids, VectorInt<int> *pvcommandid)
	{
	pvids->AddElement(IDS_MAKEACTIVEBALL);
	pvcommandid->AddElement(0);
	}

void BallEx::RunDebugCommand(int id)
	{
	switch (id)
		{
		case 0:
			g_pplayer->m_pactiveballDebug = m_pball;
			break;
		}
	}

#define CHECKSTALEBALL if (!m_pball) {return E_POINTER;}

STDMETHODIMP BallEx::get_X(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->x;

	return S_OK;
}

STDMETHODIMP BallEx::put_X(float newVal)
{
	CHECKSTALEBALL

	m_pball->x = newVal;

	return S_OK;
}

STDMETHODIMP BallEx::get_Y(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->y;

	return S_OK;
}

STDMETHODIMP BallEx::put_Y(float newVal)
{
	CHECKSTALEBALL

	m_pball->y = newVal;

	return S_OK;
}

STDMETHODIMP BallEx::get_VelX(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->vx;

	return S_OK;
}

STDMETHODIMP BallEx::put_VelX(float newVal)
{
	CHECKSTALEBALL

	m_pball->vx = newVal;

	m_pball->CalcBoundingRect();

	return S_OK;
}

STDMETHODIMP BallEx::get_VelY(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->vy;

	return S_OK;
}

STDMETHODIMP BallEx::put_VelY(float newVal)
{
	CHECKSTALEBALL

	m_pball->vy = newVal;

	m_pball->CalcBoundingRect();

	return S_OK;
}

STDMETHODIMP BallEx::get_Z(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->z;

	return S_OK;
}

STDMETHODIMP BallEx::put_Z(float newVal)
{
	CHECKSTALEBALL

	m_pball->z = newVal;

	return S_OK;
}

STDMETHODIMP BallEx::get_VelZ(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->vz;

	return S_OK;
}

STDMETHODIMP BallEx::put_VelZ(float newVal)
{
	CHECKSTALEBALL

	m_pball->vz = newVal;

	m_pball->CalcBoundingRect();

	return S_OK;
}

STDMETHODIMP BallEx::get_Color(OLE_COLOR *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->m_color;

	return S_OK;
}

STDMETHODIMP BallEx::put_Color(OLE_COLOR newVal)
{
	CHECKSTALEBALL

	m_pball->m_color = newVal;

	return S_OK;
}

STDMETHODIMP BallEx::get_Image(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_pball->m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);
	return S_OK;
}

STDMETHODIMP BallEx::put_Image(BSTR newVal)
{
	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_pball->m_szImage, 32, NULL, NULL);

	m_pball->m_pin = (lstrlen(m_pball->m_szImage) > 0) ? g_pplayer->m_ptable->GetImage(m_pball->m_szImage) : NULL;

	return S_OK;
}

HRESULT BallEx::get_UserValue(VARIANT *pVal)
	{
	VariantClear(pVal);
	VariantCopy(pVal, &m_uservalue);
	return S_OK;
	}

HRESULT BallEx::put_UserValue(VARIANT *newVal)
	{
	VariantInit(&m_uservalue);
	VariantClear(&m_uservalue);
	/*const HRESULT hr =*/ VariantCopy(&m_uservalue, newVal);
	return S_OK;
	}


STDMETHODIMP BallEx::get_FrontDecal(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_pball->m_szImageFront, -1, wz, 32);
	*pVal = SysAllocString(wz);
	return S_OK;
}

STDMETHODIMP BallEx::put_FrontDecal(BSTR newVal)
{
	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_pball->m_szImageFront, 32, NULL, NULL);

	m_pball->m_pinFront = (lstrlen(m_pball->m_szImageFront) > 0) ? g_pplayer->m_ptable->GetImage(m_pball->m_szImageFront) : NULL;

	return S_OK;
}

STDMETHODIMP BallEx::get_BackDecal(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_pball->m_szImageFront, -1, wz, 32);
	*pVal = SysAllocString(wz);
	return S_OK;
}

STDMETHODIMP BallEx::put_BackDecal(BSTR newVal)
{
	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_pball->m_szImageBack, 32, NULL, NULL);

	m_pball->m_pinBack = (lstrlen(m_pball->m_szImageBack) > 0) ? g_pplayer->m_ptable->GetImage(m_pball->m_szImageBack) : NULL;

	return S_OK;
}

STDMETHODIMP BallEx::get_Name(BSTR *pVal)
{
	*pVal = SysAllocString(L"Ball");
	return S_OK;
}

STDMETHODIMP BallEx::put_Name(BSTR newVal)
{
	return S_OK;
}

STDMETHODIMP BallEx::get_CollisionMass(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->collisionMass;

	return S_OK;
}

STDMETHODIMP BallEx::put_CollisionMass(float newVal)
{
	CHECKSTALEBALL

	m_pball->collisionMass = newVal;

	return S_OK;
}

STDMETHODIMP BallEx::get_Radius(float *pVal)
{
	CHECKSTALEBALL

	*pVal = m_pball->radius;

	return S_OK;
}

STDMETHODIMP BallEx::put_Radius(float newVal)
{
	CHECKSTALEBALL

	m_pball->radius = newVal;

	m_pball->CalcBoundingRect();

	return S_OK;
}

STDMETHODIMP BallEx::DestroyBall(int *pVal)
{
	int cnt = 0;
	if (g_pplayer)
		{
		++cnt;
		g_pplayer->DestroyBall(g_pplayer->m_pactiveball); // 	
		g_pplayer->m_pactiveball = NULL;				// clear ActiveBall
		}

	if (pVal) *pVal = cnt;

	return S_OK;
}
