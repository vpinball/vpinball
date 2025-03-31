#pragma once

#include "PUPManager.h"

#include "../common/Window.h"

class PUPScreen;

class PUPWindow : public VP::Window
{
public:
   PUPWindow(PUPScreen* pScreen, const string& szTitle, int z, int x, int y, int w, int h);
   ~PUPWindow();

   bool Init() override;
   void Render() override;

   SDL_Renderer* GetRenderer() { return m_visible ? m_pRenderer : NULL; }

private:
   PUPScreen* m_pScreen;
   SDL_Texture* m_pSDLTexture;
};