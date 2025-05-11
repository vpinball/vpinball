#pragma once

#include "PUPManager.h"

#include <algorithm>

class PUPPlaylist;
class PUPScreen;

typedef enum
{
   PUP_TRIGGER_PLAY_ACTION_NORMAL,         // plays the file until it ends
   PUP_TRIGGER_PLAY_ACTION_LOOP,           // plays the file in a Loop
   PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET,   // meant to be used with a Looping file. If this is triggered while a Looping file is currently playing…then the SplashReset file will play to its end, and then the original Looping file will resume from its beginning (there may be a pause when the Looping file begins again). This can be handy, but you would be better using SetBG in most cases to do something similar.
   PUP_TRIGGER_PLAY_ACTION_SPLASH_RETURN,  // meant to be used with a Looping file. If this is triggered while a Looping file is currently playing…then the SplashReturn file will play to its end, and then the original Looping file will resume from where it left off (there may be a pause when the Looping file begins again). This can be handy, but you would be better using SetBG in most cases to do something similar.
   PUP_TRIGGER_PLAY_ACTION_STOP_PLAYER,    // will stop whatever file is currently playing. Priority MUST be HIGHER than the file currently playing for this to work!
   PUP_TRIGGER_PLAY_ACTION_STOP_FILE,      // will stop ONLY the file specified in PlayFile (if it's playing). This has no effect on other files that are playing.
   PUP_TRIGGER_PLAY_ACTION_SET_BG,         // Set Background will set a new default looping “Background” file. When other files are done playing, then this new SetBG file will be played in a loop. Example: This can be handy for setting a new looping “mode” video, so that new other video events during the new mode will fall back to this SetBG video. Then you can change SetBG again to the main game mode video when the mode is completed.
   PUP_TRIGGER_PLAY_ACTION_PLAY_SSF,       // used to play WAV files for Surround Sound Feedback. (You don't want these sounds playing from your front / backbox speakers). The settings for the 3D position of the sound files are set in COUNTER. The format is in X,Z,Y. Example: “-2,1,-8”. X values range from -10 (left), 0 (center), 10 (right). Z values don't ever change and stay at 1. Y values range from 0 (top), -5 (center), -10 (bottom). NOTE: This currently will only work with the DEFAULT sound card in Windows. Additional sound card / devices are not yet supported!
   PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY,  // this will ignore the trigger if the file playing has the same Priority. This is nice for events such as Jackpot videos or others that will play very often, and you don't want to have them constantly interrupting each other. “Normal” PlayAction files with the same Priority will interrupt each other no matter the Rest Seconds. Using SkipSamePri will not play the new file (with same the Priority) if the current file is still playing and allows for smoother non-interruptive action for common events.
   PUP_TRIGGER_PLAY_ACTION_CUSTOM_FUNC     // Call a custom function
} PUP_TRIGGER_PLAY_ACTION;

const char* PUP_TRIGGER_PLAY_ACTION_TO_STRING(PUP_TRIGGER_PLAY_ACTION value);

class PUPTrigger final
{
public:
   ~PUPTrigger() {}
   static PUPTrigger* CreateFromCSV(PUPScreen* pScreen, const string& line);

   class PUPTriggerCondition
   {
   public:
      PUPTriggerCondition(char type, int number, int expected)
         : m_type(type), m_number(number), m_expected(expected) {}
      const char m_type; // Trigger type
      const int m_number; // Trigger id
      const int m_expected; // Expected value or -1 for default (triggered if non 0)

      bool IsTriggered() const { return ((m_expected == -1) && (m_value > 0)) || ((m_expected != -1) && (m_value == m_expected)); }
      int m_value = 0; // Current value
   };

   const string& GetDescription() const { return m_szDescript; }
   PUPScreen* GetScreen() const { return m_pScreen; }
   PUPPlaylist* GetPlaylist() const { return m_pPlaylist; }
   const string& GetPlayFile() const { return m_szPlayFile; }
   const string& GetTrigger() const { return m_szTrigger; }
   vector<PUPTriggerCondition>& GetTriggers() { return m_conditions; }
   bool IsTriggered() const { return std::ranges::all_of(m_conditions, [](const PUPTriggerCondition& x) { return x.IsTriggered(); }); }
   bool IsActive() const { return m_active; }
   float GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   int GetLength() const { return m_length; }
   int GetCounter() const { return m_counter; }
   int GetRestSeconds() const { return m_restSeconds; }
   PUP_TRIGGER_PLAY_ACTION GetPlayAction() const { return m_playAction; }
   string ToString() const;

   void SetTriggered();
   bool IsResting() const;

private:
   PUPTrigger(bool active, const string& szDescript, const string& szTrigger, PUPScreen* pScreen, PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, int length, int counter, int restSeconds, PUP_TRIGGER_PLAY_ACTION playAction);
   const string m_szDescript;
   const string m_szTrigger;
   vector<PUPTriggerCondition> m_conditions;
   PUPScreen* const m_pScreen;
   PUPPlaylist* const m_pPlaylist;
   const string m_szPlayFile;
   const bool m_active;
   const float m_volume;
   const int m_priority;
   const int m_length;
   const int m_counter;
   const int m_restSeconds;
   const PUP_TRIGGER_PLAY_ACTION m_playAction;

   Uint64 m_lastTriggered = 0;
};
