#pragma once

#include "PUPScreen.h"
#include "PUPPlaylist.h"

class PUPTrigger {
public:
   PUPTrigger();
   ~PUPTrigger();

   static PUPTrigger* CreateFromCSVLine(string line, vector<PUPScreen*>& screens, vector<PUPPlaylist*>& playlists);

   int GetId() const { return m_id; }
   bool IsActive() const { return m_active; }
   const string& GetDescription() const { return m_descript; }
   const string& GetTrigger() const { return m_trigger; }
   PUPScreen* GetScreen() const { return m_pScreen; }
   PUPPlaylist* GetPlaylist() const { return m_pPlaylist; }
   const string& GetPlayFile();
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   int GetLength() const { return m_length; }
   int GetCounter() const { return m_counter; }
   int GetRestSeconds() const { return m_restSeconds; }
   const string& GetLoop() const { return m_loop; }
   int GetDefaults() const { return m_defaults; }
   string ToString() const;

private:
   int m_id;
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
   string m_loop;
   int m_defaults;
};