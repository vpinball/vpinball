#pragma once

#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <SDL2/SDL_ttf.h>

#define PUP_SETTINGS_BACKGLASSX      320
#define PUP_SETTINGS_BACKGLASSY      30
#define PUP_SETTINGS_BACKGLASSWIDTH  290
#define PUP_SETTINGS_BACKGLASSHEIGHT 218
#define PUP_ZORDER_BACKGLASS         150

#define PUP_SETTINGS_TOPPERX         320
#define PUP_SETTINGS_TOPPERY         PUP_SETTINGS_BACKGLASSY + PUP_SETTINGS_BACKGLASSHEIGHT + 5
#define PUP_SETTINGS_TOPPERWIDTH     290
#define PUP_SETTINGS_TOPPERHEIGHT    75
#define PUP_ZORDER_TOPPER            300

typedef enum
{
   PUP_SCREEN_MODE_OFF,
   PUP_SCREEN_MODE_SHOW,                 // This will use whatever settings are set for this screen’s “default” settings.
   PUP_SCREEN_MODE_FORCE_ON,             // Forces the window to be the Top most window, and it stays on. A video is always going to be playing in this mode. When a new video starts or the current video restarts, the ForceON action happens and will force the window to be the Top window again.
   PUP_SCREEN_MODE_FORCE_POP,            // Similar to ForceOn, except the vlc window opens and closes with each video played
   PUP_SCREEN_MODE_FORCE_BACK,           // Keeps the video window always open, but under the other PuP screens. This can cause any PuP screen with this setting to get pushed behind your game window.
   PUP_SCREEN_MODE_FORCE_POP_BACK,       // Similar to ForceBack, except the vlc window opens and closes with each video played
   PUP_SCREEN_MODE_MUSIC_ONLY
} PUP_SCREEN_MODE;

typedef enum
{
   PUP_PLAYLIST_FUNCTION_DEFAULT,
   PUP_PLAYLIST_FUNCTION_OVERLAYS,         // you put 32bit pngs in there and when you play them it will set them as current overlay frame.
   PUP_PLAYLIST_FUNCTION_FRAMES,           // you put 32bit pngs in there and when you play them it will set the background png (good for performance)
   PUP_PLAYLIST_FUNCTION_ALPHAS,           // you put 32bit pngs in there and when you play them it will set them as current overlay frame with alpha blending (performance, v1.4.5+)
   PUP_PLAYLIST_FUNCTION_SHAPES            // you put 24bit bmps files in there and the pixel color (0,0) will be used as a mask to make a see=through shape.
} PUP_PLAYLIST_FUNCTION;

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
   PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY  // this will ignore the trigger if the file playing has the same Priority. This is nice for events such as Jackpot videos or others that will play very often, and you don't want to have them constantly interrupting each other. “Normal” PlayAction files with the same Priority will interrupt each other no matter the Rest Seconds. Using SkipSamePri will not play the new file (with same the Priority) if the current file is still playing and allows for smoother non-interruptive action for common events.
} PUP_TRIGGER_PLAY_ACTION;

const char* PUP_TRIGGER_PLAY_ACTION_TO_STRING(PUP_TRIGGER_PLAY_ACTION value);
const char* PUP_PLAYLIST_FUNCTION_TO_STRING(PUP_PLAYLIST_FUNCTION value);
const char* PUP_SCREEN_MODE_TO_STRING(PUP_SCREEN_MODE value);

typedef struct {
   char type;
   int number;
   int value;
} PUPTriggerData;

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;
class PUPWindow;

class PUPManager
{
public:
   ~PUPManager();

   static PUPManager* GetInstance();
   bool IsInit() { return m_init; }
   bool LoadConfig(const string& szRomName);
   const string& GetPath() { return m_szPath; }
   bool AddScreen(PUPScreen* pScreen);
   PUPScreen* GetScreen(int screenNum);
   bool AddPlaylist(PUPPlaylist* pPlaylist);
   PUPPlaylist* GetPlaylist(const string& szFolder);
   bool AddTrigger(PUPTrigger* pTrigger);
   vector<PUPTrigger*>* GetTriggers(const string& szTrigger);
   bool AddFont(TTF_Font* pFont);
   TTF_Font* GetFont(const string& szFamily);
   void QueueTriggerData(PUPTriggerData data);
   void Start();
   void Stop();

private:
   PUPManager();

   void ProcessQueue();

   static PUPManager* m_pInstance;
   bool m_init;
   string m_szPath;
   std::map<int, PUPScreen*> m_screenMap;
   std::map<string, PUPPlaylist*> m_playlistMap;
   std::map<string, vector<PUPTrigger*>> m_triggerMap;
   std::map<string, TTF_Font*> m_fontMap;
   PUPScreen* m_pBackglassScreen;
   PUPWindow* m_pBackglassWindow;
   PUPScreen* m_pTopperScreen;
   PUPWindow* m_pTopperWindow;
   std::queue<PUPTriggerData> m_triggerDataQueue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning;
   std::thread m_thread;
};