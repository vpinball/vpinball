// license:GPLv3+

#pragma once

#include <SDL3/SDL_dialog.h>

namespace VPX
{

void ShowOpenFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation, bool allowMany);
void ShowSaveFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation);
void ShowOpenFolderDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const char *defaultLocation, bool allowMany);

}
