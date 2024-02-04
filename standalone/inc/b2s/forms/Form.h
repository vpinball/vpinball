#pragma once

#include "../controls/Control.h"
#include "../../../common/Graphics.h"
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
   void Render(VP::Graphics* pGraphics);

protected:
   FormWindow* m_pWindow;

private:
   bool m_topMost;

};
