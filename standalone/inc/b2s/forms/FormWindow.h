#pragma once

#include "../../common/Window.h"
#include "../../common/Graphics.h"

#include "Form.h"

class FormWindow : public VP::Window
{
public:
   ~FormWindow();

   static FormWindow* Create(const std::string& szTitle, int x, int y, int w, int h, int z);

   void SetForm(Form* pForm);
   void Render() override;

private:
   FormWindow(const std::string& szTitle, int x, int y, int w, int h, int z, bool highDpi);

   Form* m_pForm;
   VP::Graphics* m_pGraphics;
};
