#pragma once

class RenderDevice;
class RenderPass;
class RenderCommand;

class RenderFrame final
{
public:
   RenderFrame(RenderDevice* renderDevice);
   ~RenderFrame();

   RenderPass* AddPass(const string& name, RenderTarget* const rt, const bool ignoreStereo);
   void Execute(const bool log = false);

   RenderCommand* NewCommand();

private:
   RenderDevice* const m_rd;
   vector<RenderPass*> m_passes;
   vector<RenderPass*> m_passPool;
   vector<RenderCommand*> m_commandPool;
};
