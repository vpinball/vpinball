#pragma once

#include "PUPManager.h"

#include "../common/Window.h"

class PUPScreen;

class PUPWindow : public VP::Window
{
public:
   PUPWindow(PUPScreen* pScreen, const string& szTitle, int x, int y, int w, int h, int rotation, int z);
   ~PUPWindow();

   bool Init() override;
   void Render() override;

private:
   PUPScreen* m_pScreen;
};