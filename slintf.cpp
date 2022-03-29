#include "stdafx.h"

#ifdef SLINTF
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <io.h>
#include <FCNTL.H>

static int sbConsole;

void slintf_init()
{
  sbConsole = 0;
}

const int MAX_CONSOLE_LINES = 5000;

void RedirectIoToConsole()
{
  int hConHandle;
  long lStdHandle;

  CONSOLE_SCREEN_BUFFER_INFO coninfo;

  FILE* fp = nullptr;

  // allocate a console for this app
  AllocConsole();

  // http://softwareforums.intel.com/ids/board/message?board.id=15&message.id=2971&page=1&view=by_threading
  // would like to see messages without mousing around each time I run
  {
    char name[256];
    HWND win;
    GetConsoleTitle(name, 255);
    win = FindWindow(nullptr, name);
    SetWindowPos(win, HWND_TOP, 0, 640, 640, 480, SWP_NOSIZE);
    ShowWindow(win, SW_SHOWNORMAL);
  }

  // set the screen buffer to be big enough to let us scroll text

  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

  coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  // redirect unbuffered STDOUT to the console

  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen(hConHandle, "w");

  *stdout = *fp;

  setvbuf(stdout, nullptr, _IONBF, 0);

  // redirect unbuffered STDIN to the console

  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen(hConHandle, "r");

  *stdin = *fp;

  setvbuf(stdin, nullptr, _IONBF, 0);

  // redirect unbuffered STDERR to the console

  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen(hConHandle, "w");

  *stderr = *fp;

  setvbuf(stderr, nullptr, _IONBF, 0);

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well

  // ios::sync_with_stdio();
}

void slintf_popup_console()
{
  if (!sbConsole)
  {
    sbConsole = 1;
    RedirectIoToConsole();
  }
}

void slintf_close_console()
{
  if (sbConsole)
  {
    FreeConsole();
    sbConsole = 0;
  }
}

int file_exists(const char* filename)
{
  FILE* file;
  if (fopen_s(&file, filename, "r") != 0 || !file)
  {
    return 0;
  }
  else
  {
    fclose(file);
    return 1;
  }
}

void slintf(const char* str, ...)
{
  static int initted;
  static int noconsole;

  if (!sbConsole)
  {
    return;
  }
  if (noconsole)
  {
    return;
  }
  if (!initted)
  {
    initted = 1;

    if (file_exists(".vp_noconsole"))
    {
      noconsole = 1;
      return;
    }
  }

#define MAX_SCREEN_BUFFER_SIZE 8192
  char buffer[MAX_SCREEN_BUFFER_SIZE];
  if (!str || !str[0])
  {
    return;
  }

  va_list ap;

  va_start(ap, str);

  /*const int len =*/vsprintf_s(buffer, sizeof(buffer), str, ap);

  slintf_popup_console();

  fputs(buffer, stdout);
}
#endif
