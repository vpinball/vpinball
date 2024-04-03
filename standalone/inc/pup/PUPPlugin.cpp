#include "core/stdafx.h"

#include "PUPPlugin.h"

PUPPlugin::PUPPlugin() : Plugin()
{
}

PUPPlugin::~PUPPlugin()
{
}

const std::string& PUPPlugin::GetName() const
{
   static std::string name = "PinUpPlugin";
   return name;
}

void PUPPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
}

void PUPPlugin::PluginFinish()
{
}

void PUPPlugin::DataReceive(char type, int number, int value)
{
}