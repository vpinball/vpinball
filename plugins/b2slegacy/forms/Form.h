#pragma once

#include "../controls/Control.h"

namespace B2SLegacy {

class B2SData;
class Server;

class Form : public Control
{
public:
   Form(MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, B2SData* pB2SData);
   ~Form();

   void Show();
   void Hide();
   void SetTopMost(bool topMost) { m_topMost = topMost; }
   bool IsTopMost() const { return m_topMost; }
   B2SData* GetB2SData() const { return m_pB2SData; }

protected:
   MsgPluginAPI* m_msgApi = nullptr;
   B2SData* m_pB2SData = nullptr;

private:
   bool m_topMost = false;
};

}
