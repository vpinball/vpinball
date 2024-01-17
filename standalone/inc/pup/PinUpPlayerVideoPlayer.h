#pragma once

#include <string>

#include "../common/Window.h"

struct VideoState;

class PinUpPlayerVideoPlayer {
public:
   PinUpPlayerVideoPlayer(VP::Window* window);
   ~PinUpPlayerVideoPlayer();

   void Play();
   void Stop();
   void Pause();
   void Resume();
   void SetVideo(std::string path);

private:
    VP::Window* m_window;
    VideoState* m_videoState;
    std::string m_filename;

    void video_refresh_timer(void * userdata);
    void video_display(VideoState * videoState);
};