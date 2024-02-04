#include "stdafx.h"

#include "FormWindow.h"
#include "Form.h"

FormWindow::FormWindow(const std::string& szTitle, int x, int y, int w, int h, int z, bool highDpi)
    : VP::Window(szTitle, x, y, w, h, z, highDpi)
{
   m_pForm = nullptr;
   m_pGraphics = m_pRenderer ? new VP::Graphics(m_pRenderer) : nullptr;
}

FormWindow::~FormWindow()
{
   delete m_pGraphics;
}

FormWindow* FormWindow::Create(const string& szTitle, int x, int y, int w, int h, int z)
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   FormWindow* pWindow = new FormWindow(szTitle, x, y, w, h, z,
      pSettings->LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true));

   if (!pWindow->m_pGraphics) {
      delete pWindow;
      return nullptr;
   }

   return pWindow;
}

void FormWindow::SetForm(Form* pForm)
{
   m_pForm = pForm;
}

void FormWindow::Render()
{
   if (m_pForm)
      m_pForm->Render(m_pGraphics);
}