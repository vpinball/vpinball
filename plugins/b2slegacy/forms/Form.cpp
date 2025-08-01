#include "../common.h"

#include "Form.h"
#include "../Server.h"

namespace B2SLegacy {

Form::Form(MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, B2SData* pB2SData) :
   Control(vpxApi), m_msgApi(msgApi), m_pB2SData(pB2SData)
{
}

Form::~Form()
{
}


void Form::Show()
{
}

void Form::Hide()
{
}

}
