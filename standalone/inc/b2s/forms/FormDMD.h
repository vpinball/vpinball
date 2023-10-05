#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../../common/Timer.h"

class B2SScreen;

class FormDMD : public Form
{
public:
   FormDMD();
   ~FormDMD();

   void OnPaint(VP::Graphics* pGraphics) override;

   void SetBackgroundImage(SDL_Surface* pBackgroundImage) { m_pBackgroundImage = pBackgroundImage; }
   SDL_Surface* GetBackgroundImage() const { return m_pBackgroundImage; }

private:
   B2SData* m_pB2SData;
   SDL_Surface* m_pBackgroundImage;
};
