#pragma once

#include "PUPScreen.h"
#include "PUPPlaylist.h"

class PUPTrigger {
public:
   PUPTrigger();
   ~PUPTrigger();

   static PUPTrigger* CreateFromCSVLine(string line);

   int GetId() const { return m_id; }
   bool IsActive() const { return m_active; }
   const string& GetDescription() const { return m_descript; }
   const string& GetTrigger() const { return m_trigger; }
   int GetScreen() const { return m_screen; }
   const string& GetPlaylist() const { return m_playlist; }
   const string& GetPlayFile() { return m_playFile; }
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   int GetLength() const { return m_length; }
   int GetCounter() const { return m_counter; }
   int GetRestSeconds() const { return m_restSeconds; }
   PUP_TRIGGER_PLAY_ACTION GetPlayAction() const { return m_playAction; }
   int GetDefaults() const { return m_defaults; }
   string ToString() const;

private:
   int m_id;
   bool m_active;
   string m_descript;
   string m_trigger;
   int m_screen;
   string m_playlist;
   string m_playFile;
   int m_volume;
   int m_priority;
   int m_length;
   int m_counter;
   int m_restSeconds;
   PUP_TRIGGER_PLAY_ACTION m_playAction;
   int m_defaults;
};