#include "stdafx.h"
#include "VPinMAMEGames.h"
#include "VPinMAMEGame.h"

void PINMAMECALLBACK VPinMAMEGames::GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData)
{
   VPinMAMEGames* pGames = (VPinMAMEGames*)pUserData;

   memcpy(&pGames->m_pinmameGame, pPinmameGame, sizeof(PinmameGame));
}

VPinMAMEGames::VPinMAMEGames(VPinMAMEController* pController)
{
   m_pController = pController;
}

VPinMAMEGames::~VPinMAMEGames()
{
}

STDMETHODIMP VPinMAMEGames::get_Count(LONG* pnCount)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMEGames::get_Item(VARIANT *pKey, IGame **pGame)
{
   if (V_VT(pKey) & VT_BSTR) {
      string szName = MakeString(V_BSTR(pKey));

      if (PinmameGetGame(szName.c_str(), &GetGameCallback, this) == PINMAME_STATUS_OK)
         return (new VPinMAMEGame(m_pController, &m_pinmameGame))->QueryInterface(IID_IGame, (void**)pGame);
   }

   return E_INVALIDARG;
}

STDMETHODIMP VPinMAMEGames::get__NewEnum(IUnknown** ppunkEnum)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}
