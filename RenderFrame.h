#pragma once

class RenderDevice;
class RenderPass;
class RenderCommand;

class RenderFrame final
{
public:
   RenderFrame(RenderDevice* renderDevice);
   ~RenderFrame();

   void AddPass(RenderPass* pass);
   void Execute(const bool log = false);

   RenderCommand* NewCommand();

private:
   RenderDevice* const m_rd;
   std::vector<RenderPass*> m_passes;
   std::vector<RenderCommand*> m_commandPool;
};
