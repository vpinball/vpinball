#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../utils/Timer.h"

namespace B2SLegacy {

class B2SScreen;
class Server;

class FormDMD final : public Form
{
public:
   FormDMD(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData);
   ~FormDMD();

   void OnPaint(VPXRenderContext2D* const ctx) override;
};

}
