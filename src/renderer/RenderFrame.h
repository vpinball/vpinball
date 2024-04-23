#pragma once

#include "Shader.h"

class RenderDevice;
class RenderDeviceState;
class RenderPass;
class RenderCommand;

class RenderFrame final
{
public:
   RenderFrame(RenderDevice* renderDevice);
   ~RenderFrame();

   RenderPass* AddPass(const string& name, RenderTarget* const rt);
   bool Execute(const bool log = false);

   RenderCommand* NewCommand();

private:
   void SortPasses(RenderPass* finalPass, vector<RenderPass*>& sortedPasses);
   
   RenderDevice* const m_rd;
   RenderDeviceState* m_rdState = nullptr;
   vector<RenderPass*> m_passes;
   vector<RenderPass*> m_passPool;
   vector<RenderCommand*> m_commandPool;
};
