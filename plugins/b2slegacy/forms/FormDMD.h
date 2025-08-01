#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../utils/Timer.h"

namespace B2SLegacy {

class B2SScreen;
class Server;

class FormDMD : public Form
{
public:
   FormDMD(MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, B2SData* pB2SData);
   ~FormDMD();

   void OnPaint(VPXRenderContext2D* const ctx) override;

private:
};

}
