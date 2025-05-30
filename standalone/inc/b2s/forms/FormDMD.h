#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../../common/Timer.h"

class B2SScreen;

class FormDMD : public Form
{
public:
   FormDMD(B2SData* pB2SData);
   ~FormDMD();

   void OnPaint(VP::RendererGraphics* pGraphics) override;

private:
};
