#pragma once

#include "common.h"
#include "ScriptablePlugin.h"
#include "Controller.h"
#include "GameSettings.h"
#include "Roms.h"

#include <cassert>

class Game
{
public:
   Game(Controller* pController, const PinmameGame& pinmameGame)
      : m_pController(pController)
   {
      m_pController->AddRef();
      memcpy(&m_pinmameGame, &pinmameGame, sizeof(PinmameGame));
   }
   ~Game()
   {
      assert(m_refCount == 0);
      m_pController->Release();
   }

   PSC_IMPLEMENT_REFCOUNT()

   string GetName() const { return m_pinmameGame.name; }
   string GetDescription() const { return m_pinmameGame.description; }
   string GetYear() const { return m_pinmameGame.year; }
   string GetManufacturer() const { return m_pinmameGame.manufacturer; }
   string GetCloneOf() const { return m_pinmameGame.clone_of; }
   bool GetIsSupported() const { return true; /* Not yet implemented */ }
   Roms *GetRoms() const { return nullptr; /* Not yet implemented */ }
   GameSettings* GetSettings() const { return new GameSettings(); }
   int ShowInfoDlg(int nShowOptions, void *hParentWnd) const { return 0; /* Not yet implemented */ }

private:
   PinmameGame m_pinmameGame;
   Controller* m_pController;
};
