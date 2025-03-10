// license:GPLv3+

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

   RenderCommand* NewCommand();

   RenderPass* AddPass(const string& name, RenderTarget* const rt);
   void AddBeginOfFrameCmd(const std::function<void()>& cmd) { m_beginOfFrameCmds.push_back(cmd); }
   void AddEndOfFrameCmd(const std::function<void()>& cmd) { m_endOfFrameCmds.push_back(cmd); }
   bool Execute(const bool log = false);
   void Discard();

private:
   void SortPasses(RenderPass* finalPass, vector<RenderPass*>& sortedPasses);

   RenderDevice* const m_rd;
   RenderDeviceState* m_rdState = nullptr;
   vector<RenderPass*> m_passes;
   vector<RenderPass*> m_passPool;
   vector<RenderCommand*> m_commandPool;
   vector<std::function<void()>> m_beginOfFrameCmds;
   vector<std::function<void()>> m_endOfFrameCmds;
};
