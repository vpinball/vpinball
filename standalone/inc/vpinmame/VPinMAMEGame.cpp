#include "stdafx.h"
#include "VPinMAMEGame.h"
#include "VPinMAMEGameSettings.h"

VPinMAMEGame::VPinMAMEGame(VPinMAMEController* pController, PinmameGame* pPinmameGame)
{
   m_pController = pController;
   memcpy(&m_pinmameGame, pPinmameGame, sizeof(PinmameGame));
}

VPinMAMEGame::~VPinMAMEGame()
{
}

STDMETHODIMP VPinMAMEGame::get_Name(BSTR *pVal)
{
   CComBSTR Name(m_pinmameGame.name);
   *pVal = Name.Detach();
   return S_OK;
}

STDMETHODIMP VPinMAMEGame::get_Description(BSTR *pVal)
{
   CComBSTR Description(m_pinmameGame.description);
   *pVal = Description.Detach();
   return S_OK;
}

STDMETHODIMP VPinMAMEGame::get_Year(BSTR *pVal)
{
   CComBSTR Year(m_pinmameGame.year);
   *pVal = Year.Detach();
   return S_OK;
}

STDMETHODIMP VPinMAMEGame::get_Manufacturer(BSTR *pVal)
{
   CComBSTR Manufacturer(m_pinmameGame.manufacturer);
   *pVal = Manufacturer.Detach();
   return S_OK;
}

STDMETHODIMP VPinMAMEGame::get_CloneOf(BSTR *pVal)
{
   CComBSTR CloneOf(m_pinmameGame.clone_of);
   *pVal = CloneOf.Detach();
   return S_OK;
}

STDMETHODIMP VPinMAMEGame::get_Roms(IRoms** pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMEGame::get_Settings(IGameSettings** pVal)
{
   return (new VPinMAMEGameSettings(m_pController, (IGame*)this))->QueryInterface(IID_IGameSettings, (void**)pVal);
}

STDMETHODIMP VPinMAMEGame::ShowInfoDlg(int nShowOptions, LONG_PTR hParentWnd, int *pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMEGame::get_IsSupported(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}