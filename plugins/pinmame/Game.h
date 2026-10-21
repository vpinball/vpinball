// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"
#include "Controller.h"
#include "GameSettings.h"
#include "Roms.h"

#include <cassert>

namespace PinMAME {

class Game final
{
public:
   Game(Controller* pController, const PinmameGame& pinmameGame, GameSettings* pSettings)
      : m_pController(pController)
      , m_settings(pSettings)
   {
      m_pController->AddRef();
      m_settings->AddRef();
      memcpy(&m_pinmameGame, &pinmameGame, sizeof(PinmameGame));
      m_name = m_pinmameGame.name;
   }
   // Placeholder for a name PinMAME doesn't know (e.g. a custom cGameName): VPinMAME never returns Nothing
   // from Games(name), so scripts commonly write its settings before the game is resolved
   Game(Controller* pController, const string& name, GameSettings* pSettings)
      : m_pController(pController)
      , m_settings(pSettings)
      , m_name(name)
   {
      m_pController->AddRef();
      m_settings->AddRef();
      memset(&m_pinmameGame, 0, sizeof(PinmameGame));
      m_pinmameGame.name = m_name.c_str();
      m_pinmameGame.clone_of = m_pinmameGame.description = m_pinmameGame.year = m_pinmameGame.manufacturer = "";
   }
   ~Game()
   {
      assert(m_refCount == 0);
      m_pController->Release();
      m_settings->Release();
   }

   PSC_IMPLEMENT_REFCOUNT()

   string GetName() const { return m_name; }
   string GetDescription() const { return m_pinmameGame.description; }
   string GetYear() const { return m_pinmameGame.year; }
   string GetManufacturer() const { return m_pinmameGame.manufacturer; }
   string GetCloneOf() const { return m_pinmameGame.clone_of; }
   bool GetIsSupported() const { return true; /* Not yet implemented */ }
   Roms *GetRoms() const { return nullptr; /* Not yet implemented */ }
   GameSettings* GetSettings() { m_settings->AddRef(); return m_settings; }
   int ShowInfoDlg(int nShowOptions, void* hParentWnd) const { LOGE("Game.ShowInfoDlg is not implemented."); return 0; }

private:
   PinmameGame m_pinmameGame;
   Controller* m_pController;
   GameSettings* m_settings;
   string m_name;
};

}
