#pragma once

#include "PUPManager.h"

class PUPPlaylist;
class PUPScreen;

class PUPTrigger {
public:
   ~PUPTrigger() {}

   static PUPTrigger* CreateFromCSV(string line);
   bool IsActive() const { return m_active; }
   const string& GetDescription() const { return m_descript; }
   const string& GetTrigger() const { return m_trigger; }
   PUPScreen* GetScreen() const { return m_pScreen; }
   PUPPlaylist* GetPlaylist() const { return m_pPlaylist; }
   const string& GetPlayFile() { return m_playFile; }
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   int GetLength() const { return m_length; }
   int GetCounter() const { return m_counter; }
   int GetRestSeconds() const { return m_restSeconds; }
   PUP_TRIGGER_PLAY_ACTION GetPlayAction() const { return m_playAction; }
   string ToString() const;

private:
   PUPTrigger();
   bool m_active;
   string m_descript;
   string m_trigger;
   PUPScreen* m_pScreen;
   PUPPlaylist* m_pPlaylist;
   string m_playFile;
   int m_volume;
   int m_priority;
   int m_length;
   int m_counter;
   int m_restSeconds;
   PUP_TRIGGER_PLAY_ACTION m_playAction;
};