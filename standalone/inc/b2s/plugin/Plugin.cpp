#include "stdafx.h"

#include "Plugin.h"

Plugin::Plugin()
{
   m_status = PluginStatusEnum_Active;
}

Plugin::~Plugin()
{
}

void Plugin::SetStatus(PluginStatusEnum status)
{
   m_status = status;

   const char* pStatus = NULL;

   switch (status) {
      case PluginStatusEnum_Active:
         pStatus = "Active";
         break;
      case PluginStatusEnum_Disabled:
         pStatus = "Disabled";
         break;
      case PluginStatusEnum_DisabledDueToException:
         pStatus = "DisabledDueToException";
         break;
      default:
         PLOGE.printf("Plugin status update: name=%s, status=%d", GetName().c_str(), status);
         return;
   }

   PLOGI.printf("Plugin status update: name=%s, status=%s", GetName().c_str(), pStatus);
}