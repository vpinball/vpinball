#pragma once

class PinUpPlayerTrigger {
public:
   PinUpPlayerTrigger();
   ~PinUpPlayerTrigger();

   static PinUpPlayerTrigger* CreateFromCSVLine(string line);

   int m_id;
   int m_active;
   string m_descript;
   string m_trigger;
   int m_screenNum;
   string m_playList;
   string m_playFile;
   int m_volume;
   int m_priority;
   int m_length;
   int m_counter;
   int m_restSeconds;
   string m_loop;
   int m_defaults;
};