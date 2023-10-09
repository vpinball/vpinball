#pragma once

#include "../controls/Control.h"
#include "../../common/Graphics.h"

class Form : public Control
{
public:
   Form();
   ~Form();

   void Show();
   void Hide();
   void SetTopMost(bool topMost) { m_topMost = topMost; }
   bool IsTopMost() const { return m_topMost; }

   void Render(SDL_Renderer* pRenderer);

private:
   VP::Graphics* m_pGraphics;
   bool m_topMost;

   SDL_Texture* m_pTexture;
};
