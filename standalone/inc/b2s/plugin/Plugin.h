#pragma once

#include "../b2s_i.h"

class Plugin
{
public:
   Plugin();
   virtual ~Plugin();

   virtual const std::string& GetName() const = 0;
   PluginStatusEnum GetStatus() { return m_status; }
   void SetStatus(PluginStatusEnum status);
   virtual void PluginInit(const string& szTableFilename, const string& szRomName) = 0;
   virtual void PluginFinish() = 0;
   virtual void PinMameRun() {};
   virtual void PinMamePause() {};
   virtual void PinMameContinue() {};
   virtual void PinMameStop() {};
   virtual void DataReceive(char type, int number, int value) {};

private:
   PluginStatusEnum m_status;
};