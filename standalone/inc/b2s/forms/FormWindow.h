#pragma once

#include "../../common/Window.h"
#include "../../common/Graphics.h"

#include "Form.h"

class FormWindow : public VP::Window
{
public:
   FormWindow(Form* pForm, const std::string& szTitle, int x, int y, int w, int h, int z);
   ~FormWindow();

   bool Init() override;
   void Render() override;

private:
   Form* m_pForm;
   VP::Graphics* m_pGraphics;
};
