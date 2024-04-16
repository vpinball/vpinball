#pragma once

#ifdef ENABLE_SDL_VIDEO // SDL Windowing
#include "SDL2/SDL.h"
#else // Win32 Windowing
#include <windows.h>
#endif


struct VideoMode
{
   int width;
   int height;
   int depth;
   int refreshrate;
};

struct DisplayConfig
{
   int display; // Window Display identifier (the number that appears in the native Windows settings)
   int adapter; // DirectX or SDL display/adapter identifier
   int top;
   int left;
   int width;
   int height;
   bool isPrimary;
   char DeviceName[CCHDEVICENAME]; // Device native identifier, e.g. "\\\\.\\DISPLAY1"
   char GPU_Name[MAX_DEVICE_IDENTIFIER_STRING]; // GPU name if available, device (monitor) name otherwise
};

int getNumberOfDisplays();
void EnumerateDisplayModes(const int display, vector<VideoMode>& modes);
bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height);
int getDisplayList(vector<DisplayConfig>& displays);
int getPrimaryDisplay();

namespace VPX
{

class Window final
{
public:
   Window(const string &title, const string &settingsId, const int display, const int w, const int h, const bool fullscreen, const int fsBitDeth, const int fsRefreshRate);
   ~Window();

   void GetPos(int&x, int &y) const;
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   int GetRefreshRate() const { return m_refreshrate; }

   void ShowAndFocus();
   void SetPos(const int x, const int y);

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_Window * GetCore() const { return m_nwnd; }
   #else // Win32 Windowing
      HWND GetCore() const { return m_nwnd; }
   #endif
   
private:
   int m_width, m_height;
   int m_display, m_adapter;
   int m_screenwidth, m_screenheight;
   bool m_fullscreen;
   int m_refreshrate; // Only valid for non windowed fullscreen, 0 otherwise
   string m_settingsId;

   class RenderTarget* m_backBuffer = nullptr;

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_Window *m_nwnd = nullptr;
   #else // Win32 Windowing
      HWND m_nwnd = nullptr;
   #endif
};

}