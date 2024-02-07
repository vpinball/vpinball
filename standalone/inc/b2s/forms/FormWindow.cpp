#include "stdafx.h"

#include "FormWindow.h"
#include "Form.h"

FormWindow::FormWindow(Form* pForm, const std::string& szTitle, int x, int y, int w, int h, int z)
    : VP::Window(szTitle, x, y, w, h, z)
{
   m_pForm = pForm;
   m_pGraphics = nullptr;
}

bool FormWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   m_pGraphics = new VP::RendererGraphics(m_pRenderer);

   return true;
}

FormWindow::~FormWindow()
{
   delete m_pGraphics;
}

void FormWindow::Render()
{
   m_pForm->Render(m_pGraphics);
}