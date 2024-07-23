#pragma once

#include "../controls/Control.h"
#include "../../../common/RendererGraphics.h"
#include "../../common/Window.h"

class FormWindow;

class Form : public Control
{
public:
   Form();
   ~Form();

   void Show();
   void Hide();
   void SetTopMost(bool topMost) { m_topMost = topMost; }
   bool IsTopMost() const { return m_topMost; }
   void SetGraphics(VP::RendererGraphics* pGraphics) { m_pGraphics = pGraphics; }
   bool Render();

protected:
   FormWindow* m_pWindow;

private:
   bool m_topMost;
   VP::RendererGraphics* m_pGraphics;
};
