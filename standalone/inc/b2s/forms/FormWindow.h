#pragma once

#include "../../common/Window.h"
#include "../../common/RendererGraphics.h"

#include "Form.h"

class FormWindow : public VP::Window
{
public:
   FormWindow(Form* pForm, const string& szTitle, int x, int y, int w, int h, int z, int rotation);
   ~FormWindow();

   bool Init() override;
   void Render() override;

private:
   SDL_FRect m_destRect;
   int m_angle;
   Form* m_pForm;
   VP::RendererGraphics* m_pGraphics;
   SDL_Texture* m_pTexture;
};
