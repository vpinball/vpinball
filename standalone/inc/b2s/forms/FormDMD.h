#pragma once

#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"

#include "../classes/Timer.h"
#include "../controls/Control.h"

class B2SScreen;

class FormDMD : public Control
{
public:
   FormDMD();
   ~FormDMD();

   void OnPaint(SDL_Surface* pSurface) override;

   void SetBackgroundImage(SDL_Surface* pBackgroundImage) { m_pBackgroundImage = pBackgroundImage; }
   SDL_Surface* GetBackgroundImage() const { return m_pBackgroundImage; }

private:
   SDL_Surface* m_pBackgroundImage;
};
