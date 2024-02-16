#pragma once

#include "../common/Window.h"
#include "../../../src/audio/audioplayer.h"
#include "PinUpPlayerLabel.h"

#include <map>
#include <set>

class PinUpPlayerScreen {
public:
   PinUpPlayerScreen();
   ~PinUpPlayerScreen();

   static PinUpPlayerScreen* CreateFromCSVLine(string line);

   enum WindowPreference {
      // This will use whatever settings are set for this screen’s “default” settings.
      Show,

      // Forces the window to be the Top most window, and it stays on. 
      // A video is always going to be playing in this mode. 
      // When a new video starts or the current video restarts, 
      // the ForceON action happens and will force the window to be the Top window again
      ForceON,

      // Similar to ForceOn, except the window opens and closes with each video played
      ForcePoP,

      // Keeps the video window always open, but under the other PuP screens. 
      // This can cause any PuP screen with this setting to get pushed behind your game window
      ForceBack,

      // Similar to ForceBack, except the window opens and closes with each video played
      ForcePopBack,

      // First number is always the screen. 1,22.5,45.3,55.1,54.7 
      // [pupid # source screen,xpos,ypos,width,height] values in % of pupdisplay
      CustomPos,

      MusicOnly,
      Off
   };

   int m_screenNum;
   string m_screenDes;
   string m_playList;
   string m_playFile;
   string m_loopit;
   WindowPreference m_active;
   int m_priority;
   string m_customPos;
   VP::Window* m_window = NULL;
   AudioPlayer* m_audioPlayer = NULL;
   int m_labelPageSeconds = 0;

   // Labels
   std::map<string,PupLabel*> m_labels;
   std::map<int,std::set<PupLabel*>*> m_labelPages;
   int m_defaultPageNum = 0;
   int m_currentPage = 0;

   void PlayMusic(string path, LONG volume, LONG forceplay);
   void ShowImage(string path, LONG forceplay);
   void PlayVideo(string path, LONG volume, LONG forceplay);
   void ClearBackground();
   void LabelUpdated(PupLabel* label, int pageNumber = -1);
   void Render(VP::Graphics* graphics);
   void MarkDirty();

private:
   SDL_Texture* m_imageTexture = NULL;
   volatile bool m_isDirty = false;
};

